#ifndef PRICE_ANALYZER_HTTP_CLIENT_H
#define PRICE_ANALYZER_HTTP_CLIENT_H

#include <curl.h>

#include <string>

namespace PriceAnalyzer {
enum class CurlCode {
  SUCCESS,
  NOT_INITIALIZED,
  CONNECTION_FAILED,
  BLOCKED_BY_IP,
  CAPTCHA_REQUIRED,
  OTHER_ERROR
};

class HttpClient {
 public:
  HttpClient();
  ~HttpClient();

  inline void SetCsrfToken(const std::string& token) {
    csrf_token_ = "X-CSRF-Token: " + token;
  }
  inline void SetId(const std::string& id) { id_ = id; }
  inline const std::string& GetBuffer() { return buffer_; }
  CurlCode GetSearcherPage(const std::string& url_format_name);
  CurlCode GetMainPage();
  CurlCode GetHtmlTable();

 private:
  CurlCode MapCurlCode(CURLcode code);

  CURL* curl_handle_;
  std::string url_;
  std::string id_;
  std::string csrf_token_;
  std::string x_requested_;
  std::string buffer_;
};
}  // namespace PriceAnalyzer
#endif  // PRICE_ANALYZER_HTTP_CLIENT_H