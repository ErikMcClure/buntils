// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_STACK_H__
#define __C_BSS_STACK_H__

#include "cArraySimple.h"
#include "bss_util.h"

namespace bss_util {
  // Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename SizeType=unsigned int, typename ArrayType=cArraySimple<T,SizeType>>
  class BSS_COMPILER_DLLEXPORT cBSS_Stack : protected ArrayType
  {
  protected:
    using ArrayType::_array;
    using ArrayType::_size;

  public:
    inline cBSS_Stack(const cBSS_Stack& copy) : ArrayType(copy), _length(copy._length) {}
    inline cBSS_Stack(cBSS_Stack&& mov) : ArrayType(std::move(mov)), _length(mov._length) {}
    inline explicit cBSS_Stack(SizeType init=0) : ArrayType(init), _length(0) {}
    inline ~cBSS_Stack() {}
    BSS_FORCEINLINE void BSS_FASTCALL Push(const T& value) { _push(value); }
    BSS_FORCEINLINE void BSS_FASTCALL Push(T&& value) { _push(std::move(value)); }
    BSS_FORCEINLINE T BSS_FASTCALL Pop() { assert(_length!=0); return std::move(_array[--_length]); }
    BSS_FORCEINLINE T& BSS_FASTCALL Peek() { assert(_length!=0); return _array[_length-1]; }
    BSS_FORCEINLINE const T& BSS_FASTCALL Peek() const { assert(_length!=0); return _array[_length-1]; }
    BSS_FORCEINLINE void Discard() { --_length; }
    BSS_FORCEINLINE void Clear() { _length=0; }
    BSS_FORCEINLINE SizeType Length() const { return _length; }
    BSS_FORCEINLINE SizeType Capacity() const { return _size; }
    inline cBSS_Stack& operator=(const cBSS_Stack& copy) { ArrayType::operator=(copy); _length=copy._length; return *this; }
    inline cBSS_Stack& operator=(cBSS_Stack&& mov) { ArrayType::operator=(std::move(mov)); _length=mov._length; return *this; }

  protected:
    template<typename U>
    void BSS_FASTCALL _push(U && value) { if(++_length>_size) ArrayType::SetSize(fbnext(_size)); assert(_length-1<_size); _array[_length-1]=std::forward<U>(value); }
    
    SizeType _length;
  };

}

#endif
