#include "string_utils.h"

#include <unordered_map>
#include <iostream> // !!!

namespace PriceAnalyzer {

std::string ToLowerUtf8(const std::string& str) {
	static const std::unordered_map<std::string, std::string> kToLower {
		{"А", "а"}, {"Б", "б"}, {"В", "в"}, {"Г", "г"},
		{"Д", "д"}, {"Е", "е"}, {"Ё", "ё"}, {"Ж", "ж"},
		{"З", "з"}, {"И", "и"}, {"Й", "й"}, {"К", "к"},
		{"Л", "л"}, {"М", "м"}, {"Н", "н"}, {"О", "о"},
		{"П", "п"}, {"Р", "р"}, {"С", "с"}, {"Т", "т"},
		{"У", "у"}, {"Ф", "ф"}, {"Х", "х"}, {"Ц", "ц"},
		{"Ч", "ч"}, {"Ш", "ш"}, {"Щ", "щ"}, {"Ъ", "ъ"},
		{"Ы", "ы"}, {"Ь", "ь"}, {"Э", "э"}, {"Ю", "ю"},
		{"Я", "я"}
	};

	std::string result;
	result.reserve(str.size());
	size_t i = 0;
	while (i < str.size()) {
		auto ch = static_cast<unsigned char>(str[i]);
		if (ch == 0xD0 || ch == 0xD1) {
			std::string utf8_char = str.substr(i, 2);
			auto it = kToLower.find(utf8_char);
			result = it != kToLower.end() ? result + it->second : result + utf8_char;
			i += 2;
		} else {
			result += std::tolower(str[i]);
			i += 1;
		}
	}
	return result;
}

static size_t CountAsciiCharacters(const std::string& str) {
	size_t ascii_count  = 0;
	for (unsigned char ch : str) {
		// Если значение больше 127, значит рассматривается символ utf, т.к.
		// символы utf начинают отсчитываться с C2(hex) и 80(hex), что больше 127
		if (ch < 128) {  
			ascii_count++;
		}
	}
	return ascii_count ;
}

std::string ToRuFromAscii(const std::string& str) {
	static const std::unordered_map<char, std::string> kToRuFromAscii {
		{'a', "а"}, {'x', "х"}, {'e', "е"}, {'c', "с"},
		{'t', "т"}, {'b', "в"}, {'y', "у"}, {'h', "н"},
		{'m', "м"}, {'k', "к"}, {'o', "о"}, {'p', "р"}
	};
	std::string result;
	auto ascii_count = CountAsciiCharacters(str);
	result.reserve(str.size() + ascii_count);
	for (auto ch : str) {
		auto it = kToRuFromAscii.find(ch);
		if (it != kToRuFromAscii.end()) {
			result.append(it->second);
		} else {
			result.append(1, ch);
		}
	}
	return result;
}

std::string ToUrlFormat(const std::string& name) {
	std::string result;
	// В названии n байт. Каждый символ при переводе в hex теперь занимает по 2 байта + 1 байт занимает '%'
	size_t result_size = name.size() * 3;
	result.reserve(result_size);
	char buf[4];
	for (unsigned char c : name) {
		if (isdigit(c)) {
			result += c;
		} else {
			sprintf(buf, "%%%X", c);
			result += buf;
		}
	}
	return result;
}

static size_t CountingTokens(const std::string& str, char delimiter) {
	size_t count = 0;
	for (auto ch : str) {
		if (ch == delimiter) {
			++count;
		}
	}
	return count + 1;
}

std::vector<std::string> Split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::string::size_type start = 0;
	std::string::size_type end = str.find(delimiter);

	tokens.reserve(CountingTokens(str, delimiter));
	while (end != std::string::npos) {
		tokens.emplace_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}
	tokens.emplace_back(str.substr(start));

	return tokens;
}

int CountIdenticalSubstring(const std::vector<std::string>& str1, const std::vector<std::string>& str2) {
	int counter = 0;
	for (const auto& substr1 : str1) {
		if (substr1.size() <= 1) continue;
		for (const auto& substr2 : str2) {
			if (substr2.size() <= 1) continue;
			if (substr1.find(substr2) == std::string::npos 
			    && substr2.find(substr1) == std::string::npos) {
				continue;
			}
			counter = substr1 == substr2 ? counter + 2 : counter + 1;
			break;
		}
	}
	return counter;
}

void RemoveBackPoint(std::string& str) {
  if (str.back() == '.') {
    str.pop_back();
  }
}

} // namespace PriceAnalyzer