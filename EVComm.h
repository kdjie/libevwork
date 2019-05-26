//============================================================================
// Name        : EVComm.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#include <ev.h>

#include "ExceptionErrno.h"
#include "FuncHelper.h"

namespace evwork
{
	// 连接对象
	struct IConn
	{
	public:
		IConn() : m_fd(-1), m_cid(0) {}
		virtual ~IConn() {}

		void setcid(uint32_t cid) { m_cid = cid; }
		uint32_t getcid() { return m_cid; }

		virtual void getPeerInfo(std::string& strPeerIp, uint16_t& uPeerPort16) = 0;
		virtual bool sendBin(const char* pData, size_t uSize) = 0;

	protected:
		int m_fd;
		uint32_t m_cid;
	};

	// 连接事件
	struct ILinkEvent
	{
	public:
		virtual ~ILinkEvent() {}

		virtual void onConnected(IConn* pConn) = 0;
		virtual void onClose(IConn* pConn) = 0;
	};

	// 数据事件
	struct IDataEvent
	{
	public:
		virtual ~IDataEvent() {}

		virtual int onData(IConn* pConn, const char* pData, size_t uSize) = 0;
	};

	// 事件句柄
	struct IHandle
	{
	public:
		IHandle() : m_fd(-1), m_ev(0) { m_evio.data = this; }
		virtual ~IHandle() {}

		void setFd(int fd) { m_fd = fd; }
		int getFd() { return m_fd; }

		void setEv(int ev) { m_ev = ev; }
		int getEv() { return m_ev; }

		ev_io& getEvIo() { return m_evio; }

		virtual void cbEvent(int revents) = 0;

		static void evCallBack(struct ev_loop *loop, struct ev_io *w, int revents)
		{
			IHandle* pThis = (IHandle*)w->data;

			pThis->cbEvent(revents);
		}

	protected:
		int m_fd;
		int m_ev;
		ev_io m_evio;
	};

	// 模板事件句柄
	template <typename T, void (T::*fn)(int revents)>
	class THandle
		: public IHandle
	{
	public:
		THandle(T* p) : m_pObj(p) {}
		virtual ~THandle() {}

		void cbEvent(int revents)
		{
			(m_pObj->*fn)(revents);
		}

	protected:
		T* m_pObj;
	};

	// EVLook事件循环对象
	class CEVLoop
	{
	public:
		CEVLoop();
		virtual ~CEVLoop();

		bool init();
		void destroy();

		void runLoop();
		void breakLoop();

		void setHandle(IHandle* p);
		void delHandle(IHandle* p);

		struct ev_loop* getEvLoop();

	private:
		struct ev_loop* m_pEVLoop;

		std::set<IHandle*> m_setHandle;
	};

	// TCP发包接口
	struct IWriter
	{
	public:
		virtual ~IWriter() {}

		virtual void send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize) = 0;
		virtual void flush() = 0;
	};

	// TCP连接管理
	struct IConnManager
	{
	public:
		virtual ~IConnManager() {}

		virtual IConn* getConnById(uint32_t uConnId) = 0;
		virtual IConn* getConnByIpPort(const std::string& strIp, uint16_t uPort) = 0;
	};

}
