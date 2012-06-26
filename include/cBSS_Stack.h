// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_STACK_H__
#define __C_BSS_STACK_H__

#include "cArraySimple.h"
#include "bss_util.h"
#include "bss_traits.h"

namespace bss_util {
  /* Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user. */
  template<class T, typename SizeType=unsigned int, typename ArrayClass=cArraySimple<T,SizeType>>
  class BSS_COMPILER_DLLEXPORT cBSS_Stack : protected ArrayClass
  {
  public:
    inline cBSS_Stack(const cBSS_Stack& copy) : ArrayClass(copy), _length(copy._length) {}
    inline cBSS_Stack(cBSS_Stack&& mov) : ArrayClass(std::move(mov)), _length(mov._length) {}
    inline explicit cBSS_Stack(int init=8) : ArrayClass(8), _length(0) {}
    inline ~cBSS_Stack() {}
    inline BSS_FORCEINLINE void BSS_FASTCALL Push(const T& value) { _push(value); }
    inline BSS_FORCEINLINE void BSS_FASTCALL Push(T&& value) { _push(std::move(value)); }
    inline T BSS_FASTCALL Pop() { assert(_length!=0); return T(std::move(_array[--_length])); }
    inline T& BSS_FASTCALL Top() { assert(_length!=0); return _array[_length-1]; }
    inline const T& BSS_FASTCALL Top() const { assert(_length!=0); return _array[_length-1]; }
    inline void Clear() { _length=0; }
    inline SizeType Length() const { return _length; }
    inline cBSS_Stack& operator=(const cBSS_Stack& copy) { ArrayClass::operator=(copy); _length=copy._length; return *this; }
    inline cBSS_Stack& operator=(cBSS_Stack&& mov) { ArrayClass::operator=(std::move(mov)); _length=mov._length; return *this; }

  protected:
    template<typename U>
    inline void BSS_FASTCALL _push(U && value) { if(++_length>_size) SetSize(fbnext(_size)); assert(_length-1<_size); _array[_length-1]=std::forward<U>(value); }
    
    SizeType _length;
  };

}

#endif