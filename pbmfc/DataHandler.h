//============================================================================
// Name        : DataHandler.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "libevwork/EVComm.h"
#include "libevwork/TimerHandler.h"

#include "FormDef.h"

namespace pb
{

	class CDataHandler
		: public evwork::IDataEvent
		, public IAppContextAware
	{
	public:
		CDataHandler();
		virtual ~CDataHandler();

		void setPacketLimit(uint32_t uLimit);

		virtual int onData(evwork::IConn* pConn, const char* pData, size_t uSize);

	protected:

		virtual void __makeRequest(const char *pPacket, uint32_t uPktLen, evwork::IConn* pConn);
		virtual void __requestDispatch(Request& request, evwork::IConn* pConn);

		virtual bool __onHandlerPrint();

	protected:
		uint32_t m_uProc;
		uint64_t m_uBytes64;
		uint32_t m_uPacketLimit;

		TimerHandler<CDataHandler, &CDataHandler::__onHandlerPrint> m_timerPrint;
	};

}
