CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17
INCLUDE_PATH = -I./auxiliary_libs/OpenXLSX \
               -I./auxiliary_libs/curl_x64-windows/include/curl \
               -I./include
LIB_PATH = -L./auxiliary_libs/OpenXLSX -L./auxiliary_libs/curl_x64-windows/lib
LIBS = -lOpenXLSX -lcurl
SOURCE = main.cpp \
         http_client/http_client.cpp \
         html_reader/html_reader.cpp \
         excel_processor/excel_processor.cpp \
         string_utils/string_utils.cpp \
         application/application.cpp
EXECUTE = run.exe
all: $(EXECUTE)

build: $(SOURCE)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) $(SOURCE) $(LIB_PATH) $(LIBS) -o $(EXECUTE)

clean:
	rm -rf *.o $(EXECUTE) test.exe
