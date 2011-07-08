// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SIMPLE_H__BSS__
#define __C_ARRAY_SIMPLE_H__BSS__

#include <memory.h>
#include <malloc.h>

namespace bss_util {
  /* Very simple "dynamic" array. Designed to be used when size must be maintained at an exact value. */
  template<class T, typename SizeType=unsigned int>
  class __declspec(dllexport) cArraySimple
  {
  public:
    inline cArraySimple<T,SizeType>(const cArraySimple<T,SizeType>& copy) : _array((T*)_minmalloc(copy._size*sizeof(T))), _size(copy._size)
    {
      memcpy(_array,copy._array,_size*sizeof(T));
    }
    inline explicit cArraySimple<T,SizeType>(SizeType size) : _array((T*)_minmalloc(size*sizeof(T))), _size(size)
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
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
      memcpy(narray,_array,sizeof(T)*((nsize<_size)?(nsize):(_size)));
      free(_array);
      _array=narray;
      _size=nsize;
    }
    inline void Remove(SizeType index)
    {
      memmove(_array+index,_array+index+1,sizeof(T)*(_size-index-1));
      //--_size;
    }
    inline void Insert(T item, SizeType location)
    {
      SizeType nsize=_size+1;
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
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
      _array=(T*)_minmalloc(_size*sizeof(T));
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
    inline static void* _cdecl _minmalloc(size_t n) { return malloc((n<1)?1:n); } //Malloc can legally return NULL if it tries to allocate 0 bytes
    template<typename U>
    inline void _pushback(SizeType index, SizeType length, U data) 
    {
      memmove(_array+(index+1),_array+index,length*sizeof(T));
      _array[index]=data;
    }

    T* _array;
    SizeType _size;
  };

  /* Very simple "dynamic" array that calls the constructor and destructor */
  template<class T, typename SizeType=unsigned int>
  class __declspec(dllexport) cArrayConstruct
  {
  public:
    inline cArrayConstruct(const cArrayConstruct& copy) : _array((T*)_minmalloc(copy._size*sizeof(T))), _size(copy._size)
    {
      //memcpy(_array,copy._array,_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
    }
    inline explicit cArrayConstruct(SizeType size) : _array((T*)_minmalloc(size*sizeof(T))), _size(size)
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
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
      memcpy(narray,_array,sizeof(T)*((nsize<_size)?(nsize):(_size)));

      if(nsize<_size) { //we removed some so we need to destroy them
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
      new(_array+(_size-1)) T();
    }
    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline cArrayConstruct<T,SizeType>& operator=(const cArrayConstruct<T,SizeType>& copy)
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      free(_array);
      _size=copy._size;
      _array=(T*)_minmalloc(_size*sizeof(T));
      //memcpy(_array,copy._array,_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
      return *this;
    }
    inline cArrayConstruct<T,SizeType>& operator +=(const cArrayConstruct<T,SizeType>& add)
    {
      SizeType nsize=_size+add._size;
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
      memcpy(narray,_array,_size*sizeof(T));
      //memcpy(narray+_size,add._array,add._size*sizeof(T));
      free(_array);
      _array=narray;
      
      for(SizeType i = _size; i < nsize; ++i)
        new (_array+i) T(add._array[i-_size]);

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
    inline static void* _cdecl _minmalloc(size_t n) { return malloc((n<1)?1:n); } //Malloc can legally return NULL if it tries to allocate 0 bytes
    template<typename U>
    inline void _pushback(SizeType index, SizeType length, U data) 
    {
      (_array+(index+length))->~T();
      memmove(_array+(index+1),_array+index,length*sizeof(T));
      new (_array+index) T(data);
    }

    T* _array;
    SizeType _size;
  };

  /* Typesafe array that reconstructs everything properly, without any memory moving tricks */
  template<class T, typename SizeType=unsigned int>
  class __declspec(dllexport) cArraySafe
  {
  public:
    inline cArraySafe(const cArraySafe& copy) : _array((T*)_minmalloc(copy._size*sizeof(T))), _size(copy._size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
    }
    inline explicit cArraySafe(SizeType size) : _array((T*)_minmalloc(size*sizeof(T))), _size(size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
    }
    inline ~cArraySafe()
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      free(_array);
    }
    inline SizeType Size() const { return _size; }
    inline void SetSize(SizeType nsize)
    {
      if(nsize==_size) return;
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
      
      SizeType smax = _size<nsize?_size:nsize;
      for(SizeType i = 0; i < smax; ++i) //copy over any we aren't discarding
        new (narray+i) T(_array[i]);
      for(SizeType i = smax; i < nsize; ++i) //Initialize any newcomers
        new (narray+i) T();
      for(SizeType i = 0; i < _size; ++i) //Demolish the old ones
        (_array+i)->~T();

      free(_array);
      _array=narray;
      _size=nsize;
    }
    inline void Remove(SizeType index)
    {
      --_size;
      for(SizeType i=index; i<_size;++i)
        _array[i]=_array[i+1];
      _array[_size].~T();
      new(_array+(_size++)) T();
    }
    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline cArraySafe<T,SizeType>& operator=(const cArraySafe<T,SizeType>& copy)
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      free(_array);
      _size=copy._size;
      _array=(T*)_minmalloc(_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
      return *this;
    }
    inline cArraySafe<T,SizeType>& operator +=(const cArraySafe<T,SizeType>& add)
    {
      SizeType nsize=_size+add._size;
      T* narray = (T*)_minmalloc(sizeof(T)*nsize);
      
      for(SizeType i = 0; i < _size; ++i) //copy over old ones
        new (narray+i) T(_array[i]);
      for(SizeType i = _size; i < nsize; ++i) //Copy over newcomers
        new (_array+i) T(add._array[i-_size]);
      for(SizeType i = 0; i < _size; ++i) //Demolish the old ones
        (_array+i)->~T();

      free(_array);
      _array=narray;
      _size=nsize;
      return *this;
    }
    inline cArraySafe<T,SizeType> operator +(const cArraySafe<T,SizeType>& add)
    {
      cArrayConstruct<T,SizeType> retval(*this);
      retval+=add;
      return retval;
    }

  protected:
    inline static void* _cdecl _minmalloc(size_t n) { return malloc((n<1)?1:n); }  //Malloc can legally return NULL if it tries to allocate 0 bytes
    template<typename U>
    inline void _pushback(SizeType index, SizeType length, U data) 
    {
      for(SizeType i=index+length; i>index; --i)
        _array[i]=_array[i-1];
      _array[index] = data;
    }

    T* _array;
    SizeType _size;
  };
}

#endif