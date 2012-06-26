// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_REFCOUNTER_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_REFCOUNTER_H__BSS__

#define DELETE_REF(p) if(p) { p->Drop(); p = 0; }

#include "bss_compiler.h"

namespace bss_util {  
  /* A reference counter class that is entirely inline */
  class BSS_COMPILER_DLLEXPORT cRefCounter
  {
  public:
    /* Constructor - Reference is set to 0 because you may or may not have a persistent reference to this, or something else will try to grab it or whatever */
    inline cRefCounter() { _refs = 0; }
    inline cRefCounter(const cRefCounter& copy) { _refs = 0; }
    /* Destructor - Does nothing, but because it is virtual, ensures that all superclasses get destroyed as well */
    virtual ~cRefCounter() { }
    /* Increments and returns the reference counter */
    inline BSS_FORCEINLINE unsigned int Grab() { return ++_refs; } const
    /* Decrements the reference counter and calls delete this; if it is equal to or less then 0 */
    inline BSS_FORCEINLINE void Drop() { if(--_refs <= 0) DestroyThis(); }
    /* Destroys this object - made a seperate virtual function so it is overridable to ensure it is deleted in the proper DLL */
    virtual void DestroyThis() { delete this; }

  private:
    int _refs; //holds the number of references held for this object
  };

  /* This is an autoptr with copy semantics for a reference counter */
  /*template<class T>
  class BSS_COMPILER_DLLEXPORT cRefPointer
  {
  public:
    inline cRefPointer(const cRefPointer& ptr) { _assign(ptr); }
    inline cRefPointer(T* ptr) { _assign(ptr); }
    inline ~cRefPointer() { _drop(); }
    inline void ReleasePtr() { _drop(); }

    inline cRefPointer& operator=(T* right) { _drop(); _assign(right) }
    inline cRefPointer& operator=(const cRefPointer& right) { _drop(); _assign(right); }
    inline T& operator*() const { return *_ptr; }
    inline T* operator->() const { return _ptr; }
    //operator T*() const { return _ptr; }

  protected:
    inline void _assign(T* ptr) { _ptr = ptr; if(_ptr) ((cRefCounter*)_ptr)->Grab(); }
    inline void _drop() { if(_ptr) ((cRefCounter*)_ptr)->Drop(); _ptr=0; }

    T* _ptr;
  };*/
}

#endif