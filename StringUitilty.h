#pragma once
#include <string>
#include <stringapiset.h>

namespace StringUitilty
{
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);
};

