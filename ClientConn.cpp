//============================================================================
// Name        : ClientConn.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "ClientConn.h"

#include "EVWork.h"

using namespace evwork;

#define DEF_CONN_TIMEOUT	600
#define MAX_INPUT_SIZE		8*1024*1024
#define MAX_OUTPUT_SIZE		8*1024*1024
#define BASE_READ_SIZE		8*1024

CClientConn::CClientConn(const std::string& strPeerIp, uint16_t uPeerPort16)
: m_strPeerIp(strPeerIp)
, m_uPeerPort16(uPeerPort16)
, m_bConnected(false)
, m_Input(8*1024, 1)
, m_Output(8*1024, 1)
, m_bTimerNoDataStart(false)
, m_bTimerDestroyStart(false)
, m_hRead(this)
, m_hWrite(this)
, m_pDE_Special(NULL)
{
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd == -1)
		throw exception_errno( toString("[CClientConn::%s] socket(ip:%s port:%u)", __FUNCTION__, m_strPeerIp.c_str(), m_uPeerPort16) );

	__noblock();
	__nodelay();

	struct sockaddr_in sinto;
	sinto.sin_family = AF_INET;
	sinto.sin_addr.s_addr = ::inet_addr(m_strPeerIp.c_str());
	sinto.sin_port = htons(m_uPeerPort16);

	connect(m_fd, (struct sockaddr*)&sinto, sizeof(sinto));

	m_hRead.setEv(EV_READ);
	m_hRead.setFd(m_fd);

	m_hWrite.setEv(EV_WRITE);
	m_hWrite.setFd(m_fd);

	// 关注写事件（等待连接成功）
	CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hWrite);
}

CClientConn::CClientConn(int fd, const std::string& strPeerIp, uint16_t uPeerPort16)
: m_strPeerIp(strPeerIp)
, m_uPeerPort16(uPeerPort16)
, m_bConnected(true)
, m_Input(8*1024, 1)
, m_Output(8*1024, 1)
, m_bTimerNoDataStart(false)
, m_bTimerDestroyStart(false)
, m_hRead(this)
, m_hWrite(this)
, m_pDE_Special(NULL)
{
	m_fd = fd;

	__noblock();
	__nodelay();

	m_hRead.setEv(EV_READ);
	m_hRead.setFd(m_fd);

	m_hWrite.setEv(EV_WRITE);
	m_hWrite.setFd(m_fd);

	// 关注读事件（接收数据)
	CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hRead);

	// 启动超时
	__initTimerNoData();

	// 通知连接成功
	if (CEnv::getThreadEnv()->getLinkEvent())
		CEnv::getThreadEnv()->getLinkEvent()->onConnected(this);
}

CClientConn::~CClientConn()
{
	// 请除读写事件
	CEnv::getThreadEnv()->getEVLoop()->delHandle(&m_hRead);
	CEnv::getThreadEnv()->getEVLoop()->delHandle(&m_hWrite);

	// 销毁定时器
	__destroyTimerNoData();
	__destroyTimerDestry();

	// 通知连接断开
	if (CEnv::getThreadEnv()->getLinkEvent())
		CEnv::getThreadEnv()->getLinkEvent()->onClose(this);

	if (m_fd != -1)
	{
		close(m_fd);
		m_fd = -1;
	}
}

void CClientConn::setSpecialDE(IDataEvent* pDE)
{
	m_pDE_Special = pDE;
}

void CClientConn::getPeerInfo(std::string& strPeerIp, uint16_t& uPeerPort16)
{
	strPeerIp = m_strPeerIp;
	uPeerPort16 = m_uPeerPort16;
}

bool CClientConn::sendBin(const char* pData, size_t uSize)
{
	try
	{
		// 未连接成功
		if (!m_bConnected)
		{
			// 直接拷入发送缓冲区（等待连接成功）
			__appendBuffer(pData, uSize);
		}
		// 已经连接
		else
		{
			// 原发送缓冲区为空
			if (m_Output.size() == 0)
			{
				// 直接发送数据
				size_t uBytesSend = __sendData(pData, uSize);

				// 未完全发送完毕，将余下的数据拷入发送缓冲区
				if (uBytesSend < uSize)
				{
					__appendBuffer( (pData + uBytesSend), (uSize - uBytesSend) );

					// 启动写事件（等待继续发送数据）
					CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hWrite);
				}
			}
			// 原发送缓冲区非空
			else
			{
				// 拷贝到发送缓冲区（等待下一写事件）
				__appendBuffer(pData, uSize);
			}
		}

		return true;
	}
	catch (exception_errno& e)
	{
		//__willFreeMyself( e.what() );
		// =>
		// 该函数主要由应用层调用，不能在这里释放连接，改为定时器延时释放

		LOG(Error, "[CClientConn::%s] catch exception:[%s], connection delay free!", __FUNCTION__, e.what());

		// 启动定时，销毁连接
		__initTimerDestry();

		return false;
	}
}

void CClientConn::cbEvent(int revents)
{
	try
	{
		if ((revents & EV_READ) == EV_READ)
		{
			//LOG(Info, "[CClientConn::%s] fd:[%d] peer:[%s:%u] EV_READ...", __FUNCTION__, m_fd, m_strPeerIp.c_str(), m_uPeerPort16);

			__onRead();
		}

		if ((revents & EV_WRITE) == EV_WRITE)
		{
			//LOG(Info, "[CClientConn::%s] fd:[%d] peer:[%s:%u] EV_WRITE...", __FUNCTION__, m_fd, m_strPeerIp.c_str(), m_uPeerPort16);

			__onWrite();
		}
	}
	catch (exception_errno& e)
	{
		LOG(Error, "[CClientConn::%s] catch exception:[%s], connection immediate free!", __FUNCTION__, e.what());

		__willFreeMyself( e.what() );
	}
}

void CClientConn::__noblock()
{
	int nFlags = fcntl(m_fd, F_GETFL);
	if (nFlags == -1)
		throw exception_errno( toString("[CListenConn::%s] fcntl(%d, F_GETFL) failed!", __FUNCTION__, m_fd) );

	nFlags |= O_NONBLOCK;

	int nRet = fcntl(m_fd, F_SETFL, nFlags);
	if (nRet == -1)
		throw exception_errno( toString("[CListenConn::%s] fcntl(%d, F_SETFL) failed!", __FUNCTION__, m_fd) );
}

void CClientConn::__nodelay()
{
	int uFlag = 1;
	setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &uFlag, sizeof(int));
}

void CClientConn::__initTimerNoData()
{
	if (m_bTimerNoDataStart)
		return;

	float fTimeout = DEF_CONN_TIMEOUT;
	if (CEnv::getThreadEnv()->getEVParam().uConnTimeout != (uint32_t)-1)
	{
		fTimeout = CEnv::getThreadEnv()->getEVParam().uConnTimeout;
	}

	m_evTimerNoData.data = this;
	ev_timer_init(&m_evTimerNoData, CClientConn::__cbTimerNoData, fTimeout, fTimeout);
	ev_timer_start(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimerNoData);

	m_bTimerNoDataStart = true;
}

void CClientConn::__destroyTimerNoData()
{
	if (!m_bTimerNoDataStart)
		return;

	ev_timer_stop(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimerNoData);

	m_bTimerNoDataStart = false;
}

void CClientConn::__updateTimerNoData()
{
	if (!m_bTimerNoDataStart)
		return;

	ev_timer_again(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimerNoData);
}

void CClientConn::__cbTimerNoData(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	CClientConn* pThis = (CClientConn*)w->data;

	LOG(Info, "[CClientConn::%s] fd:[%d] peer:[%s:%u] timeout", __FUNCTION__, pThis->m_fd, pThis->m_strPeerIp.c_str(), pThis->m_uPeerPort16);

	pThis->__willFreeMyself( "timeout" );
}

void CClientConn::__initTimerDestry()
{
	if (m_bTimerDestroyStart)
		return;

	m_evTimerDestroy.data = this;
	ev_timer_init(&m_evTimerDestroy, CClientConn::__cbTimerDestry, 0.1, 0);
	ev_timer_start(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimerDestroy);

	m_bTimerDestroyStart = true;
}

void CClientConn::__destroyTimerDestry()
{
	if (!m_bTimerDestroyStart)
		return;

	ev_timer_stop(CEnv::getThreadEnv()->getEVLoop()->getEvLoop(), &m_evTimerDestroy);

	m_bTimerDestroyStart = false;
}

void CClientConn::__cbTimerDestry(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	CClientConn* pThis = (CClientConn*)w->data;

	LOG(Info, "[CClientConn::%s] fd:[%d] peer:[%s:%u] destroy", __FUNCTION__, pThis->m_fd, pThis->m_strPeerIp.c_str(), pThis->m_uPeerPort16);

	pThis->__willFreeMyself( "destroy" );
}

// 读取数据
void CClientConn::__onRead()
{
	__updateTimerNoData();

	// 循环读取，直到读取到底层边界
	while (true)
	{
		if (m_Input.freesize() < BASE_READ_SIZE)
		{
			m_Input.inc_capacity(BASE_READ_SIZE);
		}

		uint32_t uFreeSize = m_Input.freesize();

		// 读取过程中，如果出现错误，会抛出异常，从而关闭连接
		size_t uBytesRecv = __recvData(m_Input.tail(), uFreeSize);

		if (uBytesRecv > 0)
		{
			if (m_Input.size() + uBytesRecv > MAX_INPUT_SIZE)
			{
				throw exception_errno(0, toString("[CClientConn::%s] input buffer overflow!!!", __FUNCTION__) );
			}

			m_Input.inc_size(uBytesRecv);
		}

		if (uBytesRecv < uFreeSize)
			break;
	}

	// 数据回调
	if (m_Input.size() > 0)
	{
		int nRetSize = 0;

		if (m_pDE_Special)
		{
			nRetSize = m_pDE_Special->onData(this, m_Input.data(), m_Input.size());
		}
		else if (CEnv::getThreadEnv()->getDataEvent())
		{
			nRetSize = CEnv::getThreadEnv()->getDataEvent()->onData(this, m_Input.data(), m_Input.size());
		}
		else
		{
			nRetSize = m_Input.size();
		}

		// 上层处理出现问题，需要关闭连接
		if (nRetSize < 0)
		{
			throw exception_errno(0, toString("[CClientConn::%s] onData[maybe format error]", __FUNCTION__) );
		}
		// 将已经处理的数据移除
		else if (nRetSize > 0)
		{
			m_Input.erase(nRetSize);
		}
	}
}

// 写入数据
void CClientConn::__onWrite()
{
	// 判断是否连接成功
	if (!m_bConnected)
	{
		int e = 0;
		socklen_t l = sizeof(e);

		getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &e, &l);
		if (e)
		{
			// 连接失败，关闭连接
			throw exception_errno(0, toString("[CClientConn::%s] connect failed", __FUNCTION__) );
		}

		// 连接成功
		m_bConnected = true;

		// 启动读事件（接收数据）
		CEnv::getThreadEnv()->getEVLoop()->setHandle(&m_hRead);

		// 启动超时
		__initTimerNoData();

		// 通知连接成功
		if (CEnv::getThreadEnv()->getLinkEvent())
			CEnv::getThreadEnv()->getLinkEvent()->onConnected(this);
	}

	// 尝试发送缓冲区
	__sendBuffer();

	// 全部发送完成
	if (m_Output.size() == 0)
	{
		// 清除写事件
		CEnv::getThreadEnv()->getEVLoop()->delHandle(&m_hWrite);
	}
}

// 加入发送缓冲区
void CClientConn::__appendBuffer(const char* pData, size_t uSize)
{
	if (m_Output.size() + uSize > MAX_OUTPUT_SIZE)
		throw exception_errno(0, toString("[CClientConn::%s] output buffer overflow!!!", __FUNCTION__) );

	m_Output.append(pData, uSize);
}

// 尝试发送缓冲区
void CClientConn::__sendBuffer()
{
	if (m_Output.size() > 0)
	{
		int nSendBytes = __sendData(m_Output.data(), m_Output.size());

		if (nSendBytes > 0)
			m_Output.erase(nSendBytes);
	}
}

// 发送数据
size_t CClientConn::__sendData(const char* pData, size_t uSize)
{
	// 循环发送，直到数据发送完成，或者遇到边界
	size_t bytes_total = 0;
	while (bytes_total < uSize)
	{
		int bytes_out = ::send(m_fd, (pData + bytes_total), (uSize - bytes_total), 0);
		if (bytes_out == -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				break;
			else
				throw exception_errno( toString("[CClientConn::%s] ::send", __FUNCTION__) );
		}
		else if (bytes_out > 0)
		{
			bytes_total += (size_t)bytes_out;
		}
	}

	return bytes_total;
}

// 读取数据
size_t CClientConn::__recvData(char* pData, size_t uSize)
{
	size_t bytes_total = 0;
	while (bytes_total < uSize)
	{
		int bytes_in = ::recv(m_fd, (pData + bytes_total), (uSize - bytes_total), 0);
		if (bytes_in < 0)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				break;
			else
				throw exception_errno( toString("[CClientConn::%s] ::recv", __FUNCTION__) );
		}
		else if (bytes_in == 0)
		{
			throw exception_errno(0, toString("[CClientConn::%s] ::recv peer close", __FUNCTION__) );
		}
		else
		{
			bytes_total += (size_t)bytes_in;
		}
	}

	return bytes_total;
}

// 准备释放本连接
void CClientConn::__willFreeMyself(const std::string& strDesc)
{
	LOG(Info, "[CClientConn::%s] fd:[%d] peer:[%s:%u], %s, delete myself", __FUNCTION__, m_fd, m_strPeerIp.c_str(), m_uPeerPort16, strDesc.c_str());

	delete this;
}
