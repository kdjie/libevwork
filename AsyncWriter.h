//============================================================================
// Name        : AsyncWriter.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "Writer.h"
#include "TimerHandler.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace evwork
{

	struct SPacket
	{
		std::string strIp;
		uint16_t uPort;

		std::string strPacket;
	};
	
	class CThreadWriter
		: public CWriter
	{
	public:
		virtual void send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize);
		virtual void flush();

	protected:
		boost::mutex m_lock;

		std::vector<SPacket*> m_vecPacket;
	};

	class CAsyncWriter
		: public IWriter
	{
	public:
		CAsyncWriter();
		virtual ~CAsyncWriter();

		virtual void send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize);
		virtual void flush();

	protected:

		void __cbFlush(int revents);
		void __onRead();
		bool __onHandlerFlush();

		CThreadWriter* __getThreadWriter();
		void __flushReal();

	protected:
		int m_fdFlush;
		bool m_bFlushing;

		THandle<CAsyncWriter, &CAsyncWriter::__cbFlush> m_hFlush;
		TimerHandler<CAsyncWriter, &CAsyncWriter::__onHandlerFlush> m_timerFlush;

		boost::thread_specific_ptr<CThreadWriter> m_tssThreadWriter;

		typedef std::vector<CThreadWriter*> VEC_PRHEADWRITER_t;
		VEC_PRHEADWRITER_t m_vecpThreadWriter;
		boost::mutex m_lockVecWriter;
	};

}
