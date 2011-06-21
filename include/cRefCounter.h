// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_REFCOUNTER_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_REFCOUNTER_H__BSS__

#define DELETE_REF(p) if(p) { p->Drop(); p = 0; }

namespace bss_util {  
  /* A reference counter class that is entirely inline */
  class __declspec(dllexport) cRefCounter
  {
  public:
    /* Constructor - Reference is set to 0 because you may or may not have a persistent reference to this, or something else will try to grab it or whatever */
    cRefCounter() { _refs = 0; }
    cRefCounter(const cRefCounter& copy) { _refs = 0; }
    /* Destructor - Does nothing, but because it is virtual, ensures that all superclasses get destroyed as well */
    virtual ~cRefCounter() { }
    /* Increments and returns the reference counter */
    inline unsigned int Grab() { return ++_refs; } const
    /* Decrements the reference counter and calls delete this; if it is equal to or less then 0 */
    inline void Drop() { if(--_refs <= 0) DestroyThis(); }
    /* Destroys this object - made a seperate virtual function so it is overridable to ensure it is deleted in the proper DLL */
    virtual void DestroyThis() { delete this; }

  private:
    int _refs; //holds the number of references held for this object
  };

  /* This is an autoptr with copy semantics for a reference counter */
  template<class T>
  class __declspec(dllexport) cRefPointer //only exported to make VC++ shut up with that stupid warning
  {
  public:
    cRefPointer(const cRefPointer& ptr) { _assign(ptr); }
    cRefPointer(T* ptr) { _assign(ptr); }
    ~cRefPointer() { _drop(); }
    void ReleasePtr() { _drop(); }

    cRefPointer& operator=(T* right) { _drop(); _assign(right) }
    cRefPointer& operator=(const cRefPointer& right) { _drop(); _assign(right); }
    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }
    //operator T*() const { return _ptr; }

  protected:
    void _assign(T* ptr) { _ptr = ptr; if(_ptr) ((cRefCounter*)_ptr)->Grab(); }
    void _drop() { if(_ptr) ((cRefCounter*)_ptr)->Drop(); _ptr=0; }

    T* _ptr;
  };
}

#endif