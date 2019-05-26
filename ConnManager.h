//============================================================================
// Name        : ConnManager.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVComm.h"

#include <map>

namespace evwork
{

	class CConnManager
		: public IConnManager
		, public ILinkEvent
	{
	public:
		CConnManager();
		virtual ~CConnManager();

		virtual IConn* getConnById(uint32_t uConnId);
		virtual IConn* getConnByIpPort(const std::string& strIp, uint16_t uPort);

		virtual void onConnected(IConn* pConn);
		virtual void onClose(IConn* pConn);

		void addLE(ILinkEvent* p);
		void delLE(ILinkEvent* p);

	private:

		void __notifyLEConnected(IConn* pConn);
		void __notifyLEClose(IConn* pConn);

		std::string __toIpPortKey(const std::string& strIp, uint16_t uPort);

	protected:
		uint32_t m_uLastConnId;

		std::map<uint32_t, IConn*> m_mapCIdConn;
		std::map<std::string, IConn*> m_mapIpPortConn;

		std::set<ILinkEvent*> m_setLE;
	};

}
