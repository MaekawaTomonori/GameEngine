#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>


namespace Utils {
    std::string Convert(const std::wstring& str);
    std::wstring Convert(const std::string& str);
    void DisplayLastErr();
    void Alert(const std::string& msg);
    std::string DateToString();
    std::string GenerateUniqueId();
    bool EqualsIgnoreCase(std::string str1, std::string str2);
    bool ConfirmDialog(const std::string& _msg);
};

#endif //UTILS_HPP
