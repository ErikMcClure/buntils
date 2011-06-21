// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BYTE_QUEUE_H__BSS__ 
#define __C_BYTE_QUEUE_H__BSS__

#include "bss_call.h"
#include "cArraySimple.h"

namespace bss_util
{
  /* Generic queue that works on a single array and is designed to be flushed after every read-through. Uses cArraySimple due to preformance critical usage, which includes array resize time. */
  template<typename SizeType=unsigned int>
  class __declspec(dllexport) cByteQueue : protected cArraySimple<unsigned char,SizeType> //cArraySimple does not have a virtual destructor. Because its inherited via protected, no one can delete it from there anyway.
  {
  public:
    /* Constructor takes initial size */
    inline explicit cByteQueue(SizeType size=64) : cArraySimple<unsigned char,SizeType>(!size?1:size), _cur(0) {}
    /* Destructor */
    inline ~cByteQueue() {}
    /* Writes a class reference */
    template<class T>
    inline void Write(const T& ref) { if(_cur+sizeof(T)>_size) _expand();  memcpy(_array+_cur,&ref,sizeof(T)); _cur+=sizeof(T); }
    template<class T, class T2>
    inline void Write(const T& ref, const T2& ref2) { if(_cur+sizeof(T)+sizeof(T2)>_size) _expand();  memcpy(_array+_cur,&ref,sizeof(T)); _cur+=sizeof(T); memcpy(_array+_cur,&ref2,sizeof(T2)); _cur+=sizeof(T2); }
    template<class T, class T2, class T3>
    inline void Write(const T& ref, const T2& ref2, const T3& ref3) { if(_cur+sizeof(T)+sizeof(T2)+sizeof(T3)>_size) _expand();  memcpy(_array+_cur,&ref,sizeof(T)); _cur+=sizeof(T); memcpy(_array+_cur,&ref2,sizeof(T2)); _cur+=sizeof(T2); memcpy(_array+_cur,&ref3,sizeof(T3)); _cur+=sizeof(T3); }
    /* Writes an arbitrary memory location */
    inline void Write(void* src, SizeType length) { if(_cur+length>_size) _expand();  memcpy(_array+_cur,src,length); _cur+=length; }
    /* Returns the given index as a pointer of the specified type */
    template<class T>
    inline T* Read(SizeType index) { return (T*)((index<_cur)?_array+index:0); }
    /* Returns the and increments the given index as a pointer of the specified type */
    template<class T>
    inline T* ReadInc(SizeType& index) { SizeType refval=index; index+=sizeof(T); return (T*)((refval<_cur)?_array+refval:0); }
    /* Resets the write pointer */
    inline void Clear() { _cur=0; }
    /* Gets the length of the used array */
    inline SizeType Length() const { return _cur; }
    /* Gets the total size of the buffer */
    inline SizeType Size() const { return _size; }

    inline void* operator[](SizeType index) { return (index<_cur)?_array[index]:0; }
    inline cByteQueue& operator=(const cByteQueue& copy) { cArraySimple<unsigned char,SizeType>::operator=(copy); _cur=copy._cur; return *this; }
  
  protected:
    void _expand()
    { 
      SizeType nsize=_size<<1;
      unsigned char* narray = (unsigned char*)malloc(nsize);
      memcpy(narray,_array,_size);
      free(_array);
      _array=narray;
      _size=nsize;
    }

    SizeType _cur;
  };
}

#endif