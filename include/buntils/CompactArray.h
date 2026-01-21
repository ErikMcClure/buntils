// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __COMPACT_ARRAY_H__BUN__
#define __COMPACT_ARRAY_H__BUN__

#include "Array.h"
#include "buntils.h"

namespace bun {
  // Array that uses a inline stack for smaller arrays before allocating something on the heap.
  template<class T, int I = 2, class CType = size_t, typename Alloc = StandardAllocator<T>>
    requires(std::is_unsigned<CType>::value && std::is_trivially_copyable_v<T>)
  class BUN_COMPILER_DLLEXPORT CompactArray final : Alloc
  {
  protected:
    using Ty                    = T;
    using CT                    = CType;
    static const CT COMPACTFLAG = CT(1) << ((sizeof(CT) << 3) - 1);
    static const CT COMPACTMASK = ~COMPACTFLAG;

  public:
    inline CompactArray(const CompactArray& copy) : Alloc(copy), _length(COMPACTFLAG)
    {
      SetLength(copy.size());
      MEMCPY(begin(), size() * sizeof(T), copy.begin(), copy.size() * sizeof(T));
    }
    inline CompactArray(CompactArray&& mov) : Alloc(std::move(mov))
    {
      MEMCPY(this, sizeof(CompactArray), &mov, sizeof(CompactArray));
      mov._length = COMPACTFLAG;
    }
    inline explicit CompactArray(std::span<const T>& s)
      requires std::is_default_constructible_v<Alloc>
      : _length(COMPACTFLAG)
    {
      SetLength(s.size());
      MEMCPY(begin(), size() * sizeof(T), s.data(), s.size_bytes());
    }
    inline explicit CompactArray(const Alloc& alloc) : Alloc(alloc), _length(COMPACTFLAG) {}
    inline CompactArray()
      requires std::is_default_constructible_v<Alloc>
      : _length(COMPACTFLAG)
    {}
    inline CompactArray(const std::initializer_list<T>& list)
      requires std::is_default_constructible_v<Alloc>
      : _length(COMPACTFLAG)
    {
      SetLength(list.size());
      auto end = list.end();
      CT j     = 0;
      CT len   = size();

      for(auto i = list.begin(); i != end && j < len; ++i)
        new(data() + (j++)) T(*i);
    }
    inline ~CompactArray()
    {
      if(!(_length & COMPACTFLAG) && !_array.empty())
        std::allocator_traits<Alloc>::deallocate(*this, _array.data(), _array.size());
    }
    inline CT Add(T item)
    {
      SetCapacity(size() + 1);
      new(end()) T(item);
      return (_length++) & COMPACTMASK;
    }
    inline void Remove(CT index) { RemoveRangeSimple<T, CT>(begin(), (_length--) & COMPACTMASK, index, 1); }
    BUN_FORCEINLINE void RemoveLast() { Remove(size() - 1); }
    BUN_FORCEINLINE void Insert(T item, CT index = 0)
    {
      SetCapacity(size() + 1);
      InsertRangeSimple<T, CT>(begin(), (_length++) & COMPACTMASK, index, &item, 1);
    }
    BUN_FORCEINLINE void Set(std::span<const T> s)
    {
      SetLength(s.size());
      MEMCPY(begin(), size() * sizeof(T), s.data(), s.size() * sizeof(T));
    }
    BUN_FORCEINLINE bool Empty() const { return !size(); }
    BUN_FORCEINLINE void Clear() { _length = (_length & COMPACTFLAG); }
    inline void SetLength(CT length)
    {
      SetCapacity(length);
#ifdef BUN_DEBUG
      if(length > size())
        bun_FillN(std::span(end(), length - size()), 0xfd);
#endif
      _length = (length | (_length & COMPACTFLAG));
    }
    inline void SetCapacity(size_t capacity)
    {
      if(capacity > I)
      {
        capacity = T_FBNEXT(capacity);

        if(_length & COMPACTFLAG)
        {
          T* a    = std::allocator_traits<Alloc>::allocate(*this, capacity);
          _length = size();

          if(_length)
            MEMCPY(a, capacity * sizeof(T), _internal, _length * sizeof(T));

          _array = std::span<T>(a, capacity);
        }
        else
          _array = standard_realloc<Alloc>(*this, capacity, _array);
      }
    }
    BUN_FORCEINLINE CT Capacity() const { return (_length & COMPACTFLAG) ? I : _array.size(); }
    BUN_FORCEINLINE const T& Front() const
    {
      assert(size() > 0);
      return begin()[0];
    }
    BUN_FORCEINLINE const T& Back() const
    {
      assert(size() > 0);
      return begin()[size() - 1];
    }
    BUN_FORCEINLINE T& Front()
    {
      assert(size() > 0);
      return begin()[0];
    }
    BUN_FORCEINLINE T& Back()
    {
      assert(size() > 0);
      return begin()[size() - 1];
    }
    BUN_FORCEINLINE const T* begin() const noexcept { return data(); }
    BUN_FORCEINLINE const T* end() const noexcept { return begin() + size(); }
    BUN_FORCEINLINE T* begin() noexcept { return data(); }
    BUN_FORCEINLINE T* end() noexcept { return begin() + size(); }
    inline CT size() const noexcept { return _length & COMPACTMASK; }
    inline T* data() noexcept { return (_length & COMPACTFLAG) ? _internal : _array.data(); }
    inline const T* data() const noexcept { return (_length & COMPACTFLAG) ? _internal : _array.data(); }
    BUN_FORCEINLINE const T& operator[](CType i) const
    {
      assert(i < size());
      return data()[i];
    }
    BUN_FORCEINLINE T& operator[](CType i)
    {
      assert(i < size());
      return data()[i];
    }
    inline CompactArray& operator=(const CompactArray& copy)
    {
      SetLength(copy.size());
      MEMCPY(begin(), size() * sizeof(T), copy.begin(), copy.size() * sizeof(T));
      return *this;
    }
    inline CompactArray& operator=(CompactArray&& mov)
    {
      if(!(_length & COMPACTFLAG) && !_array.empty())
        std::allocator_traits<Alloc>::deallocate(*this, _array.data(), _array.size());
      Alloc::operator=(std::move(mov));
      MEMCPY(this, sizeof(CompactArray), &mov, sizeof(CompactArray));
      mov._length = COMPACTFLAG;
      return *this;
    }
    inline CompactArray& operator=(std::span<const T> copy)
    {
      Set(copy);
      return *this;
    }

    using SerializerArray = T;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      s.template EvaluateArray<CompactArray, T, &_serializeAdd<Engine>, CT, &CompactArray::SetLength>(*this, size(), id);
    }

  protected:
    template<typename Engine> inline static void _serializeAdd(Serializer<Engine>& e, CompactArray& obj, int& n)
    {
      obj.SetLength(obj.size() + 1);
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }

    union
    {
      struct
      {
        std::span<T> _array;
      };
      struct
      {
        T _internal[I];
      };
    };

    CT _length;
  };
}

#endif
