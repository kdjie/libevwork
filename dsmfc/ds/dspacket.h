//============================================================================
// Name        : dspacket.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "dsbuffer.h"
#include "dstypes.h"

namespace ds
{
// x86平台，小端编码
#if defined(__i386__) || defined(__x86_64__) || defined(WIN32)

	#define DS_HTONS
	#define DS_HTONL
	#define DS_HTONLL

// 其它平台，大端编码
#else

	inline uint16_t DS_HTONS(uint16_t u16)
	{
		return ( (u16 << 8) | (u16 >> 8) );
	}
	inline uint32_t DS_HTONL(uint32_t u32)
	{
		return ( (uint32_t(DS_HTONS(uint16_t(u32))) << 16) | DS_HTONS(uint16_t(u32 >> 16)) );
	}
	inline uint64_t DS_HTONLL(uint64_t u64)
	{
	    return ( (uint64_t(DS_HTONL(uint32_t(u64))) << 32) | DS_HTONL(uint32_t(u64 >> 32)) );
	}

#endif

    #define DS_NTOHS DS_HTONS
    #define DS_NTOHL DS_HTONL
    #define DS_NTOHLL DS_HTONLL

    // 定义异常类型
	struct DSError : public std::runtime_error
	{
		DSError(const std::string & w) : std::runtime_error(w) {}
	};

    // 定义压包缓冲区
	class PackBuffer
	{
	private:
		typedef DSBuffer<block_alloc_16k, 65536> DB_t;
		DB_t db;

	public:
		char * Data()
		{
			return db.Data();
		}
		size_t Size() const
		{
			return db.Size();
		}

		void Reserve(size_t n)
		{
			if (db.Reserve(n))
				return;

			throw DSError("[PackBuffer::Reserve] reserve buffer overflow");
		}
		void Resize(size_t n)
		{
			if (db.Resize(n))
				return;

			throw DSError("[PackBuffer::Resize] resize buffer overflow");
		}

		void Append(const char * data, size_t size)
		{
			if (db.Append(data, size))
				return;

			throw DSError("[PackBuffer::Append] append buffer overflow");
		}
		void Append(const char * data)
		{
			Append(data, ::strlen(data));
		}

		void Replace(size_t pos, const char * rep, size_t n)
		{
			if (db.Replace(pos, rep, n))
				return;

			throw DSError("[PackBuffer::Replace] replace buffer overflow");
		}
	};

    // 定义序列化
	class Pack
	{
	private:
		PackBuffer & m_buffer;
		size_t m_offset;

		Pack (const Pack & o);
		Pack & operator = (const Pack& o);

	public:
		static uint16_t xhtons(uint16_t u16) { return DS_HTONS(u16); }
		static uint32_t xhtonl(uint32_t u32) { return DS_HTONL(u32); }
		static uint64_t xhtonll(uint64_t u64) { return DS_HTONLL(u64); }

        // 初使化指定一个压包缓冲区和偏移量
		Pack(PackBuffer & pb, size_t off = 0)
		: m_buffer(pb)
		{
			m_offset = pb.Size() + off;
			m_buffer.Resize(m_offset);
		}
		virtual ~Pack() {}

		char * Data() { return m_buffer.Data() + m_offset; }
		const char * Data() const { return m_buffer.Data() + m_offset; }
		size_t Size() const { return m_buffer.Size() - m_offset; }

		Pack & Push(const void * s, size_t n) { m_buffer.Append((const char *)s, n); return *this; }
		Pack & Push(const void * s) { m_buffer.Append((const char *)s); return *this; }

		Pack & Push_uint8(uint8_t u8) { return Push(&u8, 1); }
		Pack & Push_uint16(uint16_t u16) { u16 = xhtons(u16); return Push(&u16, 2); }
		Pack & Push_uint32(uint32_t u32) { u32 = xhtonl(u32); return Push(&u32, 4); }
		Pack & Push_uint64(uint64_t u64) { u64 = xhtonll(u64); return Push(&u64, 8); }

		Pack & Push_string(const void * s, size_t len)
		{
			if (len > 0xFFFF) throw DSError("[Pack::Push_string] string too big");
			return Push_uint16(uint16_t(len)).Push(s, len);
		}
		Pack & Push_string32(const void * s, size_t len)
		{
			if (len > 0xFFFFFFFF) throw DSError("[Pack::Push_string32] string too big");
			return Push_uint32(uint32_t(len)).Push(s, len);
		}

        Pack & Push_string(const void * s) { return Push_string(s, ::strlen((const char *)s)); }
        Pack & Push_string(const std::string & s) { return Push_string(s.data(), s.size()); }
		Pack & Push_string(const StringPtr & SP) { return Push_string(SP.Data(), SP.Size()); }

		size_t Replace(size_t pos, const void * data, size_t rplen)
		{
			m_buffer.Replace(pos, (const char*)data, rplen);
			return pos + rplen;
		}
		size_t Replace_uint8(size_t pos, uint8_t u8)
		{ 
			return Replace(pos, &u8, 1);
		}
		size_t Replace_uint16(size_t pos, uint16_t u16)
		{
			u16 = xhtons(u16);
			return Replace(pos, &u16, 2);
		}
		size_t Replace_uint32(size_t pos, uint32_t u32)
		{
			u32 = xhtonl(u32);
			return Replace(pos, &u32, 4);
		}

		size_t Replace_offset(size_t pos, const void * data, size_t rplen)
		{
			m_buffer.Replace(pos + m_offset, (const char*)data, rplen);
			return pos + rplen;
		}
		size_t Replace_offset_uint8(size_t pos, uint8_t u8)
		{ 
			return Replace_offset(pos, &u8, 1);
		}
		size_t Replace_offset_uint16(size_t pos, uint16_t u16)
		{
			u16 = xhtons(u16);
			return Replace_offset(pos, &u16, 2);
		}
		size_t Replace_offset_uint32(size_t pos, uint32_t u32)
		{
			u32 = xhtonl(u32);
			return Replace_offset(pos, &u32, 4);
		}
	};

    // 定义反序列化
	class Unpack
	{
	private:
		mutable const char * m_data;
		mutable size_t m_size;

	public:
		static uint16_t xntohs(uint16_t u16) { return DS_NTOHS(u16); }
		static uint32_t xntohl(uint32_t u32) { return DS_NTOHL(u32); }
		static uint64_t xntohll(uint64_t u64) { return DS_NTOHLL(u64); }

		Unpack(const void * data, size_t size)
		{
			Reset(data, size);
		}
		virtual ~Unpack()
		{
			Reset(NULL, 0);
		}

		void Reset(const void * data, size_t size) const
		{
			m_data = (const char *)data;
			m_size = size;
		}

		// 测试是否精确完成解包，否则抛出异常
		void Finish() const
		{
			if (!Empty())
				throw DSError("[Unpack::Finish] too much data");
		}

		uint8_t Pop_uint8() const
		{
			if (m_size < 1u)
				throw DSError("[Unpack::pop_uint8] not enough data");

			uint8_t u8 = *((uint8_t*)m_data);

			m_data += 1u; 
			m_size -= 1u;

			return u8;
		}

		uint16_t Pop_uint16() const
		{
			if (m_size < 2u)
				throw DSError("[Unpack::Pop_uint16] not enough data");

			uint16_t u16 = *((uint16_t*)m_data);
			u16 = xntohs(u16);

			m_data += 2u; 
			m_size -= 2u;

			return u16;
		}

		uint32_t Pop_uint32() const
		{
			if (m_size < 4u)
				throw DSError("[Unpack::Pop_uint32] not enough data");

			uint32_t u32 = *((uint32_t*)m_data);
			u32 = xntohl(u32);

			m_data += 4u; 
			m_size -= 4u;

			return u32;
		}

        // 不真正解数据，仅仅查看
		uint32_t Peek_uint32() const
		{
			if (m_size < 4u)
				throw DSError("[Unpack::Peek_uint32] not enough data");

			uint32_t u32 = *((uint32_t*)m_data);
			u32 = xntohl(u32);

			return u32;
		}

		uint64_t Pop_uint64() const
		{
			if (m_size < 8u)
				throw DSError("[Unpack::Pop_uint64] not enough data");

			uint64_t u64 = *((uint64_t*)m_data);
			u64 = xntohll(u64);

			m_data += 8u; 
			m_size -= 8u;

			return u64;
		}

		// 提取指定长度的缓冲区
		const char * Pop_fetch_ptr(size_t k) const
		{
			if (m_size < k)
				throw DSError("[Unpack::Pop_fetch_ptr] not enough data");

			const char * p = m_data;

			m_data += k;
			m_size -= k;

			return p;
		}

        // 提取指定长度的字符串
		std::string Pop_fetch_string(size_t k) const
		{
			return std::string(Pop_fetch_ptr(k), k);
		}

        // 不指定长度，提取字符串
		StringPtr Pop_StringPtr() const
		{
			StringPtr SP;
			SP.m_Size = Pop_uint16();
			SP.m_pData = Pop_fetch_ptr(SP.m_Size);
			return SP;
		}
		StringPtr Pop_StringPtr32() const
		{
			StringPtr SP;
			SP.m_Size = Pop_uint32();
			SP.m_pData = Pop_fetch_ptr(SP.m_Size);
			return SP;
		}

        // 不指定长度，提取字符串
		std::string Pop_string() const
		{
			StringPtr SP = Pop_StringPtr();
			return std::string(SP.Data(), SP.Size());
		}
		std::string Pop_string32() const
		{
			StringPtr SP = Pop_StringPtr32();
			return std::string(SP.Data(), SP.Size());
		}

		const char * Data() const { return m_data; }
		size_t Size() const	  { return m_size; }

		bool Empty() const	  { return Size() == 0; }

		operator const void *() const { return m_data; }
		bool operator!() const { return (NULL == m_data); }
	};

	// 序列化/反序列化 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	struct Marshallable
	{
		virtual ~Marshallable() {}

		virtual void Marshal(Pack &) const = 0;
		virtual void Unmarshal(const Unpack &) = 0;
	};

	struct Voidmable : public Marshallable
	{
		virtual void Marshal(Pack &) const {}
		virtual void Unmarshal(const Unpack &) {}
	};

	inline Pack & operator << (Pack & p, const Marshallable & m)
	{
		m.Marshal(p);
		return p;
	}

	inline const Unpack & operator >> (const Unpack & p, const Marshallable & m)
	{
		const_cast<Marshallable &>(m).Unmarshal(p);
		return p;
	}

	// 基础类型序列化/反序列化 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	inline Pack & operator << (Pack & p, bool b)
	{
		p.Push_uint8(b ? 1 : 0);
		return p;
	}

	inline Pack & operator << (Pack & p, uint8_t  u8)
	{
		p.Push_uint8(u8);
		return p;
	}

	inline Pack & operator << (Pack & p, uint16_t  u16)
	{
		p.Push_uint16(u16);
		return p;
	}

	inline Pack & operator << (Pack & p, uint32_t  u32)
	{
		p.Push_uint32(u32);
		return p;
	}

	inline Pack & operator << (Pack & p, uint64_t  u64)
	{
		p.Push_uint64(u64);
		return p;
	}

	inline Pack & operator << (Pack & p, int8_t  i8)
	{
		p.Push_uint8((uint8_t)i8);
		return p;
	}

	inline Pack & operator << (Pack & p, int16_t  i16)
	{
		p.Push_uint16((uint16_t)i16);
		return p;
	}

	inline Pack & operator << (Pack & p, int32_t i32)
	{
		p.Push_uint32((uint32_t)i32);
		return p;
	}

	inline Pack & operator << (Pack & p, int64_t i64)
	{
		p.Push_uint64((uint64_t)i64);
		return p;
	}

	inline Pack & operator << (Pack & p, const std::string & str)
	{
		p.Push_string(str);
		return p;
	}

	inline Pack & operator << (Pack & p, const StringPtr & SP)
	{
		p.Push_string(SP);
		return p;
	}

	inline const Unpack & operator >> (const Unpack & up, bool & b)
	{
		b =  (up.Pop_uint8() == 0) ? false : true;
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, uint8_t & u8)
	{
		u8 =  up.Pop_uint8();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, uint16_t & u16)
	{
		u16 =  up.Pop_uint16();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, uint32_t & u32)
	{
		u32 =  up.Pop_uint32();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, uint64_t & u64)
	{
		u64 =  up.Pop_uint64();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, int8_t & i8)
	{
		i8 =  (int8_t)up.Pop_uint8();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, int16_t & i16)
	{
		i16 =  (int16_t)up.Pop_uint16();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, int32_t & i32)
	{
		i32 = (int32_t)up.Pop_uint32();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, int64_t & i64)
	{
		i64 = (int64_t)up.Pop_uint64();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, std::string & str)
	{
		str = up.Pop_string();
		return up;
	}

	inline const Unpack & operator >> (const Unpack & up, StringPtr & SP)
	{
		SP = up.Pop_StringPtr();
		return up;
	}

	// 容器类型序列化/反序列化 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	template < typename ContainerClass >
	inline void Marshal_Container(Pack & p, const ContainerClass & c)
	{
		p.Push_uint32( uint32_t(c.size()) );
		for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
		{
			p << *i;
		}
	}

	template < typename OutputIterator >
	inline void Unmarshal_Container(const Unpack & up, OutputIterator i)
	{
		for (uint32_t count = up.Pop_uint32(); count > 0; --count)
		{
			typename OutputIterator::container_type::value_type tmp;
			up >> tmp;
			*i = tmp;
			++i;
		}
	}

	template < typename OutputContainer>
	inline void Unmarshal_ContainerEx(const Unpack & p, OutputContainer & c)
	{
		for (uint32_t count = p.Pop_uint32(); count > 0; --count)
		{
			typename OutputContainer::value_type tmp;
			p >> tmp;
			c.push_back(tmp);
		}
	}

	template <class T1, class T2>
	inline Pack& operator << (Pack & p, const std::pair<T1, T2> & pair)
	{
		p << pair.first << pair.second;
		return p;
	}

	template <class T1, class T2>
	inline const Unpack& operator >> (const Unpack & up, std::pair<const T1, T2> & pair)
	{
		const T1 & m = pair.first;
		T1 & m2 = const_cast<T1 &>(m);
		up >> m2 >> pair.second;
		return up;
	}

	template <class T1, class T2>
	inline const Unpack& operator >> (const Unpack& up, std::pair<T1, T2>& pair)
	{
		up >> pair.first >> pair.second;
		return up;
	}

	template <class T>
	inline Pack& operator << (Pack& p, const std::vector<T>& vec)
	{
		Marshal_Container(p, vec);
		return p;
	}

	template <class T>
	inline const Unpack& operator >> (const Unpack& up, std::vector<T>& vec)
	{
		Unmarshal_Container(up, std::back_inserter(vec));
		return up;
	}

	template <class T>
	inline Pack& operator << (Pack& p, const std::set<T>& set)
	{
		Marshal_Container(p, set);
		return p;
	}

	template <class T>
	inline const Unpack& operator >> (const Unpack& up, std::set<T>& set)
	{
		Unmarshal_Container(up, std::inserter(set, set.begin()));
		return up;
	}

	template <class T1, class T2>
	inline Pack& operator << (Pack& p, const std::map<T1, T2>& map)
	{
		Marshal_Container(p, map);
		return p;
	}

	template <class T1, class T2>
	inline const Unpack& operator >> (const Unpack& up, std::map<T1, T2>& map)
	{
		Unmarshal_Container(up, std::inserter(map, map.begin()));
		return up;
	}

	// 应用辅助序列化/反序列化 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	inline void Packet2String(const Marshallable &obj, std::string & str)
	{
		PackBuffer buffer;
		Pack pack(buffer);
		obj.Marshal(pack);
		str.assign(pack.Data(), pack.Size());
	}

	inline bool String2Packet(const std::string & str, Marshallable & obj)
	{
		try 
		{
			Unpack unpack(str.data(), str.size());
			obj.Unmarshal(unpack);
		} 
		catch (const DSError & e)
		{
			return false;
		}

		return true;
	}
}
