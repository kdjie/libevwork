//============================================================================
// Name        : AsyncDataHandler.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "DataHandler.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <list>

namespace pb
{
	class CAsyncDataHandler;

	template <typename T>
	class TRequestQueue
	{
	public:
		typedef T value_type;

		void push(const value_type& v)
		{
			boost::mutex::scoped_lock lk(m_mutex);

			m_listV.push_back(v);
			m_cond.notify_one();
		}

		value_type pop()
		{
			boost::mutex::scoped_lock lk(m_mutex);

			while (m_listV.empty())
				m_cond.wait(lk);

			value_type vT = m_listV.front();
			m_listV.pop_front();

			return vT;
		}

		size_t size()
		{
			boost::mutex::scoped_lock lk(m_mutex);

			return m_listV.size();
		}

	private:
		std::list<value_type> m_listV;
		boost::mutex m_mutex;
		boost::condition m_cond;
	};

	struct SRequestWrap
	{
		Request* pRequest;
		evwork::IConn* pConn;

		SRequestWrap()
		{
			reset(NULL, NULL);
		}
		SRequestWrap(Request* _pRequest, evwork::IConn* _pConn)
		{
			reset(_pRequest, _pConn);
		}

		void reset(Request* _pRequest, evwork::IConn* _pConn)
		{
			pRequest = _pRequest;
			pConn = _pConn;
		}
	};

	class CThreadWorker
	{
	public:
		CThreadWorker(CAsyncDataHandler* pParent);
		~CThreadWorker();

		void runLoop();

	private:
		CAsyncDataHandler* m_pParent;
		boost::thread m_thread;
	};

	class CAsyncDataHandler
		: public CDataHandler
	{
	public:
		friend class CThreadWorker;

		CAsyncDataHandler();
		virtual ~CAsyncDataHandler();

		void setWorkerCount(uint32_t uCount);
		void setWorking();

	protected:

		virtual void __makeRequest(const char *pPacket, uint32_t uPktLen, evwork::IConn* pConn);
		virtual void __requestDispatch(Request& request, evwork::IConn* pConn);

		virtual bool __onHandlerPrint();

	private:

		void __createThreadWorker();
		void __destroyThreadWorker();

	private:
		TRequestQueue<SRequestWrap> m_requestQueue;

		uint32_t m_uWorkerCount;
		std::vector<CThreadWorker*> m_vecWorker;
	};

}
