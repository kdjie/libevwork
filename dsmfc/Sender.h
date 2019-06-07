//============================================================================
// Name        : Sender.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "Request.h"

namespace ds
{

	class Sender
		: public Header
	{
	public:
		Sender()
			: m_pb(), m_hpk(m_pb), m_pk(m_pb, DS_HEADER_SIZE)
		{
		}

		Sender(const Sender& s)
			: m_pb(), m_hpk(m_pb), m_pk(m_pb, DS_HEADER_SIZE)
		{
			m_uLen = s.m_uLen;
			m_uUri = s.m_uUri;
			m_uCode = s.m_uCode;
			m_pk.Push(s.m_pk.Data(), s.m_pk.Size());
		}

		Sender(DS_URI_TYPE uri, const Marshallable &obj)
			: m_pb(), m_hpk(m_pb), m_pk(m_pb, DS_HEADER_SIZE)
		{
			m_uUri = uri;
			obj.Marshal(m_pk);
		}

		Sender(DS_URI_TYPE uri, const std::string& str)
			: m_pb(), m_hpk(m_pb), m_pk(m_pb, DS_HEADER_SIZE)
		{
			m_uUri = uri;
			m_pk.Push(str.data(), str.size());
		}

		Sender(DS_URI_TYPE uri, const char* data, size_t size)
			: m_pb(), m_hpk(m_pb), m_pk(m_pb, DS_HEADER_SIZE)
		{
			m_uUri = uri;
			m_pk.Push(data, size);
		}

		void EndPack()
		{
			m_uLen = (uint32_t)(DS_HEADER_SIZE + m_pk.Size());

			m_hpk.Replace_uint32(0, m_uLen);
			m_hpk.Replace_uint32(4, m_uUri);
			m_hpk.Replace_uint16(8, m_uCode);
		}

		const char* Data()
		{
			return m_hpk.Data();
		}

		size_t Size() const
		{
			return DS_HEADER_SIZE + m_pk.Size();
		}

		const char* Body()
		{
			return m_pk.Data();
		}

		size_t BodySize() const
		{
			return m_pk.Size();
		}

	protected:
		PackBuffer m_pb;
		Pack m_hpk;
		Pack m_pk;
	};

}
