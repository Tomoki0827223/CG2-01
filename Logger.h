#pragma once
#include <string>
#include <ostream>
#include <debugapi.h>
#include <xstring>

#ifndef _AMD64_
#define _AMD64_
#endif

namespace Logger
{
	void Log(const std::string& masege);
};

