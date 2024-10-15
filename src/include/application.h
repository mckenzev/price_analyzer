#include <conio.h>

#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <list>

#include "data_analyzer.h"
#include "excel_processor.h"
#include "html_reader.h"
#include "http_client.h"
#include "string_utils.h"

namespace PriceAnalyzer {
class Application {
 public:
  explicit Application(const std::string& mode,
                       const std::string& filename = "monitoring.xlsx");
  void Run();

 private:
  const std::string& filename_;
  const std::string& mode_;
  std::atomic<bool> stop_signal_;
  HttpClient client_;
  ExcelProcessor excel_proc_;
  std::vector<Data> full_result_;
  std::list<std::pair<std::string, std::string>> partial_result_;

  void PrimaryProcessing();
  void PostProcessing();
  void MonitorKeyboard();
  void ProcessPartialResults();
  void ProcessTitle(std::string& title, std::vector<std::string>& new_titles);
  void FinalizeResults(std::vector<std::string>& titles);
  std::string FindProductId(const std::vector<std::string>& title_vect);
  std::pair<std::string, int> FindBestResult(
    const std::vector<std::string>& title_vect,
    const std::vector<std::pair<std::string, std::string>>& pairs);
  std::string GetProductId(std::vector<std::string>& title_vect);
  std::string GetProductId(std::vector<std::string>& title_vect,
                           const std::string& url_format);
  std::string FindCsrfToken();
  std::vector<double> FillingTableOfPrices();
  std::vector<double> GetPricesTable(const std::string& id);
  std::string Normalization(const std::string& title);
  void AnalyzeAndFetchData(std::vector<std::string>& titles);
  std::vector<std::vector<std::string>> GetTitlesAndId();
  void SplitPartialResultsAndTitles(std::vector<std::string>& titles);
  void EnsureFileExists(const std::string& filename);
};
}  // namespace PriceAnalyzer