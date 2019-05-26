//============================================================================
// Name        : ListenConn.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "ListenConn.h"

#include "EVWork.h"

using namespace evwork;

#define DEF_LISTEN_BACKLOG	100

CListenConn::CListenConn(uint16_t uListenPort, const std::string& strBindIp)
: m_uListenPort(uListenPort)
, m_strBindIp(strBindIp)
, m_fd(-1)
, m_hAccept(this)
, m_pDE_Special(NULL)
{
	__create();
	__reuse();
	__noblock();
	__bind();
	__listen();

	m_hAccept.setEv(EV_READ);
	m_hAccept.setFd(m_fd);
	CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hAccept);
}
CListenConn::~CListenConn()
{
	CEnv::getThreadEnv()->getEVLoop()->delHandle(&m_hAccept);
}

void CListenConn::setSpecialDE(IDataEvent* pDE)
{
	m_pDE_Special = pDE;
}

void CListenConn::__create()
{
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd < 0)
	{
		LOG(Error, "[CListenConn::%s] socket() failed!", __FUNCTION__);
		exit(-1);
	}
}

void CListenConn::__reuse()
{
	int uFlag = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &uFlag, sizeof(int)) == -1)
	{
		LOG(Error, "[CListenConn::%s] setsockopt(%d, SO_REUSEADDR) failed!", __FUNCTION__, m_fd);
		exit(-1);
	}
}

void CListenConn::__noblock()
{
	int nFlags = fcntl(m_fd, F_GETFL);
	if (nFlags == -1)
	{
		LOG(Error, "[CListenConn::%s] fcntl(%d, F_GETFL) failed!", __FUNCTION__, m_fd);
		exit(-1);
	}

	nFlags |= O_NONBLOCK;

	int nRet = fcntl(m_fd, F_SETFL, nFlags);
	if (nRet == -1)
	{
		LOG(Error, "[CListenConn::%s] fcntl(%d, F_SETFL) failed!", __FUNCTION__, m_fd);
		exit(-1);
	}
}

void CListenConn::__bind()
{
	u_long addr = INADDR_ANY;

	if (m_strBindIp != "")
	{
		addr = ::inet_addr(m_strBindIp.c_str());
		if (addr == INADDR_NONE)
		{
			LOG(Error, "[CListenConn::%s] inet_addr(ip:%s) invalid!", __FUNCTION__, m_strBindIp.c_str());
			exit(-1);
		}
	}

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;
	sa.sin_port = htons(m_uListenPort);

	if (bind(m_fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
	{
		LOG(Error, "[CListenConn::%s] bind(port:%u ip:%s) failed", __FUNCTION__, m_uListenPort, m_strBindIp.c_str());
		exit(-1);
	}
}

void CListenConn::__listen()
{
	if (listen(m_fd, DEF_LISTEN_BACKLOG) == -1)
	{
		LOG(Error, "[CListenConn::%s] listen(%d) failed!", __FUNCTION__, m_fd);
		exit(-1);
	}
}

void CListenConn::cbAccept(int revents)
{
	while (true)
	{
		struct sockaddr_in sinfrom;
		socklen_t sinlen = sizeof(sinfrom);

		int fdClient = ::accept(m_fd, (struct sockaddr*)&sinfrom, &sinlen);
		if (fdClient == -1)
		{
			if (errno == ECONNABORTED || errno == EPROTO || errno == EINTR)
				continue;

			else if (errno == EAGAIN)
				break;

			LOG(Error, "[CListenConn::%s] ::accept(%d) failed, desc:[%s]", __FUNCTION__, m_fd, strerror(errno));
			throw exception_errno( toString("[CListenConn::%s] ::accept(%d) failed", __FUNCTION__, m_fd) );
		}

		std::string strIpFrom = inet_ntoa(*(struct in_addr*)&sinfrom.sin_addr);
		uint16_t uPortFrom = ntohs(sinfrom.sin_port);

		CClientConn* pNew = new CClientConn(fdClient, strIpFrom, uPortFrom);

		if (m_pDE_Special)
		{
			pNew->setSpecialDE(m_pDE_Special);
		}

		LOG(Info, "[CListenConn::%s] accept client[%u]: %s:%u", __FUNCTION__, fdClient, strIpFrom.c_str(), uPortFrom);
	}
}
