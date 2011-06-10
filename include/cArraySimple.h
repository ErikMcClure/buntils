// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SIMPLE_H__
#define __C_ARRAY_SIMPLE_H__

#include <memory.h>
#include <malloc.h>

namespace bss_util {
  /* Very simple "dynamic" array. Designed to be used when size must be maintained at an exact value. */
  template<class T, typename SizeType=unsigned int>
  class __declspec(dllexport) cArraySimple
  {
  public:
    inline cArraySimple<T,SizeType>(const cArraySimple<T,SizeType>& copy) : _array((T*)malloc(!copy._size?1:copy._size*sizeof(T))), _size(copy._size)
    {
      memcpy(_array,copy._array,_size*sizeof(T));
    }
    inline explicit cArraySimple<T,SizeType>(SizeType size) : _array((T*)malloc(!size?1:size*sizeof(T))), _size(size)
    {
    }
    inline ~cArraySimple<T,SizeType>()
    {
      free(_array);
    }
    inline SizeType Size() const { return _size; }
    inline void SetSize(SizeType nsize)
    {
      if(nsize==_size) return;
      T* narray = (T*)malloc(sizeof(T)*nsize);
      memcpy(narray,_array,sizeof(T)*((nsize<_size)?(nsize):(_size)));
      free(_array);
      _array=narray;
      _size=nsize;
    }
    inline void Remove(SizeType index)
    {
      memmove(_array+index,_array+index+1,sizeof(T)*(_size-index-1));
      --_size;
    }
    inline void Insert(T item, SizeType location)
    {
      SizeType nsize=_size+1;
      T* narray = (T*)malloc(sizeof(T)*nsize);
      memcpy(narray,_array,location*sizeof(T));
      narray[location]=item;
      memcpy(narray+location+1,_array+location,(_size-location)*sizeof(T));
      free(_array);
      _array=narray;
      _size=nsize;
    }
    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline cArraySimple<T,SizeType>& operator=(const cArraySimple<T,SizeType>& copy)
    {
      free(_array);
      _size=copy._size;
      _array=(T*)malloc(_size*sizeof(T));
      memcpy(_array,copy._array,_size*sizeof(T));
      return *this;
    }
    inline cArraySimple<T,SizeType>& operator +=(const cArraySimple<T,SizeType>& add)
    {
      SizeType oldsize=_size;
      SetSize(_size+add._size);
      memcpy(_array+oldsize,add._array,add._size*sizeof(T));
      return *this;
    }
    inline cArraySimple<T,SizeType> operator +(const cArraySimple<T,SizeType>& add)
    {
      cArraySimple<T,SizeType> retval(*this);
      retval+=add;
      return retval;
    }

  protected:
    T* _array;
    SizeType _size;
  };

  /* Very simple "dynamic" array that calls the constructor and destructor */
  template<class T, typename SizeType=unsigned int>
  class __declspec(dllexport) cArrayConstruct
  {
  public:
    inline cArrayConstruct(const cArrayConstruct& copy) : _array((T*)malloc(copy._size*sizeof(T))), _size(copy._size)
    {
      memcpy(_array,copy._array,_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
    }
    inline explicit cArrayConstruct(SizeType size) : _array((T*)malloc(size*sizeof(T))), _size(size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
    }
    inline ~cArrayConstruct()
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      free(_array);
    }
    inline SizeType Size() const { return _size; }
    inline void SetSize(SizeType nsize)
    {
      if(nsize==_size) return;
      T* narray = (T*)malloc(sizeof(T)*nsize);
      memcpy(narray,_array,sizeof(T)*((nsize<_size)?(nsize):(_size)));

      if(((int)(nsize-_size))<0) { //we removed some so we need to destroy them
        for(SizeType i = _size; i > nsize;)
          (_array+(--i))->~T();
      } else { //we created some so we need to construct them
        for(SizeType i = _size; i < nsize; ++i)
          new(narray+i) T();
      }

      free(_array);
      _array=narray;
      _size=nsize;
    }
    inline void Remove(SizeType index)
    {
      _array[index].~T();
      memmove(_array+index,_array+index+1,sizeof(T)*(_size-index-1));
      --_size;
    }
    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline cArrayConstruct<T,SizeType>& operator=(const cArrayConstruct<T,SizeType>& copy)
    {
      free(_array);
      _size=copy._size;
      _array=(T*)malloc(_size*sizeof(T));
      memcpy(_array,copy._array,_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
      return *this;
    }
    inline cArrayConstruct<T,SizeType>& operator +=(const cArrayConstruct<T,SizeType>& add)
    {
      SizeType nsize=_size+add._size;
      T* narray = (T*)malloc(sizeof(T)*nsize);
      memcpy(narray,_array,_size*sizeof(T));
      memcpy(narray+_size,add._array,add._size*sizeof(T));
      free(_array);
      _array=narray;
      
      for(SizeType i = _size; i < nsize; ++i)
        new (_array+i) T();

      _size=nsize;
      return *this;
    }
    inline cArrayConstruct<T,SizeType> operator +(const cArrayConstruct<T,SizeType>& add)
    {
      cArrayConstruct<T,SizeType> retval(*this);
      retval+=add;
      return retval;
    }

  protected:
    T* _array;
    SizeType _size;
  };
}

#endif