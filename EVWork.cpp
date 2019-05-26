//============================================================================
// Name        : EVWork.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "EVWork.h"

using namespace evwork;

boost::thread_specific_ptr<CThreadEnv> CEnv::m_tssEnv;

CThreadEnv::CThreadEnv()
: m_pEVLoop(NULL)
, m_pLogger(NULL)
, m_pLinkEvent(NULL)
, m_pDataEvent(NULL)
, m_pWriter(NULL)
, m_pConnManager(NULL)
{
}
CThreadEnv::~CThreadEnv()
{
}

void CThreadEnv::setEVLoop(CEVLoop* p)
{
	m_pEVLoop = p;
}
CEVLoop* CThreadEnv::getEVLoop()
{
	return m_pEVLoop;
}

void CThreadEnv::setLogger(ILogReport* p)
{
	m_pLogger = p;
}
ILogReport* CThreadEnv::getLogger()
{
	return m_pLogger;
}

void CThreadEnv::setLinkEvent(ILinkEvent* p)
{
	m_pLinkEvent = p;
}
ILinkEvent* CThreadEnv::getLinkEvent()
{
	return m_pLinkEvent;
}

void CThreadEnv::setDataEvent(IDataEvent* p)
{
	m_pDataEvent = p;
}
IDataEvent* CThreadEnv::getDataEvent()
{
	return m_pDataEvent;
}

void CThreadEnv::setWriter(IWriter* p)
{
	m_pWriter = p;
}
IWriter* CThreadEnv::getWriter()
{
	return m_pWriter;
}

void CThreadEnv::setConnManager(IConnManager* p)
{
	m_pConnManager = p;
}
IConnManager* CThreadEnv::getConnManager()
{
	return m_pConnManager;
}

SEVParam& CThreadEnv::getEVParam()
{
	return m_evParam;
}

CThreadEnv* CEnv::getThreadEnv()
{
	if (m_tssEnv.get() == NULL)
		m_tssEnv.reset(new CThreadEnv());

	return m_tssEnv.get();
}
