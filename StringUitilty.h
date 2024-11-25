#pragma once
#include <string>
#include <stringapiset.h>

#ifndef _AMD64_
#define _AMD64_
#endif

namespace StringUitilty
{
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);
};

