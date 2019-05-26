//============================================================================
// Name        : TimeHandler.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVWork.h"

template <typename T>
struct bind_func
{
	typedef bool (T::*callback)();
};

template<typename T, typename bind_func<T>::callback ptr>
class TimerHandler
{
public:
	TimerHandler()
	{
		_x = NULL;
		m_bStart = false;
	}
	TimerHandler(T* x)
	{
		_x = x;
		m_bStart = false;
	}
	virtual ~TimerHandler()
	{
		stop();
	}

	void init(T* x)
	{
		_x = x;
	}

	void start(uint32_t timeout)
	{
		if (m_bStart)
			return;

		using namespace evwork;
		
		m_evTimer.data = this;
		ev_timer_init(&m_evTimer, TimerHandler::__cbTimer, timeout/(float)1000, timeout/(float)1000);
		ev_timer_start(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimer);

		m_bStart = true;
	}

	void stop()
	{
		if (m_bStart)
		{
			using namespace evwork;

			ev_timer_stop(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimer);
			m_bStart = false;
		}
	}

	void onTimer()
	{
		if (_x) 
		{		
			if (!(_x->*ptr)()) 
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
	T* _x;

	ev_timer m_evTimer;
	bool m_bStart;
};
