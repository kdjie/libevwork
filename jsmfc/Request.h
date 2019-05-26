//============================================================================
// Name        : Request.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdint.h>

#include <json/json.h>

#define JS_CMD_TYPE		uint32_t
#define JS_HEADER_SIZE		8


#if defined(__i386__)||defined(WIN32)||defined(__x86_64__)

#define JS_HTONS
#define JS_HTONL
#define JS_HTONLL

#else /* big end */

inline uint16_t JS_HTONS(uint16_t i16) 
{
	return ((i16 << 8) | (i16 >> 8));
}
inline uint32_t JS_HTONL(uint32_t i32) 
{
	return ((uint32_t(JS_HTONS(i32)) << 16) | JS_HTONS(i32>>16));
}
inline uint64_t JS_HTONLL(uint64_t i64) 
{
	return ((uint64_t(JS_HTONL((uint32_t)i64)) << 32) |JS_HTONL((uint32_t(i64>>32))));
}

#endif /* __i386__ */

#define JS_NTOHS JS_HTONS
#define JS_NTOHL JS_HTONL
#define JS_NTOHLL JS_HTONLL

namespace js
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

		void setCmd(JS_CMD_TYPE uCmd)
		{
			m_uCmd = uCmd;
		}
		JS_CMD_TYPE getCmd() const
		{
			return m_uCmd;
		}

	protected:
		Header()
			: m_uLen(0), m_uCmd(0)
		{
		}
		Header(uint32_t uLen, JS_CMD_TYPE uCmd)
			: m_uLen(uLen), m_uCmd(uCmd)
		{
		}

	protected:
		uint32_t m_uLen;
		JS_CMD_TYPE m_uCmd;
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
			return JS_NTOHL(i);
		}
		void parseHeader()
		{
			m_uLen = *((uint32_t*)m_pData);
			m_uCmd = *((JS_CMD_TYPE*)(m_pData + 4));

			m_uLen = JS_NTOHL(m_uLen);
			m_uCmd = JS_NTOHL(m_uCmd);
		}

		const char* Body()
		{
			if (m_uSize < JS_HEADER_SIZE)
				return NULL;

			return m_pData + JS_HEADER_SIZE;
		}
		uint32_t BodySize()
		{
			if (m_uSize < JS_HEADER_SIZE)
				return 0;

			return m_uSize - JS_HEADER_SIZE;
		}

	private:
		char* m_pData;
		uint32_t m_uSize;
		bool m_bCopy;
	};

}
