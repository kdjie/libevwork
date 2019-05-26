//============================================================================
// Name        : dstypes.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <sys/types.h>

#ifndef __cplusplus

	typedef unsigned char bool;
	#define     true    1
	#define     false   0

#endif

#ifdef  __GNUC__

	#include <stdint.h>

#elif   defined(_MSC_VER)

	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef signed int int32_t;
	typedef signed __int64 int64_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
	typedef unsigned __int64 uint64_t;

#endif

namespace ds
{
    struct StringPtr
    {
        const char * m_pData;
        size_t m_Size;

        StringPtr(const char * pData = "", size_t Size = 0)
        : m_pData(pData)
        , m_Size(Size)
        {
        }
		StringPtr(const StringPtr & SP)
		{
			m_pData = SP.m_pData;
			m_Size = SP.m_Size;
		}

        void Set(const char * pData, size_t Size)
        {
            m_pData = pData;
            m_Size = Size;
        }

        bool Empty() const
        {
            return (m_Size == 0);
        }

        const char * Data() const
        {
            return m_pData;
        }

        size_t Size() const
        {
            return m_Size;
        }

		StringPtr& operator = (const StringPtr & SP)
		{
			m_pData = SP.m_pData;
			m_Size = SP.m_Size;
			return *this;
		}
    };
}
