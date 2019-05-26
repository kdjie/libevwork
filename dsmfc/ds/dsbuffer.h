//============================================================================
// Name        : dsbuffer.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new>

namespace ds
{
	template <unsigned BlockSize>
	struct Block_Allocator_MallocFree
	{
	    enum { AllocBlockSize = BlockSize };

		static char * ordered_malloc(size_t BlockCount)
		{
			return (char *)malloc(AllocBlockSize * BlockCount);
		}
		static void ordered_free(char * const pBlock, size_t)
		{
			free(pBlock);
		}
	};

	template <unsigned BlockSize>
	struct Block_Allocator_NewDelete
	{
	    enum { AllocBlockSize = BlockSize };

		static char * ordered_malloc(size_t BlockCount)
		{
			return new (std::nothrow) char[AllocBlockSize * BlockCount];
		}
		static void ordered_free(char * const pBlock, size_t)
		{
			delete[] pBlock;
		}
	};

#ifdef  __ALLOCATOR_NEW_DELETE__
	typedef Block_Allocator_NewDelete<1*1024> block_alloc_1k;
	typedef Block_Allocator_NewDelete<2*1024> block_alloc_2k;
	typedef Block_Allocator_NewDelete<4*1024> block_alloc_4k;
	typedef Block_Allocator_NewDelete<8*1024> block_alloc_8k;
	typedef Block_Allocator_NewDelete<16*1024> block_alloc_16k;
	typedef Block_Allocator_NewDelete<32*1024> block_alloc_32k;
#else
	typedef Block_Allocator_MallocFree<1*1024> block_alloc_1k;
	typedef Block_Allocator_MallocFree<2*1024> block_alloc_2k;
	typedef Block_Allocator_MallocFree<4*1024> block_alloc_4k;
	typedef Block_Allocator_MallocFree<8*1024> block_alloc_8k;
	typedef Block_Allocator_MallocFree<16*1024> block_alloc_16k;
	typedef Block_Allocator_MallocFree<32*1024> block_alloc_32k;
#endif

	template <typename BlockAllocator = block_alloc_4k, unsigned MaxBlocks = 1>
	class DSBuffer
	{
	private:
		char * m_pData;
		size_t m_Size;
		size_t m_BlockCount;

	public:
		enum { maxblocks = MaxBlocks };
		enum { npos = size_t(-1) };

		typedef BlockAllocator allocator;

		DSBuffer() : m_pData(NULL), m_Size(0), m_BlockCount(0) {}
		virtual ~DSBuffer() { __Free(); }

		char * Data() { return m_pData; }
		size_t Size() const { return m_Size; }

		bool Empty() const	 { return Size() == 0; }
		size_t BlockCount() const	 { return m_BlockCount; }
		size_t BlockSize() const { return allocator::AllocBlockSize; }
		size_t Capacity() const  { return allocator::AllocBlockSize * m_BlockCount; }
		size_t MaxCapacity() const	 { return allocator::AllocBlockSize * maxblocks; }
		size_t CurFree() const { return Capacity() - Size(); }
		size_t MaxFree() const	 { return MaxCapacity() - Size(); }

        inline bool Reserve(size_t S);
		inline bool Resize(size_t S, char C = 0);
		inline bool Append(const char * P, size_t S);
		inline bool Replace(size_t pos, const char * P, size_t S);
		inline void Erase(size_t pos = 0, size_t S = npos, bool hold = false);

	protected:
	    char * __Tail() { return m_pData + m_Size; }
	    inline void __Free();
        // 尝试增加容量，S为需要增加的数据长度，可能需要预先扩容
		inline bool __TryIncreaseCapacity(size_t S);

	private:
		DSBuffer(const DSBuffer &);
		void operator = (const DSBuffer &);
	};

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline bool DSBuffer<BlockAllocator, MaxBlocks >::Reserve(size_t S)
	{
		return (S <= Capacity() || __TryIncreaseCapacity(S - Size()));
	}

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline bool DSBuffer<BlockAllocator, MaxBlocks >::Resize(size_t S, char C)
	{
		if (S > Size())
		{
			size_t IS = S - Size();
			if (!__TryIncreaseCapacity(IS))
				return false;

			memset(__Tail(), C, IS);
		}

		m_Size = S;

		return true;
	}

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline bool DSBuffer<BlockAllocator, MaxBlocks >::Append(const char * P, size_t S)
	{
		if (S == 0)
			return true;

		if (!__TryIncreaseCapacity(S))
		    return false;

		memmove(__Tail(), P, S);
		m_Size += S;

		return true;
	}

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline bool DSBuffer<BlockAllocator, MaxBlocks >::Replace(size_t pos, const char * P, size_t S)
	{
	    // pos可以设为-1，语义上表示向尾部添加
		if (pos >= Size())
			return Append(P, S);

        // 当替换长度盖过当前长度时
		if (pos + S >= Size())
		{
			m_Size = pos;
			return Append(P, S);
		}

        // 替换区域在当前数据之内
		if (S > 0)
		{
		    memmove(m_pData + pos, P, S);
		}

		return true;
	}

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline void DSBuffer<BlockAllocator, MaxBlocks >::Erase(size_t pos, size_t S, bool hold)
	{
	    // 确保pos有效
		assert(pos <= Size());

		size_t CanErase = Size() - pos;
		if (S >= CanErase)
		{
		    // 全部清除
		    m_Size = pos;
		}
		else
		{
			m_Size -= S;
			memmove(m_pData + pos, m_pData + pos + S, CanErase - S);
		}

        // 是否保留
		if (Empty() && !hold)
		{
		    __Free();
		}
	}

	template <typename BlockAllocator, unsigned MaxBlocks>
	inline void DSBuffer<BlockAllocator, MaxBlocks >::__Free()
	{
        if (m_BlockCount > 0)
        {
            allocator::ordered_free(m_pData, m_BlockCount);

            m_pData = NULL;
            m_Size = 0;
            m_BlockCount = 0;
        }
	}

    // 尝试增加容量，S为需要增加的数据长度，可能需要预先扩容
	template <typename BlockAllocator, unsigned MaxBlocks>
	inline bool DSBuffer<BlockAllocator, MaxBlocks >::__TryIncreaseCapacity(size_t S)
	{
		if (S == 0)
			return true;

		size_t Free = CurFree();
		if (S <= Free)
			return true;

	    S -= Free;

		size_t NewBlockCount = m_BlockCount;

		NewBlockCount += S/BlockSize();
		if ( (S%BlockSize()) != 0 )
			NewBlockCount ++;

		if (NewBlockCount > maxblocks)
			return false;

		char * pNew = (char*)(allocator::ordered_malloc(NewBlockCount));
		if (pNew == NULL)
			return false;

		if (m_BlockCount > 0)
		{
			memcpy(pNew, m_pData, m_Size);
			allocator::ordered_free(m_pData, m_BlockCount);
		}

		m_pData = pNew;
		m_BlockCount = NewBlockCount;

		return true;
	}
}
