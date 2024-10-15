#ifndef PRICE_ANALYZER_DATA_ANALYZER_H
#define PRICE_ANALYZER_DATA_ANALYZER_H

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

namespace PriceAnalyzer {
struct Data {
 public:
  explicit Data(const std::string& title, const std::string& id, const std::vector<double>& prices)
      : title_(title),
        id_(id),
        min_(std::numeric_limits<double>::max()),
        max_(std::numeric_limits<double>::lowest()),
        average_(0) {
    Calculate(prices);
  }

  explicit Data(std::string&& title, std::string&& id, const std::vector<double>& prices)
      : title_(std::move(title)),
        id_(std::move(id)),
        min_(std::numeric_limits<double>::max()),
        max_(std::numeric_limits<double>::lowest()),
        average_(0) {
    Calculate(prices);
  }

  std::string title_;
  std::string id_;
  double min_;
  double max_;
  double average_;

 private:
  inline void Calculate(const std::vector<double>& prices) {
    if (prices.empty()) {
      min_ = max_ = average_ = 0;
      return;
    }

    double sum = 0;
    for (const auto& price : prices) {
      min_ = std::min(min_, price);
      max_ = std::max(max_, price);
      sum += price;
    }
    average_ = sum / prices.size();
  }
};
}  // namespace PriceAnalyzer
#endif  // PRICE_ANALYZER_DATA_ANALYZER_H