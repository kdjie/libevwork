//============================================================================
// Name        : AsyncDataHandler.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "AsyncDataHandler.h"

#include "libevwork/EVWork.h"
#include "Request.h"

#include <sys/syscall.h>

using namespace evwork;
using namespace js;

#undef DEF_WORKER_COUNT
#define DEF_WORKER_COUNT	20

CThreadWorker::CThreadWorker(CAsyncDataHandler* pParent)
: m_pParent(pParent)
, m_thread( boost::bind(&CThreadWorker::runLoop, this) )
{
}

CThreadWorker::~CThreadWorker()
{
	m_thread.join();
}

void CThreadWorker::runLoop()
{
	LOG(Notice, "[js::CThreadWorker::%s] tid:%u running...", __FUNCTION__, syscall(__NR_gettid));

	while (true)
	{
		SRequestWrap reqWrap = m_pParent->m_requestQueue.pop();

		if (reqWrap.pRequest == NULL)
			break;

		m_pParent->__requestDispatch(*reqWrap.pRequest, reqWrap.pConn);

		delete reqWrap.pRequest;
	}

	LOG(Notice, "[js::CThreadWorker::%s] tid:%u exit!", __FUNCTION__, syscall(__NR_gettid));
}

CAsyncDataHandler::CAsyncDataHandler()
: m_uWorkerCount(DEF_WORKER_COUNT)
{
}

CAsyncDataHandler::~CAsyncDataHandler()
{
	__destroyThreadWorker();
}

void CAsyncDataHandler::setWorkerCount(uint32_t uCount)
{
	m_uWorkerCount = uCount;
}

void CAsyncDataHandler::setWorking()
{
	__createThreadWorker();
}

void CAsyncDataHandler::__makeRequest(const char *pPacket, uint32_t uPktLen, evwork::IConn* pConn)
{
	Request* pRequest = new Request(pPacket, uPktLen, true);
	pRequest->parseHeader();

	m_requestQueue.push( SRequestWrap(pRequest, NULL) );
}

void CAsyncDataHandler::__requestDispatch(Request& request, evwork::IConn* pConn)
{
	assert( getAppContext() );

	try
	{
		getAppContext()->RequestDispatch(request, pConn);
	}
	catch(std::exception& e)
	{
		LOG(Error, "[js::CAsyncDataHandler::%s] catch exeception : %s", __FUNCTION__, e.what());
	}
}

bool CAsyncDataHandler::__onHandlerPrint()
{
	LOG(Notice, "[js::CDataHandler::%s] request:%u proc:%u bytes:%llu", __FUNCTION__, m_requestQueue.size(), m_uProc, m_uBytes64);

	m_uProc = 0;
	m_uBytes64 = 0;

	return true;
}

void CAsyncDataHandler::__createThreadWorker()
{
	for (uint32_t i = 0; i < m_uWorkerCount; ++i)
	{
		m_vecWorker.push_back( new CThreadWorker(this) );
	}
}

void CAsyncDataHandler::__destroyThreadWorker()
{
	for (uint32_t i = 0; i < m_vecWorker.size(); ++i)
	{
		m_requestQueue.push( SRequestWrap() );
	}

	for (std::vector<CThreadWorker*>::iterator iter = m_vecWorker.begin(); iter != m_vecWorker.end(); ++iter)
	{
		delete (*iter);
	}

	m_vecWorker.clear();
}
