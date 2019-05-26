//============================================================================
// Name        : DataHandler.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "DataHandler.h"

#include "libevwork/EVWork.h"
#include "Request.h"

using namespace evwork;
using namespace pb;

#undef DEFAULT_PACKET_LIMIT
#undef PRINT_INTERVAL

#define DEFAULT_PACKET_LIMIT 1024*1024*16
#define PRINT_INTERVAL		 1000*10

CDataHandler::CDataHandler()
: m_uProc(0)
, m_uBytes64(0)
, m_uPacketLimit(DEFAULT_PACKET_LIMIT)
{
	m_timerPrint.init(this);
	m_timerPrint.start(PRINT_INTERVAL);
}

CDataHandler::~CDataHandler()
{
	m_timerPrint.stop();
}

void CDataHandler::setPacketLimit(uint32_t uLimit)
{ 
	m_uPacketLimit = uLimit; 
}

int CDataHandler::onData(evwork::IConn* pConn, const char* pData, size_t uSize)
{
	int nProcessed = 0;

	while (uSize > 0)
	{
		if (uSize < PB_HEADER_SIZE)
			break;

		uint32_t uPktLen = Request::peekLen(pData);

		if ( (uPktLen < PB_HEADER_SIZE) || (m_uPacketLimit != (uint32_t)-1 && uPktLen > m_uPacketLimit) )
		{
			std::string strPeerIp = "";
			uint16_t uPeerPort = 0;
			pConn->getPeerInfo(strPeerIp, uPeerPort);

			LOG(Warn, "[pb::CDataHandler::%s] from:%s:%u recv a invalid packet, pktlen:%u", __FUNCTION__, strPeerIp.c_str(), uPeerPort, uPktLen);

			return -1;
		}

		if (uSize < uPktLen)
			break;

		__makeRequest(pData, uPktLen, pConn);

		pData += uPktLen;
		uSize -= uPktLen;

		nProcessed += (int)uPktLen;

		m_uProc++;
		m_uBytes64 += uPktLen;
	}

	return nProcessed;
}

void CDataHandler::__makeRequest(const char *pPacket, uint32_t uPktLen, evwork::IConn* pConn)
{
	Request request(pPacket, uPktLen, false);
	request.parseHeader();

	__requestDispatch(request, pConn);
}

void CDataHandler::__requestDispatch(Request& request, evwork::IConn* pConn)
{
	assert( getAppContext() );

	try
	{
		getAppContext()->RequestDispatch(request, pConn);
	}
	catch(std::exception& e)
	{
		LOG(Error, "[pb::CDataHandler::%s] catch exeception : %s", __FUNCTION__, e.what());
	}
}

bool CDataHandler::__onHandlerPrint()
{
	LOG(Notice, "[pb::CDataHandler::%s] proc:%u bytes:%llu", __FUNCTION__, m_uProc, m_uBytes64);

	m_uProc = 0;
	m_uBytes64 = 0;

	return true;
}
