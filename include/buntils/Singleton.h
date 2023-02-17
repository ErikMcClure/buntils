// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __SINGLETON_H__BUN__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __SINGLETON_H__BUN__

#include "compiler.h"

namespace bun {
  template<class T>
  class BUN_COMPILER_DLLEXPORT Singleton //exported to make VC++ shut up
  {
    inline Singleton(const Singleton&) = delete;
    inline Singleton& operator=(const Singleton&) = delete;
  public:
    inline Singleton(Singleton&& mov) { if(_instance == static_cast<T*>(&mov)) _instance = static_cast<T*>(this); }
    inline Singleton() { _instance = static_cast<T*>(this); }
    inline ~Singleton() { if(_instance == static_cast<T*>(this)) _instance = 0; }

    //inline static T* Instance() { return _instance; } // You have to provide this so it gets called from the correct DLL

    inline Singleton& operator=(Singleton&& mov) { if(_instance == static_cast<T*>(&mov)) _instance = static_cast<T*>(this); return *this; }

  protected:
    static T* _instance;
  };
}

#endif
