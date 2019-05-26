//============================================================================
// Name        : ConnManager.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "ConnManager.h"

#include "EVWork.h"

using namespace evwork;

CConnManager::CConnManager()
: m_uLastConnId(1)
{
}
CConnManager::~CConnManager()
{
}

IConn* CConnManager::getConnById(uint32_t uConnId)
{
	std::map<uint32_t, IConn*>::iterator iter = m_mapCIdConn.find(uConnId);
	if (iter == m_mapCIdConn.end())
		return NULL;

	return iter->second;
}

IConn* CConnManager::getConnByIpPort(const std::string& strIp, uint16_t uPort)
{
	std::string strKey = __toIpPortKey(strIp, uPort);

	std::map<std::string, IConn*>::iterator iter = m_mapIpPortConn.find(strKey);
	if (iter != m_mapIpPortConn.end())
		return iter->second;

	CClientConn* pNew = new CClientConn(strIp, uPort);
	m_mapIpPortConn[strKey] = pNew;

	return pNew;
}

void CConnManager::onConnected(IConn* pConn)
{
	pConn->setcid(m_uLastConnId);

	m_mapCIdConn[m_uLastConnId] = pConn;

	m_uLastConnId++;

	__notifyLEConnected(pConn);
}

void CConnManager::onClose(IConn* pConn)
{
	__notifyLEClose(pConn);

	m_mapCIdConn.erase( pConn->getcid() );

	std::string strPeerIp = "";
	uint16_t uPeerPort = 0;
	pConn->getPeerInfo(strPeerIp, uPeerPort);

	m_mapIpPortConn.erase( __toIpPortKey(strPeerIp, uPeerPort) );
}

void CConnManager::addLE(ILinkEvent* p)
{
	m_setLE.insert(p);
}

void CConnManager::delLE(ILinkEvent* p)
{
	m_setLE.erase(p);
}

void CConnManager::__notifyLEConnected(IConn* pConn)
{
	for (std::set<ILinkEvent*>::iterator iter = m_setLE.begin(); iter != m_setLE.end(); ++iter)
	{
		(*iter)->onConnected(pConn);
	}
}

void CConnManager::__notifyLEClose(IConn* pConn)
{
	for (std::set<ILinkEvent*>::iterator iter = m_setLE.begin(); iter != m_setLE.end(); ++iter)
	{
		(*iter)->onClose(pConn);
	}
}

std::string CConnManager::__toIpPortKey(const std::string& strIp, uint16_t uPort)
{
	std::stringstream ssKey;
	ssKey << strIp << ":" << uPort;
	return ssKey.str();
}
