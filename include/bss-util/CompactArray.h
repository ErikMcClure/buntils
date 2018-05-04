// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __COMPACT_ARRAY_H__BSS__
#define __COMPACT_ARRAY_H__BSS__

#include "Array.h"
#include "bss_util.h"

namespace bss {
  // Array that uses a inline stack for smaller arrays before allocating something on the heap. Can only be used with simple data.
  template<class T, int I = 2, class CType = size_t, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT CompactArray final : Alloc
  {
  protected:
    typedef T Ty;
    typedef CType CT;
    static_assert(std::is_unsigned<CT>::value, "CT must be unsigned");
    static const CT COMPACTFLAG = CT(1) << ((sizeof(CT) << 3) - 1);
    static const CT COMPACTMASK = ~COMPACTFLAG;

  public:
    inline CompactArray(const CompactArray& copy) : Alloc(copy), _length(COMPACTFLAG)
    {
      SetLength(copy.Length());
      MEMCPY(begin(), Length() * sizeof(T), copy.begin(), copy.Length() * sizeof(T));
    }
    inline CompactArray(CompactArray&& mov) : Alloc(std::move(mov))
    {
      MEMCPY(this, sizeof(CompactArray), &mov, sizeof(CompactArray));
      mov._length = COMPACTFLAG;
    }
    inline explicit CompactArray(const Slice<const T, CT>& slice) : _length(COMPACTFLAG)
    {
      SetLength(slice.length);
      MEMCPY(begin(), Length() * sizeof(T), slice.start, slice.length * sizeof(T));
    }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline explicit CompactArray(typename Alloc::policy_type* policy) : Alloc(policy), _length(COMPACTFLAG) { }
    inline CompactArray() : _length(COMPACTFLAG) { }
    inline CompactArray(const std::initializer_list<T>& list) : _length(COMPACTFLAG)
    {
      SetLength(list.size());
      auto end = list.end();
      CT j = 0;
      CT len = Length();

      for(auto i = list.begin(); i != end && j < len; ++i)
        new(_array + (j++)) T(*i);
    }
    inline ~CompactArray() 
    { 
      if(!(_length&COMPACTFLAG) && _array != 0)
        Alloc::deallocate(_array, _capacity);
    }
    inline CT Add(T item) 
    {
      SetCapacity(Length() + 1);
      new(end()) T(item);
      return (_length++)&COMPACTMASK; 
    }
    inline void Remove(CT index) { RemoveRangeSimple<T, CT>(begin(), (_length--)&COMPACTMASK, index, 1); }
    BSS_FORCEINLINE void RemoveLast() { Remove(Length() - 1); }
    BSS_FORCEINLINE void Insert(T item, CT index = 0)
    {
      SetCapacity(Length() + 1);
      InsertRangeSimple<T, CT>(begin(), (_length++)&COMPACTMASK, index, &item, 1);
    }
    BSS_FORCEINLINE void Set(const Slice<const T, CT>& slice) { Set(slice.start, slice.length); }
    BSS_FORCEINLINE void Set(const T* p, CT n)
    {
      SetLength(n);
      MEMCPY(begin(), Length() * sizeof(T), p, n * sizeof(T));
    }
    BSS_FORCEINLINE bool Empty() const { return !Length(); }
    BSS_FORCEINLINE void Clear() { _length = (_length&COMPACTFLAG); }
    inline void SetLength(CT length)
    {
      SetCapacity(length);
#ifdef BSS_DEBUG
      if(length > Length())
        bssFillN<T>(end(), length - Length(), 0xfd);
#endif
      _length = (length | (_length&COMPACTFLAG));
    }
    inline void SetCapacity(CT capacity)
    {
      if(capacity > I)
      {
        capacity = T_FBNEXT(capacity);

        if(_length&COMPACTFLAG)
        {
          T* a = Alloc::allocate(capacity, 0);
          _length = Length();

          if(_length)
            MEMCPY(a, capacity * sizeof(T), _internal, _length * sizeof(T));

          _array = a;
          _capacity = capacity;
        }
        else
        {
          _array = Alloc::allocate(capacity, _array, _capacity);
          _capacity = capacity;
        }
      }
    }
    BSS_FORCEINLINE CT Length() const { return _length&COMPACTMASK; }
    BSS_FORCEINLINE CT Capacity() const { return (_length&COMPACTFLAG) ? I : _capacity; }
    BSS_FORCEINLINE const T& Front() const { assert(Length() > 0); return begin()[0]; }
    BSS_FORCEINLINE const T& Back() const { assert(Length() > 0); return begin()[Length() - 1]; }
    BSS_FORCEINLINE T& Front() { assert(Length() > 0); return begin()[0]; }
    BSS_FORCEINLINE T& Back() { assert(Length() > 0); return begin()[Length() - 1]; }
    BSS_FORCEINLINE const T* begin() const noexcept { return (_length&COMPACTFLAG) ? _internal : _array; }
    BSS_FORCEINLINE const T* end() const noexcept { return begin() + Length(); }
    BSS_FORCEINLINE T* begin() noexcept { return (_length&COMPACTFLAG) ? _internal : _array; }
    BSS_FORCEINLINE T* end() noexcept { return begin() + Length(); }
    BSS_FORCEINLINE Slice<T, CT> GetSlice() const noexcept { return Slice<T, CT>(begin(), Length()); }
    BSS_FORCEINLINE operator T*() { return begin(); }
    BSS_FORCEINLINE operator const T*() const { return begin(); }
    inline CompactArray& operator=(const CompactArray& copy)
    {
      SetLength(copy.Length());
      MEMCPY(begin(), Length() * sizeof(T), copy.begin(), copy.Length() * sizeof(T));
      return *this;
    }
    inline CompactArray& operator=(CompactArray&& mov)
    {
      if(!(_length&COMPACTFLAG) && _array != 0)
        Alloc::deallocate(_array, _capacity);
      Alloc::operator=(std::move(mov));
      MEMCPY(this, sizeof(CompactArray), &mov, sizeof(CompactArray));
      mov._length = COMPACTFLAG;
      return *this;
    }
    inline CompactArray& operator=(const Slice<const T, CT>& copy) 
    { 
      Set(copy); 
      return *this; 
    }

    typedef T SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      s.template EvaluateArray<CompactArray, T, &_serializeAdd<Engine>, CT, &CompactArray::SetLength>(*this, Length(), id);
    }

  protected:
    template<typename Engine>
    inline static void _serializeAdd(Serializer<Engine>& e, CompactArray& obj, int& n)
    {
      obj.SetLength(obj.Length() + 1);
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }

    union
    {
      struct {
        T* _array;
        CT _capacity;
      };
      struct {
        T _internal[I];
      };
    };

    CT _length;
  };
}

#endif
