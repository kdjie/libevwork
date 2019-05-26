//============================================================================
// Name        : Sender.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "Request.h"

#include "libevwork/Buffer.h"

namespace js
{

	class Sender
		: public Header
	{
	public:
		Sender()
			: m_buffer(4096, 0)
		{
		}

		Sender(const Sender& s)
			: m_buffer(4096, 0)
		{
			m_uLen = s.m_uLen;
			m_uCmd = s.m_uCmd;

			m_buffer = s.m_buffer;
		}

		Sender(JS_CMD_TYPE cmd, const char* pData, uint32_t uSize)
			: m_buffer(4096, 0)
		{
			m_uLen = JS_HEADER_SIZE + uSize;
			m_uCmd = cmd;

			m_buffer.reset();
			m_buffer.inc_capacity(m_uLen);

			*(uint32_t*)m_buffer.tail() = JS_HTONL(m_uLen);
			*(uint32_t*)(m_buffer.tail() + 4) = JS_HTONL(m_uCmd);

			memcpy(m_buffer.tail() + 8, pData, uSize);

			m_buffer.inc_size(m_uLen);
		}

		Sender(JS_CMD_TYPE cmd, const std::string& strData)
			: m_buffer(4096, 0)
		{
			m_uLen = JS_HEADER_SIZE + strData.size();
			m_uCmd = cmd;

			m_buffer.reset();
			m_buffer.inc_capacity(m_uLen);

			*(uint32_t*)m_buffer.tail() = JS_HTONL(m_uLen);
			*(uint32_t*)(m_buffer.tail() + 4) = JS_HTONL(m_uCmd);

			memcpy(m_buffer.tail() + 8, strData.data(), strData.size());

			m_buffer.inc_size(m_uLen);
		}

		const char* Data()
		{
			return m_buffer.data();
		}

		size_t Size() const
		{
			return m_buffer.size();
		}

		const char* Body()
		{
			if (m_buffer.size() < JS_HEADER_SIZE)
				return NULL;

			return m_buffer.data() + JS_HEADER_SIZE;
		}

		size_t BodySize() const
		{
			if (m_buffer.size() < JS_HEADER_SIZE)
				return 0;

			return m_buffer.size() - JS_HEADER_SIZE;
		}

	protected:
		evwork::CBuffer m_buffer;
	};

}
