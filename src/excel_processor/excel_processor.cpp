#include "excel_processor.h"

#include <set>

#include "string_utils.h"

namespace PriceAnalyzer {

ExcelProcessor::ExcelProcessor(const std::string& filename) {
  OpenFile(filename);
}

void ExcelProcessor::OpenFile(const std::string& filename) {
  doc_.open(filename);
  worksheet_ = doc_.workbook().worksheet(1);
}
  
std::vector<std::string> ExcelProcessor::GetSingleColumn(const std::string& header) {
  std::vector<std::string> titles;
  auto header_idxs = FindHeader(header);
  auto count_rows = CountNumberRows(header);
  titles.reserve(count_rows);
  auto end_row_idx = header_idxs.first + count_rows;
  for (size_t i = header_idxs.first + 1; i <= end_row_idx; ++i) {
    titles.emplace_back(worksheet_.cell(i, header_idxs.second).getString());
  }
  return titles;
}

std::vector<std::vector<std::string>> ExcelProcessor::GetSelectedColumns(
    const std::initializer_list<std::string>& columns,
    const std::string& filter_header,
    std::function<bool(const std::string&)> filter) {

  auto columns_idxs = ListColumnIdxs(columns);
  if (columns_idxs.empty()) return {};

  size_t number_rows = CountNumberRows(columns_idxs[0]);
  size_t number_filtered_rows = CountFilteredRows(filter_header, filter, number_rows);
  size_t number_cols = columns_idxs.size();
  auto filter_header_idx = FindHeader(filter_header);
  auto result = Initialize2DVector(number_filtered_rows, number_cols);

  size_t j = 0;
  size_t end_idx = number_rows + 1;
  std::string record;
  for (size_t i = 2; i <= end_idx; ++i) {
    record = worksheet_.cell(i, filter_header_idx.second).getString();
    record = ToLowerUtf8(record);
    if (filter(record)) {
      for (auto column : columns_idxs) {
        result[j].emplace_back(std::move(worksheet_.cell(i, column).getString()));
      }
      ++j;
    }
  }
  return result;
}

std::vector<std::vector<std::string>> ExcelProcessor::Initialize2DVector(int number_rows, int number_cols) {
  std::vector<std::vector<std::string>> result(number_rows);
  for (auto& row : result) {
    row.reserve(number_cols);
  }
  return result;
}

std::pair<int, int> ExcelProcessor::FindHeader(const std::string& header) {
  int columnCount = worksheet_.columnCount();
  for (int i = 1; i <= columnCount; ++i) {
    auto record = worksheet_.cell(1, i).getString();
    if (IsHeader(record, header)) {
      return std::pair<int, int>(1, i);
    }
  }
  throw std::runtime_error("The column with the specified header was not found in .xlsx file");
}

bool ExcelProcessor::IsHeader(const std::string& record, const std::string& header) {
  return ToLowerUtf8(record).find(header) != std::string::npos;
}

size_t ExcelProcessor::CountNumberRows(const std::string& header) {
  auto start_position = FindHeader(header);
  return CountRowsFromIndex(start_position.first + 1, start_position.second);
}

size_t ExcelProcessor::CountNumberRows(size_t column_index) {
  return CountRowsFromIndex(2, column_index);
}

size_t ExcelProcessor::CountRowsFromIndex(size_t start_row, size_t column_index) {
  size_t counter = 0;
  auto last_idx = worksheet_.rowCount();
  for (size_t i = start_row; i <= last_idx; ++i) {
    if(worksheet_.cell(i, column_index).getString().empty()) {
      break;
    } 
    ++counter;
  }
  return counter;
}

int ExcelProcessor::CountNumberCols() {
  int counter = 0;
  auto cols_count = worksheet_.columnCount();
  for (size_t i = 1; i <= cols_count; ++i) {
    if (worksheet_.cell(1, i).getString().empty()) {
      break;
    }
    ++counter;
  }
  return counter;
}

size_t ExcelProcessor::CountFilteredRows(const std::string& ignored_header,
std::function<bool(const std::string&)> filter, size_t max_rows) {
  auto start_position = FindHeader(ignored_header);
  auto number_rows = max_rows != 0 ? max_rows : CountNumberRows(ignored_header);
  int last_idx = number_rows + start_position.first;
  int counter = 0;
  for (int i = start_position.first + 1; i <= last_idx; ++i) {
    auto record = worksheet_.cell(i, start_position.second).getString();
    record = ToLowerUtf8(record);
    if (filter(record)) {
      ++counter;
    }
  }
  return counter;
}

std::vector<size_t> ExcelProcessor::ListColumnIdxs(
const std::initializer_list<std::string>& columns) {
  
  std::set<std::string> headers(columns);
  std::vector<size_t> result;
  result.reserve(headers.size());
  auto column_count = CountNumberCols();
  for (int i = 1; i <= column_count; ++i) {
    auto record = worksheet_.cell(1, i).getString();
    record = ToLowerUtf8(record);
    if (headers.count(record) != 0) {
      result.push_back(i);
    }
  }
  return result;
}

void ExcelProcessor::InitHeaders(OpenXLSX::XLWorksheet& wks) {
  wks.cell(1, 1).value() = "№ п/п";
  wks.cell(1, 2).value() = "Наименование товара";
  wks.cell(1, 3).value() = "product-id";
  wks.cell(1, 4).value() = "Минимальная цена";
  wks.cell(1, 5).value() = "Максимальная цена";
  wks.cell(1, 6).value() = "Средняя цена";
}

void ExcelProcessor::CreateNewDocument(const std::vector<Data>& data) {
  OpenXLSX::XLDocument doc;
  doc.create("final_" + doc_.name());
  auto wks = doc.workbook().worksheet(1);
  InitHeaders(wks);
  FillingTable(data, 1, wks);
  doc.save();
}

void ExcelProcessor::OverwriteDocument(const std::vector<Data>& data) {
  size_t total_titles = CountNumberRows(1);
  int counter = total_titles - data.size() + 1;
  FillingTable(data, counter, worksheet_);
  doc_.save();
}

void ExcelProcessor::FillingTable(const std::vector<Data> data,
size_t counter, OpenXLSX::XLWorksheet& wks) {
  int row_idx = counter + 1;
  for (const auto& item : data) {
    wks.cell(row_idx, 1).value() = std::to_string(counter);
    wks.cell(row_idx, 2).value() = item.title_;
    wks.cell(row_idx, 3).value() = item.id_;
    wks.cell(row_idx, 4).value() = item.min_;
    wks.cell(row_idx, 5).value() = item.max_;
    wks.cell(row_idx, 6).value() = item.average_;
    ++counter;
    ++row_idx;
  }
}
}
