#pragma once
#include <string>
#include "HAL/PlatformType.h"


inline std::wstring StringToWString(const std::string& String)
{
    if (String.empty())
    {
        return std::wstring{};
    }

    const int32 Size = ::MultiByteToWideChar(CP_UTF8, 0, String.c_str(), static_cast<int32>(String.size()), nullptr, 0);
    std::wstring WString(Size, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, String.c_str(), static_cast<int32>(String.size()), WString.data(), Size);
    return WString;
}

inline std::string WStringToString(const std::wstring& WString)
{
    if (WString.empty())
    {
        return std::string{};
    }

    const int32 Size = ::WideCharToMultiByte(CP_UTF8, 0, WString.c_str(), static_cast<int32>(WString.size()), nullptr, 0, nullptr, nullptr);
    std::string String(Size, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, WString.c_str(), static_cast<int32>(WString.size()), String.data(), Size, nullptr, nullptr);
    return String;
}
