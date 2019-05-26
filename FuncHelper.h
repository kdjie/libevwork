//============================================================================
// Name        : FuncHelper.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include <iostream>
#include <string>

#include <sys/resource.h>

namespace evwork
{

	inline void setRLimitCore(uint32_t uLimit)
	{
		rlimit rl;
		rl.rlim_cur = uLimit;
		rl.rlim_max = uLimit;
		setrlimit(RLIMIT_CORE, &rl);
	}

	inline std::string toString(const char* szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);

		char szLine[1024] = {0};
		vsnprintf(szLine, sizeof(szLine), szFormat, va);

		va_end(va);

		return std::string(szLine);
	}

	inline int random(int nBase, int nMax, int nFactor = 0)
	{
		srand((unsigned int)time(NULL) + rand() + nFactor);
		return nBase + rand() % (nMax - nBase + 1);
	}

}
