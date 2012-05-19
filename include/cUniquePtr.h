// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_UNIQUEPTR_H__BSS__
#define __C_UNIQUEPTR_H__BSS__

#include <memory>

#ifndef __NO_UNIQUE_MODIFY__
#define UqP_ std::unique_ptr

template<typename T, typename D>
const std::unique_ptr<T,D>& operator<<(std::unique_ptr<T,D>& l, typename std::unique_ptr<T,D>::pointer r) { l.reset(r); return l; }
#endif

namespace bss_util
{
  template<class _Ty> // Trick that uses implicit conversion that allows cUniquePtr below to take _Ty* pointers without confusing itself for them.
  struct cUniquePtr_Ref { inline cUniquePtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  /* Modification of the standard unique_ptr taking advantage of several tricks to make management of the pointer easier */
  template<class _Ty>
  class cUniquePtr : public std::unique_ptr<_Ty>
  {
  public:
    inline cUniquePtr(cUniquePtr&& mov) : std::unique_ptr<_Ty>(std::move(mov)) {}
    explicit inline cUniquePtr(const cUniquePtr_Ref<_Ty>& pref) : std::auto_ptr<_Ty>(pref._p) {}
    inline cUniquePtr() {}

    inline bool operator !() { return !get(); }
    inline bool operator ==(const _Ty* right) { return get()==right; }
    inline bool operator !=(const _Ty* right) { return get()!=right; }
    inline operator _Ty*() { return get(); }
    inline operator const _Ty*() const { return get(); }
    inline _Ty* operator->() { return get(); }
    inline const _Ty* operator->() const { return get(); }
    inline _Ty& operator*() { return *get(); }
    inline const _Ty& operator*() const { return *get(); }
    inline cUniquePtr& operator=(const cUniquePtr_Ref<_Ty>& right) { reset(right._p); return *this; }
    inline cUniquePtr& operator=(cUniquePtr&& right) { std::unique_ptr<_Ty>::operator=(std::move(right)); return *this; }

  private:
    inline cUniquePtr(const cUniquePtr&) {}
    inline cUniquePtr& operator=(const cUniquePtr&) {}
  };

  /* Implementation of an automatic reference tracker for use in conjunction with cRefCounter. Mimics a pointer. */
  //template<class T>
  //class cAutoRef
  //{
  //public:
  //  inline cAutoRef(const cAutoRef& copy) : _p(copy._p) { if(_p!=0) _p->Grab(); }
  //  inline cAutoRef(T* p=0) : _p(p) { if(_p!=0) _p->Grab(); }
  //  inline ~cAutoRef() { if(_p!=0) _p->Drop(); }

  //  inline bool operator !() { return !_p; }
  //  inline bool operator ==(const T* right) { return _p==right; }
  //  inline bool operator !=(const T* right) { return _p!=right; }
  //  inline operator T*() { return _p; }
  //  inline operator const T*() const { return _p; }
  //  inline T* operator->() { return _p; }
  //  inline const T* operator->() const { return _p; }
  //  inline T& operator*() { return *_p; }
  //  inline const T& operator*() const { return *_p; }
  //  inline cAutoRef& operator=(const cAutoRef& right) { return operator=(right._p); }
  //  inline cAutoRef& operator=(T* right) { if(_p!=0) _p->Drop(); _p=right; if(_p!=0) _p->Grab(); return *this; }

  //protected:
  //  T* _p;
  //};
}

#endif