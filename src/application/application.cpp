#include "application.h"

#include <filesystem>

namespace PriceAnalyzer {
  
Application::Application(const std::string& mode, const std::string& filename)
    : filename_(filename), mode_(mode) {
  stop_signal_.exchange(false);
}

void Application::Run() {
  try {
    if (mode_ == "primary") {
      EnsureFileExists(filename_);
      excel_proc_.OpenFile(filename_);
      PrimaryProcessing();
    } else if (mode_ == "post") {
      auto final_file = "final_" + filename_;
      EnsureFileExists(final_file);
      excel_proc_.OpenFile(final_file);
      PostProcessing();
    } else {
      std::cerr << "Режим " << mode_ << " не предусмотрен" << std::endl;
      std::cerr << "В данный момент есть режимы \"primary\" и \"post\"" << std::endl; 
    }
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void Application::PrimaryProcessing() {
  auto titles = excel_proc_.GetSingleColumn("наименование товара");
  full_result_.reserve(titles.size());
  std::thread keyboard_thread(&Application::MonitorKeyboard, this);

  AnalyzeAndFetchData(titles);
  keyboard_thread.join();

  excel_proc_.CreateNewDocument(full_result_);
}

void Application::PostProcessing() {
  std::vector<std::string> titles;
  SplitPartialResultsAndTitles(titles);
  full_result_.reserve(titles.size() + partial_result_.size());
  std::thread keyboard_thread(&Application::MonitorKeyboard, this);

  AnalyzeAndFetchData(titles);
  keyboard_thread.join();

  excel_proc_.OverwriteDocument(full_result_);
}

std::vector<std::vector<std::string>> Application::GetTitlesAndId() {
  auto result = excel_proc_.GetSelectedColumns({"наименование товара", "product-id"}, "минимальная цена", [](const std::string& record) {
    return record == "0" || record.empty();
  });
  return result;
}

void Application::SplitPartialResultsAndTitles(std::vector<std::string>& titles) {
  auto data = GetTitlesAndId();
  size_t counter = 0;
  for (; counter < data.size(); ++counter) {
    if (data[counter][1] == "-" || data[counter][1].empty()) {
      break;
    }
    partial_result_.emplace_back(std::move(data[counter][0]), std::move(data[counter][1]));
  }
  size_t titles_num = data.size() - counter;
  titles.reserve(titles_num);
  for (; counter < data.size(); ++counter) {
    titles.emplace_back(std::move(data[counter][0]));
  }
}

void Application::AnalyzeAndFetchData(std::vector<std::string>& titles) {
  std::vector<std::string> new_titles;
  new_titles.reserve(titles.size());

  while (!stop_signal_.load()) {
    if (titles.empty() && partial_result_.empty()) {
      stop_signal_.store(true);
      break;
    }

    std::cout << "Начало цикла" << std::endl;

    ProcessPartialResults();
    for (auto& title : titles) {
      ProcessTitle(title, new_titles);
    }
    titles = std::move(new_titles);
    new_titles.clear();
  }
  FinalizeResults(titles);
}

void Application::ProcessPartialResults() {
  for (auto it = partial_result_.begin(); it != partial_result_.end();) {
    auto table = GetPricesTable(it->second);

    if (!table.empty()) {
      std::cout << "Выполнен анализ для " << it->first << std::endl;
      full_result_.emplace_back(Data(std::move(it->first), std::move(it->second), table));
      it = partial_result_.erase(it);
      // continue;
    } else { // Сверху убрал continue, снизу ++it занес под else
      std::cout << "Для " << it->first << " не удалось получить куки" << std::endl;
      ++it;
    }
      // ++it;
  }
}

void Application::ProcessTitle(std::string& title, std::vector<std::string>& new_titles) {
  auto changed_title = Normalization(title);
  auto title_vect = Split(changed_title, ' ');
  auto id = GetProductId(title_vect);

  if (id.empty()) {
    new_titles.emplace_back(title);
    return;
  }

  auto table = GetPricesTable(id);
  if (table.empty()) {
    partial_result_.emplace_back(title, id);
  } else {
    std::cout << "Выполнен анализ для " << title << std::endl;
    full_result_.emplace_back(Data(std::move(title), std::move(id), table));
  }
}

void Application::FinalizeResults(std::vector<std::string>& titles) {
  for (auto it = partial_result_.begin(); it != partial_result_.end();) {
    full_result_.emplace_back(Data(std::move(it->first), std::move(it->second), {}));
    it = partial_result_.erase(it);
  }

  for (auto& title : titles) {
    full_result_.emplace_back(Data(std::move(title), "-", {}));
  }
}

std::string Application::GetProductId(std::vector<std::string>& title_vect) {
  std::string id;
  std::string url_format;

  RemoveBackPoint(title_vect[0]);
  // Выделение памяти под 1 (или под 2 слова и '+', если второе слово имеется) и '\0'
  size_t reserve_size = title_vect[0].size() + (title_vect.size() > 1 ? title_vect[1].size() + 1 : 0) + 1;
  url_format.reserve(reserve_size);
  url_format.append(ToUrlFormat(title_vect[0]));
  if (title_vect.size() > 1) {
    RemoveBackPoint(title_vect[1]);
    url_format += '+';
    url_format.append(ToUrlFormat(title_vect[1]));
  }

  id = GetProductId(title_vect, url_format);

  return id;
}

std::string Application::GetProductId(std::vector<std::string>& title_vect, const std::string& url_format) {
  std::string code;
  std::string id;

  client_.GetSearcherPage(url_format);
  code = client_.GetBuffer();
  id = FindProductId(title_vect);
  // Если удалось найти id при запросе из 2-х слов, то завершить поиск
  // иначе повторить попытку с запросом из 1-ого слова
  if (!id.empty()) return id;

  // Рекурсия для уменьшения запроса (Тауфон 4% -> Тауфон)
  auto new_url = ToUrlFormat(title_vect[0]);  
  if (url_format != new_url) {
    return GetProductId(title_vect, new_url);
  }

  return id;
}

std::string Application::FindProductId(const std::vector<std::string>& title_vect) {
  const auto& code = client_.GetBuffer();
  if (code.empty()) return "";

  HtmlReader reader(code);
  auto size = reader.Count("p-item");
  // Вектор пар (id, название)
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.reserve(size);
  for (int i = 0; i < size; ++i) {
    auto id = reader.Find("data-product-id");
    auto medicin_name = reader.Find("alt");
    pairs.emplace_back(id, medicin_name);
  }
  // Пара (id, количество совпадений с продуктом для поиска)
  auto best_result = FindBestResult(title_vect, pairs);
  return best_result.first;
}

std::pair<std::string, int> Application::FindBestResult(
  const std::vector<std::string>& title_vect,
  const std::vector<std::pair<std::string, std::string>>& pairs) {
  std::pair<std::string, int> best_result = {"", 0};
  for (const auto& pair : pairs) {
    std::string tmp_str = ToLowerUtf8(pair.second);
    tmp_str = ToRuFromAscii(tmp_str);
    auto tmp_vec = Split(tmp_str, ' ');
    // Первые слова в наименовании всегда совпадают, иначе рассматривается аналог необходимого препарата
    // Нурофен != Ибупрофен
    if (title_vect[0] != tmp_vec[0]) continue;

    int res_counting = CountIdenticalSubstring(title_vect, tmp_vec);
    if (best_result.second < res_counting) {
      best_result.first = pair.first;
      best_result.second = res_counting;
    }
  }
  return best_result;
}

std::vector<double> Application::FillingTableOfPrices() {
  const auto& code = client_.GetBuffer();
  if (code.empty()) return {};

  HtmlReader reader(code);
  std::vector<double> prices;
  size_t size = reader.Count("data-price");
  prices.reserve(size);
  while (prices.size() != size) {
    auto price_str = reader.Find("data-price");
    double price = std::stod(price_str);
    prices.push_back(price);
  }
  return prices;
}

std::string Application::FindCsrfToken() {
  const auto& code = client_.GetBuffer();
  if (code.empty()) return "";

  HtmlReader reader(code);
  auto token = reader.Find("csrf-token", HtmlReader::FinderMode::CONTENT);
  return token;
}

std::vector<double> Application::GetPricesTable(const std::string& id) {
  client_.SetId(id);
  client_.GetMainPage();

  auto token = FindCsrfToken();
  if (token.empty()) return {};

  client_.SetCsrfToken(token);
  client_.GetHtmlTable();
  auto table = FillingTableOfPrices();

  return table;
}

std::string Application::Normalization(const std::string& title) {
  auto changed_title = ToLowerUtf8(title);
  changed_title = ToRuFromAscii(changed_title);

  std::transform(changed_title.begin(), changed_title.end(), changed_title.begin(), [](char ch) -> char {
    if (ch == '+' || ch == '/') return ' ';
    if (ch == ',') return '.';
    return ch;
  });

  return changed_title;
}

void Application::MonitorKeyboard() {
  while (!stop_signal_.load()) {
    if (!_kbhit()) continue;
    char ch = _getch();
    if (ch == 'q' || ch == 'Q') {
      std::cout << "Сигнал для выхода получен\n" << std::endl;
      stop_signal_.store(true);
    }
  }
}

void Application::EnsureFileExists(const std::string& filename) {
  if (!std::filesystem::exists(filename)) {
    throw std::runtime_error("Не удается найти файл " + filename);
  }
}

} // namespace PriceAnalyzer