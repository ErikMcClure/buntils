// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SMART_PTR_H__BSS__
#define __C_SMART_PTR_H__BSS__

#include "bss_compiler.h"
#include <memory> // for std::default_delete<_Ty>

namespace bss_util
{  
  template<class _Ty> // Trick that uses implicit conversion that allows cUniquePtr below to take _Ty* pointers without confusing itself for them.
  struct BSS_COMPILER_DLLEXPORT cOwnerPtr_Ref { inline cOwnerPtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  // A different kind of pointer handling class that allows copying, but keeps track of whether or not it is actually allowed to delete the given pointer
  template<class _Ty, class _Dy = std::default_delete<_Ty>>
  class BSS_COMPILER_DLLEXPORT cOwnerPtr : private _Dy
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

    BSS_FORCEINLINE bool operator !() { return !_p; }
    BSS_FORCEINLINE bool operator ==(const _Ty* right) { return _p==right; }
    BSS_FORCEINLINE bool operator !=(const _Ty* right) { return _p!=right; }
    BSS_FORCEINLINE bool operator ==(const cOwnerPtr& right) { return _p==right._p; }
    BSS_FORCEINLINE bool operator !=(const cOwnerPtr& right) { return _p!=right._p; }
    BSS_FORCEINLINE operator _Ty*() { return _p; }
    BSS_FORCEINLINE operator const _Ty*() const { return _p; }
    BSS_FORCEINLINE _Ty* operator->() { return _p; }
    BSS_FORCEINLINE const _Ty* operator->() const { return _p; }
    BSS_FORCEINLINE _Ty& operator*() { return *_p; }
    BSS_FORCEINLINE const _Ty& operator*() const { return *_p; }
    inline cOwnerPtr& operator=(const cOwnerPtr_Ref<_Ty>& right) { _disown(); _p=right._p; _owner=true; return *this; }
    inline cOwnerPtr& operator=(const cOwnerPtr& right) { _disown(); _p=right._p; _owner=false; return *this; }
    inline cOwnerPtr& operator=(_Ty& ref) { _disown(); _p=&ref; _owner=false; return *this; }
    inline cOwnerPtr& operator=(cOwnerPtr&& right) { _disown(); _p=right._p; _owner=true; right._owner=false; return *this; }

  protected:
    BSS_FORCEINLINE void _disown() const { if(_owner) { _Dy::operator()(_p); } }

    _Ty* _p;
    bool _owner;
  };
}

#endif


//template<typename T, typename D>
//const std::unique_ptr<T,D>& operator<<(std::unique_ptr<T,D>& l, typename std::unique_ptr<T,D>::pointer r) { l.reset(r); return l; }

/*

  template<class _Ty> // Trick that uses implicit conversion that allows cUniquePtr below to take _Ty* pointers without confusing itself for them.
  struct cUniquePtr_Ref { inline cUniquePtr_Ref(_Ty* p) : _p(p) {} _Ty* _p; };

  // Modification of the standard unique_ptr taking advantage of several tricks to make management of the pointer easier
  template<class _Ty>
  class cUniquePtr : public std::unique_ptr<_Ty>
  {
  public:
    using std::unique_ptr<_Ty>::get;

    inline cUniquePtr(cUniquePtr&& mov) : std::unique_ptr<_Ty>(std::move(mov)) {}
    explicit inline cUniquePtr(const cUniquePtr_Ref<_Ty>& pref) : std::unique_ptr<_Ty>(pref._p) {}
    inline cUniquePtr() {}

    BSS_FORCEINLINE bool operator !() { return !get(); }
    BSS_FORCEINLINE bool operator ==(const _Ty* right) { return get()==right; }
    BSS_FORCEINLINE bool operator !=(const _Ty* right) { return get()!=right; }
    BSS_FORCEINLINE operator _Ty*() { return get(); }
    BSS_FORCEINLINE operator const _Ty*() const { return get(); }
    BSS_FORCEINLINE _Ty* operator->() { return get(); }
    BSS_FORCEINLINE const _Ty* operator->() const { return get(); }
    BSS_FORCEINLINE _Ty& operator*() { return *get(); }
    BSS_FORCEINLINE const _Ty& operator*() const { return *get(); }
    inline cUniquePtr& operator=(const cUniquePtr_Ref<_Ty>& right) { reset(right._p); return *this; }
    inline cUniquePtr& operator=(cUniquePtr&& right) { std::unique_ptr<_Ty>::operator=(std::move(right)); return *this; }

  private:
    inline cUniquePtr(const cUniquePtr&) {}
    inline cUniquePtr& operator=(const cUniquePtr&) {}
  };
  */