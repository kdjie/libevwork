//============================================================================
// Name        : ListenConn.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVComm.h"

namespace evwork
{

	class CListenConn
	{
	public:
		CListenConn(uint16_t uListenPort, const std::string& strBindIp = "");
		virtual ~CListenConn();

		void cbAccept(int revents);

		void setSpecialDE(IDataEvent* pDE);

	private:

		void __create();
		void __reuse();
		void __noblock();
		void __bind();
		void __listen();

	private:
		uint16_t m_uListenPort;
		std::string m_strBindIp;

		int m_fd;

		THandle<CListenConn, &CListenConn::cbAccept> m_hAccept;

		IDataEvent* m_pDE_Special;
	};

}
