#include "html_reader.h"

namespace PriceAnalyzer {
int HtmlReader::Count(const std::string& str) {
  int counter = 0;
  size_t pos = html_code_->find(str);
  while (pos != std::string::npos) {
    pos = html_code_->find(str, pos + 1);
    ++counter;
  }
  return counter;
}

std::string HtmlReader::Find(const std::string& str, FinderMode mode) {
  pos_ = html_code_->find(str, pos_);
  if (pos_ == std::string::npos) return nullptr;

  size_t next_pos = pos_;

  if (mode == FinderMode::CONTENT) {
    next_pos = html_code_->find("content=", pos_);
    if (next_pos == std::string::npos) return nullptr;
  }

  size_t start_quote = html_code_->find('"', next_pos);
  size_t end_quote = html_code_->find('"', start_quote + 1);
  pos_ = end_quote;

  if (start_quote == std::string::npos || end_quote == std::string::npos)
    return "";

  return html_code_->substr(start_quote + 1, end_quote - start_quote - 1);
}

void HtmlReader::NewHtmlCode(const std::string& code) {
    html_code_ = &code;
    ResetPos();
}
}