// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __SINGLETON_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __SINGLETON_H__BSS__

#include "compiler.h"

namespace bss {
  template<class T>
  class BSS_COMPILER_DLLEXPORT Singleton //exported to make VC++ shut up
  {
    inline Singleton(const Singleton&) BSS_DELETEFUNC
      inline Singleton(Singleton&&) BSS_DELETEFUNC
      inline Singleton& operator=(const Singleton&) BSS_DELETEFUNCOP
      inline Singleton& operator=(Singleton&&) BSS_DELETEFUNCOP
  public:
    inline Singleton(T* ptr) { _ptr = ptr; _instance = _ptr; }
    inline ~Singleton() { if(_instance == _ptr) _instance = 0; }

    inline static T* Instance() { return _instance; }
    inline static T& InstRef() { return *_instance; }

  protected:
    static T* _instance;

  private:
    T* _ptr;
  };

  template<class T>
  T* Singleton<T>::_instance = 0;
}

#endif
