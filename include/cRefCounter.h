// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_REFCOUNTER_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_REFCOUNTER_H__BSS__

#include "bss_compiler.h"

namespace bss_util {  
  // A reference counter class that is entirely inline
  class BSS_COMPILER_DLLEXPORT cRefCounter
  {
  public:
    // Increments and returns the reference counter
    BSS_FORCEINLINE int Grab() noexcept { return ++_refs; }
    // Decrements the reference counter and calls delete this; if it is equal to or less then 0
    BSS_FORCEINLINE int Drop() {
      assert(_refs > 0);
      if(--_refs > 0)
        return _refs;
      DestroyThis();
      return 0;
    }

  protected:
    // Constructor - Reference is set to 0 because you may or may not have a persistent reference to this, or something else will try to grab it or whatever
    inline cRefCounter() { _refs = 0; }
    inline cRefCounter(const cRefCounter& copy) { _refs = 0; }
    // Destructor - Does nothing, but because it is virtual, ensures that all superclasses get destroyed as well
    virtual ~cRefCounter() { }
    // Destroys this object - made a seperate virtual function so it is overridable to ensure it is deleted in the proper DLL
    virtual void DestroyThis() { delete this; }

    inline cRefCounter& operator=(const cRefCounter& right) { return *this; } // This does not actually change the reference count

    int _refs; //holds the number of references held for this object
  };

  // Implementation of an automatic reference tracker for use in conjunction with cRefCounter. Mimics a pointer.
  template<class T>
  class BSS_COMPILER_DLLEXPORT ref_ptr
  {
  public:
    inline ref_ptr(const ref_ptr& copy) : _p(copy._p) { if(_p != 0) _p->Grab(); }
    inline ref_ptr(ref_ptr&& mov) : _p(mov._p) { mov._p = 0; }
    inline ref_ptr(T* p = 0) : _p(p) { if(_p != 0) _p->Grab(); }
    inline ~ref_ptr() { if(_p != 0) _p->Drop(); }

    inline bool operator !() const noexcept { return !_p; }
    inline bool operator ==(const T* right) const noexcept { return _p == right; }
    inline bool operator !=(const T* right) const noexcept { return _p != right; }
    inline operator T*() noexcept { return _p; }
    inline operator const T*() const noexcept { return _p; }
    inline T* operator->() noexcept { return _p; }
    inline const T* operator->() const noexcept { return _p; }
    inline T& operator*() noexcept { return *_p; }
    inline const T& operator*() const noexcept { return *_p; }
    inline ref_ptr& operator=(const ref_ptr& right) { return operator=(right._p); }
    inline ref_ptr& operator=(ref_ptr&& right) { if(_p != 0) _p->Drop(); _p = right._p; right._p = 0; return *this; }
    inline ref_ptr& operator=(T* right) { if(_p != 0) _p->Drop(); _p = right; if(_p != 0) _p->Grab(); return *this; }

  protected:
    T* _p;
  };
}

#endif
