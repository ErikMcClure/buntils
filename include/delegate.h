// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __DELEGATE_H__BSS__
#define __DELEGATE_H__BSS__

#include "bss_defines.h"

namespace bss_util {
  // delegate class using variadic templates in a beautiful, elegant solution I can't use.
  /*template<typename R, typename Args...>
  class delegate
  {
  public:
    inline delegate(void* src, R (BSS_FASTCALL *stub)(void*,Args...)) : _src(src), _stub(stub) {}
    R operator()(Args... args) const { return (*_stub)(_src,args...); }

    template<class T, R (BSS_FASTCALL T::*F)(Args...)>
    inline delegate static From(T* src) { return delegate(src, &stub<T,F>); }

  protected:
    void* _src;
    R (BSS_FASTCALL *_stub)(void*,Args...);

    template <class T, void (BSS_FASTCALL T::*F)(Args...)>
    static R BSS_FASTCALL stub(void* src, Args... args) { return (static_cast<T*>(src)->*F)(args...); }
  }; */
  
  template<typename R=void, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
  class BSS_COMPILER_DLLEXPORT delegate {};

  // delegate class done via macros because Microsoft is full of shit.
  template<typename R>
  class BSS_COMPILER_DLLEXPORT delegate<R,void,void,void,void>
  {
  public:
    inline delegate(void* src, R (BSS_FASTCALL *stub)(void*)) : _src(src), _stub(stub) {}
    R operator()(void) const { return (*_stub)(_src); }

    template<class T, R (BSS_FASTCALL T::*F)(void)>
    inline delegate static From(T* src) { return delegate(src, &stub<T,F>); }

  protected:
    void* _src;
    R (BSS_FASTCALL *_stub)(void*);

    template <class T, void (BSS_FASTCALL T::*F)(void)>
    static R BSS_FASTCALL stub(void* src) { return (static_cast<T*>(src)->*F)(); }
  };

#define BUILD_DELEGATE(T1,T2,T3,T4,TLIST,ARGLIST,ARGS,...) template<typename R,__VA_ARGS__> \
  class BSS_COMPILER_DLLEXPORT delegate<R,T1,T2,T3,T4> \
  { \
  public: \
    inline delegate(void* src, R (BSS_FASTCALL *stub)(void*,TLIST)) : _src(src), _stub(stub) {} \
    R operator()(ARGLIST) const { return (*_stub)(_src,ARGS); } \
    template<class T, R (BSS_FASTCALL T::*F)(TLIST)> \
    inline delegate static From(T* src) { return delegate(src, &stub<T,F>); } \
  protected: \
    void* _src; \
    R (BSS_FASTCALL *_stub)(void*,TLIST); \
    template <class T, void (BSS_FASTCALL T::*F)(TLIST)> \
    static R BSS_FASTCALL stub(void* src, ARGLIST) { return (static_cast<T*>(src)->*F)(ARGS); } \
  }

  BUILD_DELEGATE(T1,void,void,void,CONCAT(T1),CONCAT(T1 t1), CONCAT(t1),typename T1);
  BUILD_DELEGATE(T1,T2,void,void,CONCAT(T1, T2),CONCAT(T1 t1,T2 t2), CONCAT(t1,t2),typename T1,typename T2);
  BUILD_DELEGATE(T1,T2,T3,void,CONCAT(T1, T2, T3),CONCAT(T1 t1,T2 t2,T3 t3), CONCAT(t1,t2,t3),typename T1,typename T2,typename T3);
  BUILD_DELEGATE(T1,T2,T3,T4,CONCAT(T1, T2, T3, T4),CONCAT(T1 t1,T2 t2,T3 t3,T4 t4), CONCAT(t1,t2,t3,t4),typename T1,typename T2,typename T3,typename T4);
}

#endif
