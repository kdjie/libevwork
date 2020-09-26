//============================================================================
// Name        : ClientConn.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "EVComm.h"
#include "Buffer.h"
#include "TimerHandler.h"

namespace evwork
{

	class CClientConn
		: public IConn
	{
	public:
		CClientConn(const std::string& strPeerIp, uint16_t uPeerPort16);
		CClientConn(int fd, const std::string& strPeerIp, uint16_t uPeerPort16);
		virtual ~CClientConn();
		
		virtual void getPeerInfo(std::string& strPeerIp, uint16_t& uPeerPort16);
		virtual bool sendBin(const char* pData, size_t uSize);

		void setSpecialDE(IDataEvent* pDE);

		// 读写事件控制（慎用）
		void startRead();
		void stopRead();

	private:

		void __noblock();
		void __nodelay();

		// 接收数据超时控制
		void __initTimerNoData();
		void __destroyTimerNoData();
		void __updateTimerNoData();
		bool __cbTimerNoData();

		// 延迟销毁对象
		void __initTimerDestry();
		void __destroyTimerDestry();
		bool __cbTimerDestry();

		// IO事件回调
		void __cbEvent(int revents);

		// 读取数据
		void __onRead();

		// 写入数据
		void __onWrite();

		// 加入发送缓冲区
		void __appendBuffer(const char* pData, size_t uSize);

		// 尝试发送缓冲区
		void __sendBuffer();

		// 发送数据
		size_t __sendData(const char* pData, size_t uSize);

		// 读取数据
		size_t __recvData(char* pData, size_t uSize);

		// 准备释放本连接
		void __willFreeMyself(const std::string& strDesc);

	private:
		std::string m_strPeerIp;
		uint16_t m_uPeerPort16;

		bool m_bConnected;

		CBuffer m_Input;
		CBuffer m_Output;

		THandle<CClientConn, &CClientConn::__cbEvent> m_hRead;
		THandle<CClientConn, &CClientConn::__cbEvent> m_hWrite;

		TimerHandler<CClientConn, &CClientConn::__cbTimerNoData> m_timerNoData;
		TimerHandler<CClientConn, &CClientConn::__cbTimerDestry> m_timerDestry;

		IDataEvent* m_pDE_Special;
	};

}
