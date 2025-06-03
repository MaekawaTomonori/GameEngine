#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>


namespace Utils {
	std::string Convert(const std::wstring& str);
	std::wstring Convert(const std::string& str);
	void DisplayLastErr();
	void Alert(const std::string& msg);
	std::string DateToString();
};



#endif //UTILS_HPP
