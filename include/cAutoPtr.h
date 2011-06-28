// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AUTOPTR_H__BSS__
#define __C_AUTOPTR_H__BSS__

#include <memory>

namespace bss_util
{
  template<class _Ty> // Trick that uses implicit conversion that allows cAutoPtr below to take _Ty* pointers without confusing itself for them.
  struct cAutoPtr_Ref { inline cAutoPtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  /* Modification of the standard auto ptr taking advantage of several tricks to make management of the pointer easier */
  template<class _Ty>
  class cAutoPtr : public std::auto_ptr<_Ty>
  {
  public:
    inline cAutoPtr(const cAutoPtr& copy) : std::auto_ptr<_Ty>(copy) {}
    inline cAutoPtr(cAutoPtr& copy) : std::auto_ptr<_Ty>(copy) {}
    explicit inline cAutoPtr(const cAutoPtr_Ref<_Ty>& pref) : std::auto_ptr<_Ty>(pref._p) {}
    inline cAutoPtr() {}

    inline bool operator !() { return !get(); }
    inline bool operator ==(const _Ty* right) { return get()==right; }
    inline bool operator !=(const _Ty* right) { return get()!=right; }
    inline operator _Ty*() { return get(); }
    inline operator const _Ty*() const { return get(); }
    inline _Ty* operator->() { return get(); }
    inline const _Ty* operator->() const { return get(); }
    inline _Ty& operator*() { return *get(); }
    inline const _Ty& operator*() const { return *get(); }
    inline cAutoPtr& operator=(const cAutoPtr_Ref<_Ty>& right) { reset(right._p); return *this; }
    inline cAutoPtr& operator=(cAutoPtr& right) { std::auto_ptr<_Ty>::operator=(right); return *this; }
  };
}

#endif