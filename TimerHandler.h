//============================================================================
// Name        : TimeHandler.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVWork.h"

template<typename T, bool (T::*callback)()>
class TimerHandler
{
public:
	TimerHandler()
	{
		m_pObj = NULL;
		m_bStart = false;
	}
	TimerHandler(T* pObj)
	{
		m_pObj = pObj;
		m_bStart = false;
	}
	virtual ~TimerHandler()
	{
		stop();
	}

	void init(T* pObj)
	{
		m_pObj = pObj;
	}

	void start(uint32_t uTimeoutMs)
	{
		if (!m_bStart)
		{
			using namespace evwork;

			m_evTimer.data = this;
			ev_timer_init(&m_evTimer, TimerHandler::__cbTimer, uTimeoutMs / (float)1000, uTimeoutMs / (float)1000);
			ev_timer_start(CEnv::getThreadEnv()->getEVLoop()->getLoop(), &m_evTimer);

			m_bStart = true;
		}
	}

	void stop()
	{
		if (m_bStart)
		{
			using namespace evwork;

			ev_timer_stop(CEnv::getThreadEnv()->getEVLoop()->getLoop(), &m_evTimer);
			m_bStart = false;
		}
	}

	void refresh()
	{
		if (m_bStart)
		{
			using namespace evwork;

			ev_timer_again(CEnv::getThreadEnv()->getEVLoop()->getLoop(), &m_evTimer);
		}
	}

	void onTimer()
	{
		if (m_pObj)
		{		
			if (!(m_pObj->*callback)())
			{
				stop();
			}
		}
	}

private:

	static void __cbTimer(struct ev_loop *loop, struct ev_timer *w, int revents)
	{
		TimerHandler* pThis = (TimerHandler*)w->data;

		pThis->onTimer();
	}

protected:
	T* m_pObj;

	ev_timer m_evTimer;
	bool m_bStart;
};
