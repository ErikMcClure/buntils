// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SINGLETON_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_SINGLETON_H__BSS__

#include "bss_compiler.h"

namespace bss_util {
  template<class T>
  class BSS_COMPILER_DLLEXPORT cSingleton //exported to make VC++ shut up
  {
    inline cSingleton(const cSingleton&) BSS_DELETEFUNC
    inline cSingleton(cSingleton&&) BSS_DELETEFUNC
    inline cSingleton& operator=(const cSingleton&) BSS_DELETEFUNCOP
    inline cSingleton& operator=(cSingleton&&) BSS_DELETEFUNCOP
  public:
    inline cSingleton(T* ptr) { _ptr = ptr; _instance = _ptr; }
    inline ~cSingleton() { if(_instance == _ptr) _instance = 0; }
    
    inline static T* Instance() { return _instance; }
    inline static T& InstRef() { return *_instance; }

  protected:
    static T* _instance;

  private:
    T* _ptr;
  };

  template<class T>
  T* cSingleton<T>::_instance=0;
}

#endif
