#include "http_client.h"

#include <iostream>
#include <vector>
#include <utility>

#include "callback_func.h"

namespace PriceAnalyzer {
HttpClient::HttpClient() {
  x_requested_ = "X-Requested-With: XMLHttpRequest";  // const
  url_ = "cdn.farmlend.ru";
  curl_handle_ = curl_easy_init();
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
  curl_easy_cleanup(curl_handle_);
  curl_global_cleanup();
}

CurlCode HttpClient::GetHtmlTable() {
  buffer_ = "";
  if (!curl_handle_) return CurlCode::NOT_INITIALIZED;

  std::string post_data = "dfm_id=" + id_ + "&sphere=0";

  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, csrf_token_.c_str());
  headers = curl_slist_append(headers, x_requested_.c_str());
  
  curl_easy_reset(curl_handle_);
  curl_easy_setopt(curl_handle_, CURLOPT_URL, "https://cdn.farmlend.ru/ajax/get-variants?.");
  curl_easy_setopt(curl_handle_, CURLOPT_COOKIEFILE, "cookies.txt");
  curl_easy_setopt(curl_handle_, CURLOPT_COOKIEJAR, "cookies.txt");
  curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, post_data.c_str());
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &buffer_);

  CURLcode res = curl_easy_perform(curl_handle_);

  curl_slist_free_all(headers);
  if (res != CURLE_OK) {
    std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
    return MapCurlCode(res);
  }

  return CurlCode::SUCCESS;
}

CurlCode HttpClient::GetMainPage() {
  buffer_ = "";
  if (!curl_handle_) return CurlCode::NOT_INITIALIZED;

  std::string url = "https://" + url_ + "/kazan/product/" + id_ + "?.";  // ?. обходит блокировку

  curl_easy_reset(curl_handle_);
  curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle_, CURLOPT_COOKIEFILE, "cookies.txt");
  curl_easy_setopt(curl_handle_, CURLOPT_COOKIEJAR, "cookies.txt");
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &buffer_);

  CURLcode res = curl_easy_perform(curl_handle_);

  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
    return MapCurlCode(res);
  }

  return CurlCode::SUCCESS;
}

CurlCode HttpClient::GetSearcherPage(const std::string& url_format_name) {
  static int counter = 0;
  static const std::vector<std::string> kUserAgents = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 YaBrowser/24.6.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:102.0) Gecko/20100101 Firefox/102.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.1 Safari/605.1.15",
    "Mozilla/5.0 (Linux; Android 11; SM-A505F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.131 Mobile Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36 Edg/127.0.0.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36"
  };
  static const std::vector<std::pair<std::string, std::string>> kSearchPattern = {
    {"", "."}, {"", "?."}, {"", ".?"}, {"", "%20."}, {"", "+."},
    {".", ""}, {"?.", ""}, {".?", ""}, {".%20", ""}, {".+", ""}
  };
  
  buffer_ = "";
  if (!curl_handle_) return CurlCode::NOT_INITIALIZED;

  std::string url = "https://" + url_ + "/search?keyword="
                  + kSearchPattern[counter % kSearchPattern.size()].first
                  + url_format_name
                  + kSearchPattern[counter % kSearchPattern.size()].second;
  
  struct curl_slist* headers = nullptr;
  if (!csrf_token_.empty()) {
    headers = curl_slist_append(headers, csrf_token_.c_str());
  }
  headers = curl_slist_append(headers, x_requested_.c_str());
  curl_easy_reset(curl_handle_);
  curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle_, CURLOPT_COOKIEJAR, "cookies.txt");
  curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl_handle_, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br, zstd");
  curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, kUserAgents[counter % kUserAgents.size()]);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &buffer_);
  ++counter;

  CURLcode res = curl_easy_perform(curl_handle_);
  curl_slist_free_all(headers);
  if (res != CURLE_OK) {
      std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
  }
  return MapCurlCode(res);;
}

CurlCode HttpClient::MapCurlCode(CURLcode code) {
  switch (code) {
    case CURLE_OK:
      return CurlCode::SUCCESS;
    case CURLE_COULDNT_CONNECT:
      return CurlCode::CONNECTION_FAILED;
    case CURLE_TOO_MANY_REDIRECTS:
    case CURLE_ABORTED_BY_CALLBACK:
      return CurlCode::BLOCKED_BY_IP;
    default:
      return CurlCode::OTHER_ERROR;
  }
}
}