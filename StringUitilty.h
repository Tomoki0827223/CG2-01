#ifndef STRING_UTILITY_H
#define STRING_UTILITY_H

#include <string>
#include <stringapiset.h>

namespace StringUitilty
{
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);
};

#endif // STRING_UTILITY_H
