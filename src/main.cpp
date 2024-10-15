#include "application.h"

#include <string>
#include <Windows.h>

int main(int argc, char* argv[]) {
  SetConsoleOutputCP(CP_UTF8);
  std::string filename = "monitoring.xlsx";
  if (argc > 1) {
    PriceAnalyzer::Application A(argv[1], filename);
    try {
      A.Run();
    } catch (const std::runtime_error& e) {
      std::cerr << "Файл, который вы пытаетесь открыть, поврежден " << e.what() << '\n';
    }
  } else {
    std::cout << "Укажите режим работы программы." << std::endl;
    std::cout << "primary для первичной обработки списка и создания final_monitoring.xlsx." << std::endl;
    std::cout << "post для вторичной и более обработки строки из файла final_monitoring.xlsx." << std::endl;
  }
  return 0;
}