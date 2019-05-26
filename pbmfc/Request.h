//============================================================================
// Name        : Request.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdint.h>

#define PB_CMD_TYPE		uint32_t
#define PB_HEADER_SIZE		8


#if defined(__i386__)||defined(WIN32)||defined(__x86_64__)

#define PB_HTONS
#define PB_HTONL
#define PB_HTONLL

#else /* big end */

inline uint16_t PB_HTONS(uint16_t i16) 
{
	return ((i16 << 8) | (i16 >> 8));
}
inline uint32_t PB_HTONL(uint32_t i32) 
{
	return ((uint32_t(PB_HTONS(i32)) << 16) | PB_HTONS(i32>>16));
}
inline uint64_t PB_HTONLL(uint64_t i64) 
{
	return ((uint64_t(PB_HTONL((uint32_t)i64)) << 32) |PB_HTONL((uint32_t(i64>>32))));
}

#endif /* __i386__ */

#define PB_NTOHS PB_HTONS
#define PB_NTOHL PB_HTONL
#define PB_NTOHLL PB_HTONLL

namespace pb
{

	class Header
	{
	public:
		void setLen(uint32_t uLen)
		{
			m_uLen = uLen;
		}
		uint32_t getLen() const
		{
			return m_uLen;
		}

		void setCmd(PB_CMD_TYPE uCmd)
		{
			m_uCmd = uCmd;
		}
		PB_CMD_TYPE getCmd() const
		{
			return m_uCmd;
		}

	protected:
		Header()
			: m_uLen(0), m_uCmd(0)
		{
		}
		Header(uint32_t uLen, PB_CMD_TYPE uCmd)
			: m_uLen(uLen), m_uCmd(uCmd)
		{
		}

	protected:
		uint32_t m_uLen;
		PB_CMD_TYPE m_uCmd;
	};

	class Request
		: public Header
	{
	public:
		Request(const char *pData, uint32_t uSize, bool bCopy = false)
			: Header(0, 0)
			, m_pData((char*)pData)
			, m_uSize(uSize)
			, m_bCopy(bCopy)
		{
			if (bCopy && pData && uSize > 0)
			{
				m_pData = new char[uSize];
				m_uSize = uSize;

				memcpy(m_pData, pData, uSize);
			}
		}
		~Request()
		{
			if (m_bCopy && m_pData && m_uSize > 0)
			{
				delete[] m_pData;
				m_pData = NULL;
			}
		}

		static uint32_t peekLen(const void * d)
		{
			uint32_t i = *((uint32_t*)d);
			return PB_NTOHL(i);
		}
		void parseHeader()
		{
			m_uLen = *((uint32_t*)m_pData);
			m_uCmd = *((PB_CMD_TYPE*)(m_pData + 4));

			m_uLen = PB_NTOHL(m_uLen);
			m_uCmd = PB_NTOHL(m_uCmd);
		}

		const char* Body()
		{
			if (m_uSize < PB_HEADER_SIZE)
				return NULL;

			return m_pData + PB_HEADER_SIZE;
		}
		uint32_t BodySize()
		{
			if (m_uSize < PB_HEADER_SIZE)
				return 0;

			return m_uSize - PB_HEADER_SIZE;
		}

	private:
		char* m_pData;
		uint32_t m_uSize;
		bool m_bCopy;
	};

}
