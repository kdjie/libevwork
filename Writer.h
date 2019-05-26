//============================================================================
// Name        : Writer.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVComm.h"

namespace evwork
{

	class CWriter
		: public IWriter
	{
	public:
		CWriter();
		virtual ~CWriter();

		virtual void send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize);
		virtual void flush() {}
	};

}
