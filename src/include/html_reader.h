#ifndef PRICE_ANALYZER_HTTP_READER_H
#define PRICE_ANALYZER_HTTP_READER_H

#include <string>

namespace PriceAnalyzer {
class HtmlReader {
 public:
  enum class FinderMode { NONE, CONTENT };

  HtmlReader(const std::string& code) : html_code_(&code), pos_(0) {}
  int Count(const std::string& str);  // Количество совпадений в коде
  std::string Find(const std::string& str, FinderMode mode = FinderMode::NONE);
  void NewHtmlCode(const std::string& code);

 private:
  inline void ResetPos() {
    pos_ = 0;
  }  // Значение курсора становится равно нулю

  const std::string* html_code_;  // << Html код
  size_t pos_;                    // < Позиция курсора в коде
};
}  // namespace PriceAnalyzer
#endif