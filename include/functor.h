// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __FUNCTOR_H__BSS__
#define __FUNCTOR_H__BSS__

namespace bss_util {
  template<typename R, typename T1=void,typename T2=void,typename T3=void>
  struct BSS_COMPILER_DLLEXPORT Functor
  {
    inline virtual ~Functor() {}
    inline virtual Functor* BSS_FASTCALL Clone() const=0;
    virtual R BSS_FASTCALL Call(T1,T2,T3) const=0;
    R operator()(T1 t1, T2 t2, T3 t3) const { return Call(t1,t2,t3); }
  };

  template<typename R, typename T1, typename T2>
  struct BSS_COMPILER_DLLEXPORT Functor<R,T1,T2,void>
  {
    inline virtual ~Functor() {}
    inline virtual Functor* BSS_FASTCALL Clone() const=0;
    virtual R BSS_FASTCALL Call(T1,T2) const=0;
    R operator()(T1 t1, T2 t2) const { return Call(t1,t2); }
  };

  template<typename R, typename T1>
  struct BSS_COMPILER_DLLEXPORT Functor<R,T1,void,void>
  {
    inline virtual ~Functor() {}
    inline virtual Functor* BSS_FASTCALL Clone() const=0;
    virtual R BSS_FASTCALL Call(T1) const=0;
    R operator()(T1 t1) const { return Call(t1); }
  };

  template<typename R>
  struct BSS_COMPILER_DLLEXPORT Functor<R,void,void,void>
  {
    inline virtual ~Functor() {}
    inline virtual Functor* BSS_FASTCALL Clone() const=0;
    virtual R BSS_FASTCALL Call(void) const=0;
    R operator()(void) const { return Call(); }
  };

  template<typename T, typename R, typename T1=void,typename T2=void,typename T3=void>
  struct BSS_COMPILER_DLLEXPORT FuncInst : Functor<R,T1,T2,T3>
  {
    FuncInst(T* source, R (BSS_FASTCALL T::*funcptr)(T1,T2,T3)) : _funcptr(funcptr), _source(source) {}
    virtual R BSS_FASTCALL Call(T1 t1, T2 t2, T3 t3) const { return (_source->*_funcptr)(t1,t2,t3); }
    inline virtual FuncInst* BSS_FASTCALL Clone() const { return new FuncInst(*this); }

  protected:
    T* _source;
    R (BSS_FASTCALL T::*_funcptr)(T1,T2,T3);
  };

  template<typename T, typename R, typename T1,typename T2>
  struct BSS_COMPILER_DLLEXPORT FuncInst<T,R,T1,T2,void> : Functor<R,T1,T2,void>
  {
    FuncInst(T* source, R (BSS_FASTCALL T::*funcptr)(T1,T2)) : _funcptr(funcptr), _source(source) {}
    virtual R BSS_FASTCALL Call(T1 t1, T2 t2) const { return (_source->*_funcptr)(t1,t2); }
    inline virtual FuncInst* BSS_FASTCALL Clone() const { return new FuncInst(*this); }

  protected:
    T* _source;
    R (BSS_FASTCALL T::*_funcptr)(T1,T2);
  };

  template<typename T, typename R, typename T1>
  struct BSS_COMPILER_DLLEXPORT FuncInst<T,R,T1,void,void> : Functor<R,T1,void,void>
  {
    FuncInst(T* source, R (BSS_FASTCALL T::*funcptr)(T1)) : _funcptr(funcptr), _source(source) {}
    virtual R BSS_FASTCALL Call(T1 t1) const { return (_source->*_funcptr)(t1); }
    inline virtual FuncInst* BSS_FASTCALL Clone() const { return new FuncInst(*this); }

  protected:
    T* _source;
    R (BSS_FASTCALL T::*_funcptr)(T1);
  };

  template<typename T, typename R>
  struct BSS_COMPILER_DLLEXPORT FuncInst<T,R,void,void,void> : Functor<R,void,void,void>
  {
    FuncInst(T* source, R (BSS_FASTCALL T::*funcptr)(void)) : _funcptr(funcptr), _source(source) {}
    virtual R BSS_FASTCALL Call(void) const { return (_source->*_funcptr)(); }
    inline virtual FuncInst* BSS_FASTCALL Clone() const { return new FuncInst(*this); }

  protected:
    T* _source;
    R (BSS_FASTCALL T::*_funcptr)();
  };
}
#endif