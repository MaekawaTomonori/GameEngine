#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>

namespace Utils {
    std::string Convert(const std::wstring& _str);
    std::wstring Convert(const std::string& _str);
    void DisplayLastErr();
    void Alert(const std::string& _msg);
    std::string DateToString();
    std::string GenerateUniqueId();
    bool EqualsIgnoreCase(std::string _str1, std::string _str2);
    bool ConfirmDialog(const std::string& _msg);
};

#endif //UTILS_HPP
