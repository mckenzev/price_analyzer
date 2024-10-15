#ifndef PRICE_ANALYZER_STRING_UTILS_H
#define PRICE_ANALYZER_STRING_UTILS_H

#include <string>
#include <vector>

namespace PriceAnalyzer {
std::string ToLowerUtf8(const std::string& str);
std::string ToRuFromAscii(const std::string& str);
std::string ToAsciiFromRu(const std::string& str);
std::string ToUrlFormat(const std::string& name);
std::vector<std::string> Split(const std::string& str, char delimiter);
int CountIdenticalSubstring(const std::vector<std::string>& str1,
                            const std::vector<std::string>& str2);
void RemoveBackPoint(std::string& str);

}  // namespace PriceAnalyzer
#endif  // PRICE_ANALYZER_STRING_UTILS_H