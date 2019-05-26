//============================================================================
// Name        : AsyncWriter.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "AsyncWriter.h"

#include "EVWork.h"

#include <sys/eventfd.h>

using namespace evwork;

void CThreadWriter::send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize)
{
	SPacket* pPacket = new SPacket();
	pPacket->strIp = strIp;
	pPacket->uPort = uPort;
	pPacket->strPacket.assign(pData, uSize);

	boost::mutex::scoped_lock lk(m_lock);

	m_vecPacket.push_back(pPacket);
}

void CThreadWriter::flush()
{
	boost::mutex::scoped_lock lk(m_lock);

	for (std::vector<SPacket*>::iterator iter = m_vecPacket.begin(); iter != m_vecPacket.end(); ++iter)
	{
		SPacket* pPacket = *iter;

		CWriter::send(pPacket->strIp, pPacket->uPort, pPacket->strPacket.data(), pPacket->strPacket.size());

		delete pPacket;
	}

	m_vecPacket.clear();
}

CAsyncWriter::CAsyncWriter()
: m_bFlushing(false)
, m_hFlush(this)
{
	// 使用eventfd作为事件通知机制
	m_fdFlush = eventfd(0, EFD_NONBLOCK);
	assert(m_fdFlush != -1);

	m_hFlush.setEv(EV_READ);
	m_hFlush.setFd(m_fdFlush);

	CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hFlush);

	m_timerFlush.init(this);
	m_timerFlush.start(1000);
}

CAsyncWriter::~CAsyncWriter()
{
	if (m_fdFlush != -1)
	{
		CEnv::getThreadEnv()->getEVLoop()->delHandle(&m_hFlush);

		close(m_fdFlush);
		m_fdFlush = -1;
	}

	m_timerFlush.stop();
}

void CAsyncWriter::send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize)
{
	__getThreadWriter()->send(strIp, uPort, pData, uSize);
}

void CAsyncWriter::flush()
{
	if (!m_bFlushing)
	{
		uint64_t u = 0x11111111;
		::write(m_fdFlush, &u, sizeof(uint64_t));

		m_bFlushing = true;
	}
}

void CAsyncWriter::__cbFlush(int revents)
{
	if ((revents & EV_READ) == EV_READ)
	{
		//LOG(Info, "[CAsyncWriter::%s] fd:[%d] EV_READ...", __FUNCTION__, m_fdFlush);

		__onRead();
	}
}

void CAsyncWriter::__onRead()
{
	uint64_t u;
	int ret = ::read(m_fdFlush, &u, sizeof(uint64_t));
	if (ret == sizeof(uint64_t))
	{
		__flushReal();
	}
}

bool CAsyncWriter::__onHandlerFlush()
{
	__flushReal();

	return true;
}

CThreadWriter* CAsyncWriter::__getThreadWriter()
{
	if (m_tssThreadWriter.get() == NULL)
	{
		boost::mutex::scoped_lock lk(m_lockVecWriter);

		CThreadWriter* pThreadWriter = new CThreadWriter();

		m_vecpThreadWriter.push_back(pThreadWriter);

		m_tssThreadWriter.reset(pThreadWriter);
	}

	return m_tssThreadWriter.get();
}

void CAsyncWriter::__flushReal()
{
	boost::mutex::scoped_lock sl(m_lockVecWriter);

	for(VEC_PRHEADWRITER_t::iterator iter = m_vecpThreadWriter.begin(); iter != m_vecpThreadWriter.end(); ++iter)
	{
		(*iter)->flush();
	}

	m_bFlushing = false;
}
