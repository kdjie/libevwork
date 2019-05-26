//============================================================================
// Name        : Request.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "ds/dspacket.h"

#define DS_URI_TYPE		uint32_t
#define DS_HEADER_SIZE		10
#define DS_RES_SUCCESS     200     /* 功能成功完成,一切正常 */

namespace ds
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

		void setUri(DS_URI_TYPE uUri)
		{
			m_uUri = uUri;
		}
		DS_URI_TYPE getUri() const
		{
			return m_uUri;
		}

		void setCode(uint16_t uCode)
		{
			m_uCode = uCode;
		}
		uint16_t getCode() const
		{
			return m_uCode;
		}

		bool isSuccess()
		{
			return (DS_RES_SUCCESS == m_uCode);
		}

	protected:
		Header()
			: m_uLen(0), m_uUri(0), m_uCode(DS_RES_SUCCESS)
		{
		}
		Header(uint32_t uLen, DS_URI_TYPE uType)
			: m_uLen(uLen), m_uUri(uType), m_uCode(DS_RES_SUCCESS)
		{
		}

	protected:
		uint32_t m_uLen;
		DS_URI_TYPE m_uUri;
		uint16_t m_uCode;
	};

	class Request 
		: public Header
	{
	public:
		Request(const char *pData, uint32_t uSize, bool bCopy = false)
			: Header(0, 0)
			, m_Unpack(pData, uSize)
			, m_pData(NULL)
			, m_uSize(0)
		{
			if (bCopy && uSize > 0)
			{
				m_pData = new char[uSize];
				m_uSize = uSize;

				memcpy(m_pData, pData, uSize);
				m_Unpack.Reset(m_pData, m_uSize);
			}
		}
		~Request()
		{
			if (m_pData)
			{
				delete[] m_pData;
				m_pData = NULL;
			}
		}

		static uint32_t peekLen(const void * d)
		{
			uint32_t i = *((uint32_t*)d);
			return DS_NTOHL(i);
		}

		void parseHeader()
		{
			m_uLen = m_Unpack.Pop_uint32();
			m_uUri = m_Unpack.Pop_uint32();
			m_uCode = m_Unpack.Pop_uint16();
		}

		Unpack& getUnpack()
		{
			return m_Unpack;
		}

	private:
		Unpack m_Unpack;

		char* m_pData;
		uint32_t m_uSize;
	};

    inline char *uri2str(uint32_t uri)
    {
        uint32_t uri_h = uri / 256;
        uint32_t uri_t = uri % 256;

        static char s[16];
        sprintf(s, "%3d|%3d", uri_h, uri_t);
        return s;
    }

}
