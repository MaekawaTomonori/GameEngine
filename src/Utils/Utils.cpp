#include "include/Utils.hpp"
#include <Windows.h>
#include <chrono>

namespace Utils {
    std::string Convert(const std::wstring& str) {
        if (str.empty())return {};

        auto size_needed = WideCharToMultiByte(CP_UTF8,0,str.data(),static_cast<int>(str.size()),nullptr, 0, nullptr,nullptr);
        if (size_needed == 0) return {};

        std::string result(size_needed, 0);
        if (!WideCharToMultiByte(CP_UTF8,0, str.data(),static_cast<int>(str.size()),result.data(), size_needed,nullptr, nullptr))return {};

        return result;
    }

    std::wstring Convert(const std::string& str) {
        if (str.empty())return {};

        auto size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(),static_cast<int>(str.size()),nullptr, 0);
        if (size_needed == 0)return {};

        std::wstring result(size_needed, 0);
        if (!MultiByteToWideChar(CP_UTF8, 0, str.data(),static_cast<int>(str.size()),result.data(), size_needed))return {};

        return result;
    }

    void DisplayLastErr() {
#ifdef _WIN32
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError();

        if (dw == 0)return;

        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&lpMsgBuf),
            0, nullptr) == 0){
            MessageBox(nullptr, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
            return;
        }
        MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), TEXT("Error"), MB_OK);
        LocalFree(lpMsgBuf);
#endif
    }

    void Alert(const std::string &msg) {
#ifdef _WIN32
        MessageBoxA(nullptr, msg.c_str(), msg.c_str(), MB_ICONSTOP);
#endif
    }

    std::string DateToString() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto now_time_t = system_clock::to_time_t(now);
        auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm_snapshot;
        #if defined(_WIN32) || defined(_WIN64)
        // Use localtime_s for Windows/MSVC
        localtime_s(&tm_snapshot, &now_time_t);
        #else
        // Use localtime_r for Linux/macOS
        localtime_r(&now_time_t, &tm_snapshot);
        #endif

        std::ostringstream oss;
        oss << std::put_time(&tm_snapshot, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setw(3) << std::setfill('0') << now_ms.count();
        return oss.str();
    }

    #include <rpc.h>
    #pragma comment(lib, "rpcrt4.lib")

    std::string GenerateUniqueId() {
        UUID uuid;
        UuidCreate(&uuid);
        RPC_CSTR szUuid = nullptr;
        UuidToStringA(&uuid, &szUuid);
        struct UUIDCleaner{
            RPC_CSTR& ptr;
            ~UUIDCleaner() {
                if (ptr)RpcStringFreeA(&ptr);
            }
        } cleaner {szUuid};
        return reinterpret_cast<char*>(szUuid);
    }

    bool EqualsIgnoreCase(std::string str1, std::string str2) {
        return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(),
                                                        [](char a, char b){ return tolower(a) == tolower(b); });
    }
}
