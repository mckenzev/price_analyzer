#ifndef PRICE_ANALYZER_EXCEL_PROCESSOR_H
#define PRICE_ANALYZER_EXCEL_PROCESSOR_H

#include <functional>
#include <initializer_list>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include <OpenXLSX.hpp>

#include "data_analyzer.h"

namespace PriceAnalyzer {

class ExcelProcessor {
 public:
  ExcelProcessor() = default;
  explicit ExcelProcessor(const std::string& filename);
  std::vector<std::string> GetSingleColumn(const std::string& column);
  std::vector<std::vector<std::string>> GetSelectedColumns(
    const std::initializer_list<std::string>& columns,
    const std::string& filter_header, std::function<bool(const std::string&)> filter);
  void OpenFile(const std::string& filename);
  void CreateNewDocument(const std::vector<Data>& data);
  void OverwriteDocument(const std::vector<Data>& data);

 private:
  std::pair<int, int> FindHeader(const std::string& header);
  size_t CountNumberRows(const std::string& header);
  size_t CountNumberRows(size_t column_index);
  size_t CountRowsFromIndex(size_t column_index, size_t start_row);
  int CountNumberCols();
  size_t CountFilteredRows(const std::string& ignored_header,
    std::function<bool(const std::string&)> filter, size_t max_rows = 0);
  std::vector<size_t> ListColumnIdxs(const std::initializer_list<std::string>& columns);
  bool IsHeader(const std::string& record, const std::string& header);
  void InitHeaders(OpenXLSX::XLWorksheet& wks);
  std::vector<std::vector<std::string>> Initialize2DVector(int number_rows, int number_cols);
  void FillingTable(const std::vector<Data> data, size_t counter, OpenXLSX::XLWorksheet& wks);

  OpenXLSX::XLDocument doc_;
  OpenXLSX::XLWorksheet worksheet_;
};
} // namespase PriceAnalyzer
#endif  // PRICE_ANALYZER_EXCEL_PROCESSOR_H