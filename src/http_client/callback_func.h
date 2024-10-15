#ifndef PRICE_ANALYZER_CALBACK_FUNC_H
#define PRICE_ANALYZER_CALBACK_FUNC_H

namespace PriceAnalyzer {
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
}
#endif  // PRICE_ANALYZER_CALBACK_FUNC_H