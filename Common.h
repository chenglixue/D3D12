#pragma once

#include "pch.h"

namespace Util
{
    inline std::wstring UTF8ToWideString(const std::string& str)
    {
        wchar_t wstr[MAX_PATH];
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), -1, wstr, MAX_PATH))
            wstr[0] = L'\0';
        return wstr;
    }

    inline std::string WideStringToUTF8(const std::wstring& wstr)
    {
        char str[MAX_PATH];
        if (!WideCharToMultiByte(CP_ACP, MB_PRECOMPOSED, wstr.c_str(), -1, str, MAX_PATH, nullptr, nullptr))
            str[0] = L'\0';
        return str;
    }
}