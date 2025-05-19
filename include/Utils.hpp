#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>


namespace Utils {
	std::string Convert(const std::wstring& str);
	std::wstring Convert(const std::string& str);
	void DisplayLastErr();

	std::string GetCurrentTime();
	std::string GetCurrentDate();
};



#endif //UTILS_HPP
