// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BYTE_QUEUE_H__BSS__ 
#define __C_BYTE_QUEUE_H__BSS__

#include "bss_defines.h"
#include "cArraySimple.h"
#include "lockless.h"

namespace bss_util
{
  // Generic queue that works on a single array and is designed to be flushed after every read-through. Uses cArraySimple due to preformance critical usage, which includes array resize time.
  template<typename SizeType=unsigned int>
  class BSS_COMPILER_DLLEXPORT cByteQueue : protected cArraySimple<unsigned char,SizeType> //cArraySimple does not have a virtual destructor. Because its inherited via protected, no one can delete it from there anyway.
  {
  protected:
    using cArraySimple<unsigned char,SizeType>::_array;
    using cArraySimple<unsigned char,SizeType>::_size;

  public:
    inline cByteQueue(const cByteQueue& copy) : cArraySimple<unsigned char,SizeType>(copy), _cur(copy._cur) {}
    inline cByteQueue(cByteQueue&& mov) : cArraySimple<unsigned char,SizeType>(std::move(mov)), _cur(mov._cur) {}
    // Constructor takes initial size
    inline explicit cByteQueue(SizeType size=64) : cArraySimple<unsigned char,SizeType>(!size?1:size), _cur(0) {}
    // Destructor
    inline ~cByteQueue() {}
    // Writes a class reference
    template<typename T>
    inline void Write(const T& ref)
    { 
      if((_cur+sizeof(T))>_size) 
        SetSize(sizeof(T)+(_size<<1));
      memcpy(_array+_cur,&ref,sizeof(T)); 
      atomic_xadd<SizeType>(&_cur,sizeof(T)); 
    }
/*#ifdef BSS_COMPILER_GCC
    template<typename... Args>
    inline void Write(Args... args)
    {
      SizeType cnt = _count<Args...>();
      if((_cur+cnt)>_size) 
        SetSize(cnt+(_size<<1));
      _write<Args...>(_cur, args...);
      atomic_xadd<SizeType>(&_cur,cnt); 
    }
#endif*/
    // Writes an arbitrary memory location
    inline void Write(void* src, SizeType length) 
    { 
      if(_cur+length>_size) 
        SetSize(length+(_size<<1));
      memcpy(_array+_cur,src,length); 
      atomic_xadd<SizeType>(&_cur,length); 
    }
    // Returns the given index as a pointer of the specified type
    template<class T>
    inline T* Read(SizeType index) { return (T*)((index<_cur)?_array+index:0); }
    // Returns the and increments the given index as a pointer of the specified type
    template<class T>
    inline T* ReadInc(SizeType& index) { SizeType refval=index; index+=sizeof(T); return (T*)((refval<_cur)?_array+refval:0); }
    // Resets the write pointer
    inline void Clear() { _cur=0; }
    // Gets the length of the used array
    inline SizeType Length() const { return _cur; }
    // Gets the total size of the buffer
    inline SizeType Capacity() const { return _size; }

    inline void* operator[](SizeType index) { return (index<_cur)?_array[index]:0; }
    inline cByteQueue& operator=(const cByteQueue& copy) { cArraySimple<unsigned char,SizeType>::operator=(copy); _cur=copy._cur; return *this; }
    inline cByteQueue& operator=(cByteQueue&& mov) { cArraySimple<unsigned char,SizeType>::operator=(std::move(mov)); _cur=mov._cur; return *this; }
    
  protected:
    SizeType _cur;
  };
}

#endif
