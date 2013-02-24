// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_UNIQUEPTR_H__BSS__
#define __C_UNIQUEPTR_H__BSS__

#include "bss_compiler.h"
#include <memory>

#ifndef __NO_UNIQUE_MODIFY__
#define UqP_ std::unique_ptr

//template<typename T, typename D>
//const std::unique_ptr<T,D>& operator<<(std::unique_ptr<T,D>& l, typename std::unique_ptr<T,D>::pointer r) { l.reset(r); return l; }
#endif

namespace bss_util
{
  template<class _Ty> // Trick that uses implicit conversion that allows cUniquePtr below to take _Ty* pointers without confusing itself for them.
  struct cUniquePtr_Ref { inline cUniquePtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  // Modification of the standard unique_ptr taking advantage of several tricks to make management of the pointer easier
  template<class _Ty>
  class cUniquePtr : public std::unique_ptr<_Ty>
  {
    using std::unique_ptr<_Ty>::get;

  public:
    inline cUniquePtr(cUniquePtr&& mov) : std::unique_ptr<_Ty>(std::move(mov)) {}
    explicit inline cUniquePtr(const cUniquePtr_Ref<_Ty>& pref) : std::unique_ptr<_Ty>(pref._p) {}
    inline cUniquePtr() {}

    inline BSS_FORCEINLINE bool operator !() { return !get(); }
    inline BSS_FORCEINLINE bool operator ==(const _Ty* right) { return get()==right; }
    inline BSS_FORCEINLINE bool operator !=(const _Ty* right) { return get()!=right; }
    inline BSS_FORCEINLINE operator _Ty*() { return get(); }
    inline BSS_FORCEINLINE operator const _Ty*() const { return get(); }
    inline BSS_FORCEINLINE _Ty* operator->() { return get(); }
    inline BSS_FORCEINLINE const _Ty* operator->() const { return get(); }
    inline BSS_FORCEINLINE _Ty& operator*() { return *get(); }
    inline BSS_FORCEINLINE const _Ty& operator*() const { return *get(); }
    inline cUniquePtr& operator=(const cUniquePtr_Ref<_Ty>& right) { reset(right._p); return *this; }
    inline cUniquePtr& operator=(cUniquePtr&& right) { std::unique_ptr<_Ty>::operator=(std::move(right)); return *this; }

  private:
    inline cUniquePtr(const cUniquePtr&) {}
    inline cUniquePtr& operator=(const cUniquePtr&) {}
  };
  
  template<class _Ty> // Trick that uses implicit conversion that allows cUniquePtr below to take _Ty* pointers without confusing itself for them.
  struct cOwnerPtr_Ref { inline cOwnerPtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  // A different kind of pointer handling class that allows copying, but keeps track of whether or not it is actually allowed to delete the given pointer
  template<class _Ty, class _Dy = std::default_delete<_Ty>>
  class cOwnerPtr : private _Dy
  {
  public:
    inline cOwnerPtr(const cOwnerPtr& copy) : _p(copy._p), _owner(false) { }
    template<class T,class D>
    inline explicit cOwnerPtr(cOwnerPtr<T,D>& copy) : _p(static_cast<_Ty*>(copy)), _owner(false) { }
    inline cOwnerPtr(cOwnerPtr&& mov) : _p(mov._p), _owner(mov._owner) { mov._owner=false; }
    inline explicit cOwnerPtr(const cOwnerPtr_Ref<_Ty>& p) : _p(p._p), _owner(p._p!=0) { }
    inline explicit cOwnerPtr(_Ty& ref) : _p(&ref), _owner(false) { }
    inline cOwnerPtr() : _p(0), _owner(false) { }
    inline ~cOwnerPtr() { _disown(); }

    inline BSS_FORCEINLINE bool operator !() { return !_p; }
    inline BSS_FORCEINLINE bool operator ==(const _Ty* right) { return _p==right; }
    inline BSS_FORCEINLINE bool operator !=(const _Ty* right) { return _p!=right; }
    inline BSS_FORCEINLINE bool operator ==(const cOwnerPtr& right) { return _p==right._p; }
    inline BSS_FORCEINLINE bool operator !=(const cOwnerPtr& right) { return _p!=right._p; }
    inline BSS_FORCEINLINE operator _Ty*() { return _p; }
    inline BSS_FORCEINLINE operator const _Ty*() const { return _p; }
    inline BSS_FORCEINLINE _Ty* operator->() { return _p; }
    inline BSS_FORCEINLINE const _Ty* operator->() const { return _p; }
    inline BSS_FORCEINLINE _Ty& operator*() { return *_p; }
    inline BSS_FORCEINLINE const _Ty& operator*() const { return *_p; }
    inline cOwnerPtr& operator=(const cOwnerPtr_Ref<_Ty>& right) { _disown(); _p=right._p; _owner=true; return *this; }
    inline cOwnerPtr& operator=(const cOwnerPtr& right) { _disown(); _p=right._p; _owner=false; return *this; }
    inline cOwnerPtr& operator=(_Ty& ref) { _disown(); _p=&ref; _owner=false; return *this; }
    inline cOwnerPtr& operator=(cOwnerPtr&& right) { _disown(); _p=right._p; _owner=true; right._owner=false; return *this; }

  protected:
    inline BSS_FORCEINLINE void _disown() const { if(_owner) { _Dy::operator()(_p); } }

    _Ty* _p;
    bool _owner;
  };
  // Implementation of an automatic reference tracker for use in conjunction with cRefCounter. Mimics a pointer.
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
