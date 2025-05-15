//
// Created by tomo- on 25/05/08.
//

#include "include/Utils.hpp"
#include <Windows.h>
#include <string>
#include <algorithm>

namespace Utils {
	std::string Convert(const std::wstring& str) {
		if (str.empty()){
			return {};
		}

		auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
		if (sizeNeeded == 0){
			return {};
		}
		std::string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, nullptr, nullptr);
		return result;
	}

	std::wstring Convert(const std::string& str) {
		if (str.empty()){
			return {};
		}

		auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), static_cast<int>(str.size()), nullptr, 0);
		if (sizeNeeded == 0){
			return {};
		}
		std::wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), static_cast<int>(str.size()), result.data(), sizeNeeded);
		return result;
	}
}
