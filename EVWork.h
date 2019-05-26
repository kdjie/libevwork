//============================================================================
// Name        : EVWork.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVComm.h"
#include "Logger.h"
#include "ListenConn.h"
#include "ClientConn.h"
#include "Writer.h"
#include "ConnManager.h"

#include <boost/thread/tss.hpp>

namespace evwork
{
	struct SEVParam
	{
		uint32_t uConnTimeout;

		SEVParam()
		{
			uConnTimeout = (uint32_t)-1;
		}
	};

	class CThreadEnv
	{
	public:
		CThreadEnv();
		virtual ~CThreadEnv();

		void setEVLoop(CEVLoop* p);
		CEVLoop* getEVLoop();

		void setLogger(ILogReport* p);
		ILogReport* getLogger();

		void setLinkEvent(ILinkEvent* p);
		ILinkEvent* getLinkEvent();

		void setDataEvent(IDataEvent* p);
		IDataEvent* getDataEvent();

		void setWriter(IWriter* p);
		IWriter* getWriter();

		void setConnManager(IConnManager* p);
		IConnManager* getConnManager();

		SEVParam& getEVParam();

	private:
		CEVLoop* m_pEVLoop;
		ILogReport* m_pLogger;
		ILinkEvent* m_pLinkEvent;
		IDataEvent* m_pDataEvent;
		IWriter* m_pWriter;
		IConnManager* m_pConnManager;
		SEVParam m_evParam;
	};

	class CEnv
	{
	public:
		static CThreadEnv* getThreadEnv();

	private:
		static boost::thread_specific_ptr<CThreadEnv> m_tssEnv;
	};

	#define LOG(l,f,...) CEnv::getThreadEnv()->getLogger()->log(l, f, ##__VA_ARGS__)

}
