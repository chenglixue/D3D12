#pragma once

#include "pch.h"

inline std::string WStringToString(const std::wstring& input)
{
	std::string res;

	int len = WideCharToMultiByte(
		CP_ACP, 
		0, 
		input.c_str(),
		input.size(),
		nullptr, 
		0, 
		nullptr, 
		nullptr);
	if (len <= 0)
	{
		return res;
	}

	char* buffer = new char[len + 1];
	if (buffer == nullptr)
	{
		return res;
	}

	WideCharToMultiByte(
		CP_ACP, 
		0, 
		input.c_str(), 
		input.size(), 
		buffer, 
		len, 
		nullptr, 
		nullptr);

	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;

	return res;
}

inline std::wstring StringToWString(const std::string& input)
{
	std::wstring res;

	int len = MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), nullptr, 0);
	if (len < 0)
	{
		return res;
	}

	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == nullptr)
	{
		return res;
	}

	MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), buffer, len);

	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;

	return res;
}