// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_STACK_H__
#define __C_BSS_STACK_H__

//#include "bss_alloc.h"
#include "cArraySimple.h"
#include "bss_util.h"
#include "bss_traits.h"

namespace bss_util {
  /* Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user. */
  template<class T, class Traits=ValueTraits<T>, typename SizeType=unsigned int, typename ArrayClass=cArraySimple<T,SizeType>>
  class __declspec(dllexport) cBSS_Stack : protected ArrayClass, protected Traits
  {
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;

  public:
    inline cBSS_Stack(const cBSS_Stack& copy) : ArrayClass(copy), _length(copy._length) {}
    inline explicit cBSS_Stack(int init=8) : ArrayClass(8), _length(0) {}
    inline ~cBSS_Stack() {}
    inline void BSS_FASTCALL Push(constref value) { if(++_length>_size) SetSize(fbnext(_size)); assert(_length-1<_size); _array[_length-1]=value; }
    inline T BSS_FASTCALL Pop() { assert(_length!=0); return _array[--_length]; }
    inline reference BSS_FASTCALL Top() { assert(_length!=0); return _array[_length-1]; }
    inline constref BSS_FASTCALL Top() const { return Top(); }
    inline void Clear() { _length=0; }
    inline SizeType Length() const { return _length; }

  protected:
    SizeType _length;
  };

}

#endif