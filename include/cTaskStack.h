// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_TASKSTACK_H__
#define __C_TASKSTACK_H__

#include "cKhash.h"
#include "cMutex.h"
#include <stdarg.h>

namespace bss_util {
  template<class T>
  struct __declspec(dllexport) FUNC_DEF_BASE
  {
    FUNC_DEF_BASE(unsigned int msize) : size(msize) {}
    const unsigned int size;
    virtual void BSS_FASTCALL Execute(T* ptr, void* memptr) const=0;
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const=0; //Given a variable argument list, fills the given memory with the arguments

    inline virtual FUNC_DEF_BASE* Clone() const=0;
  };

  /* This is a thread-safe task stack that accumulates function calls for a class */
  template<class T>
  class __declspec(dllexport) cTaskStack : public cLockable
  {
  public:
    cTaskStack(unsigned int stacksize=128, bool unlock=true) : _memory((unsigned char*)malloc(stacksize)), _memsize(stacksize), cLockable(4),_curindex(0),_unlock(unlock) {}
    ~cTaskStack()
    {
      _syncobj.Lock();
      _funchash.ResetWalk();
      khiter_t iter; 
      const FUNC_DEF_BASE<T>* ptr;
      while((iter=_funchash.GetNext())!=_funchash.End())
        if(ptr=_funchash.Get(iter))
          delete ptr;

      _funchash.Clear();
      free(_memory);
      _syncobj.Unlock();
    }

    bool AddTask(unsigned int FuncID, ...)
    {
      LOCKME();
      const FUNC_DEF_BASE<T>* funcdef = _funchash.GetKey(FuncID);
      if(!funcdef) return false;
      unsigned int nextindex = _curindex + sizeof(unsigned int)+funcdef->size;
      while(nextindex>_memsize)
        if(!Expand(_memsize<<1)) //if this is false we ran out of memory, so we run screaming out of the room and hope that someone else can clean up the mess.
          return false; //AAAAAAAAAAAAAAAAAIIIIIIIIIIIIIIIIIIIIIIIIIIEEEEEEEEEEEEEEEEEEEEEEE
      va_list vl;
      va_start(vl, FuncID);
      funcdef->Fill(vl, _memory+_curindex + sizeof(unsigned int));
      va_end(vl);
      *((unsigned int*)(_memory+_curindex)) = FuncID;
      _curindex=nextindex;
      return true;
    }
    /* this allows you to pass in the arguments as a struct and is size-safe, but not necessarily typesafe */
    template<class Args>
    bool BSS_FASTCALL AddTaskArg(unsigned int FuncID, const Args& funcarg)
    {
      LOCKME();
      const FUNC_DEF_BASE<T>* funcdef = _funchash.GetKey(FuncID);
      if(!funcdef) return false;
      assert(funcdef->size != sizeof(Args));
      //if(!funcdef || !funcdef->Compare(funcarg)) return false;
      unsigned int nextindex = _curindex + sizeof(unsigned int)+funcdef->size;
      while(nextindex>_memsize)
        if(!Expand(_memsize<<1))
          return false;
      *((Args*)(_memory+_curindex + sizeof(unsigned int))) = funcarg;
      *((unsigned int*)(_memory+_curindex)) = FuncID;
      _curindex=nextindex;
      return true;
    }
    bool BSS_FASTCALL RegisterFunction(const FUNC_DEF_BASE<T>& def, unsigned int FuncID)
    {
      LOCKME();
      return _funchash.Insert(FuncID,def.Clone());
    }
    bool BSS_FASTCALL UnRegisterFunction(unsigned int FuncID)
    {
      LOCKME();
      if(!!_curindex) return false; //You can only do this if you have already cleared the stack
      const FUNC_DEF_BASE<T>* funcdef = _funchash.Remove(FuncID);
      bool retval = funcdef!=0;
      if(retval)
        delete funcdef;
      return retval;
    }
    void BSS_FASTCALL EvaluateStack(T* ptr)
    {
      unsigned int step=0;
      unsigned int id;
      const FUNC_DEF_BASE<T>* funchold;
      _syncobj.Lock();
      if(_unlock)
      {
        while(step<_curindex)
        { 
          id=*(unsigned int*)(&_memory[step]); //we can't take this out of the mutex lock or we run the risk of going over the end, which is a very bad thing
          funchold = _funchash.GetKey(id);
          _syncobj.Unlock();
          assert(funchold!=0);
          funchold->Execute(ptr,&_memory[step+=sizeof(unsigned int)]);
          step+=funchold->size;
          _syncobj.Lock();
        }
      }
      else
      {
        while(step<_curindex)
        { 
          id=*(unsigned int*)(&_memory[step]);
          funchold = _funchash.GetKey(id);
          assert(funchold!=0);
          funchold->Execute(ptr,&_memory[step+=sizeof(unsigned int)]);
          step+=funchold->size;
        }
      }
      _curindex=0;
      _syncobj.Unlock();
    }
    void Dump()
    {
      LOCKME();
      _curindex=0;
    }
    /* If true, unlocks the mutex while it executes each function. Otherwise the mutex is locked for the duration of the stack evaluation */
    void SetUnlock(bool unlock)
    {
      LOCKME();
      _unlock=unlock;
    }
    bool IsEmpty() const { LOCKME(); return !_curindex; }

  protected:
    bool Expand(unsigned int nsize)
    {
      unsigned char* oldmem = _memory;
      _memory = (unsigned char*)malloc(nsize);
      if(!_memory) { _memsize=0; return false; }//OH SHIT
      memcpy(_memory,oldmem,_memsize);
      free(oldmem);
      _memsize=nsize;
      return true;
    }

    cKhash_Int<const FUNC_DEF_BASE<T>*> _funchash;
    unsigned char* _memory;
    unsigned int _curindex;
    unsigned int _memsize;
    bool _unlock;
  };

  template<class T> //if you try to put , typedef calltype=__stdcall it actually crashes the compiler, which I find amusing.
  struct FUNC_DEF0 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF0(const FUNC_DEF0& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(0) {}
    FUNC_DEF0(void (T::*funcptr)(void)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(0) {}
    virtual void BSS_FASTCALL Execute(T* ptr, void* memptr) const
    {
      (ptr->*_funcptr)();
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
    } 
    inline virtual FUNC_DEF0* Clone() const { return new FUNC_DEF0(*this); }

  protected: 
    void (T::*_funcptr)(void); 
  };

  template<class T, class Arg1> 
  struct FUNC_ARGS1
  {
    Arg1 _arg1;
  };

  template<class T, class Arg1> 
  struct FUNC_DEF1 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF1(const FUNC_DEF1& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS1<T,Arg1>)) {}
    FUNC_DEF1(void (T::*funcptr)(Arg1)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS1<T,Arg1>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS1<T,Arg1>* ptr=(FUNC_ARGS1<T,Arg1>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS1<T,Arg1>* ptr=(FUNC_ARGS1<T,Arg1>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
    } 
    inline virtual FUNC_DEF1* Clone() const { return new FUNC_DEF1(*this); }

  protected: 
    void (T::*_funcptr)(Arg1); 
  };

  template<class T, class Arg1> 
  struct FUNC_DEF1_fastcall : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF1_fastcall(const FUNC_DEF1_fastcall& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS1<T,Arg1>)) {}
    FUNC_DEF1_fastcall(void (BSS_FASTCALL T::*funcptr)(Arg1)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS1<T,Arg1>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS1<T,Arg1>* ptr=(FUNC_ARGS1<T,Arg1>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS1<T,Arg1>* ptr=(FUNC_ARGS1<T,Arg1>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
    } 
    inline virtual FUNC_DEF1_fastcall* Clone() const { return new FUNC_DEF1_fastcall(*this); }

  protected: 
    void (BSS_FASTCALL T::*_funcptr)(Arg1); 
  };

  template<class T, class Arg1, class Arg2> 
  struct FUNC_ARGS2
  {
    Arg1 _arg1;
    Arg2 _arg2;
  };

  template<class T, class Arg1, class Arg2> 
  struct FUNC_DEF2 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF2(const FUNC_DEF2& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS2<T,Arg1,Arg2>)) {}
    FUNC_DEF2(void (T::*funcptr)(Arg1,Arg2)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS2<T,Arg1,Arg2>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS2<T,Arg1,Arg2>* ptr=(FUNC_ARGS2<T,Arg1,Arg2>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS2<T,Arg1,Arg2>* ptr=(FUNC_ARGS2<T,Arg1,Arg2>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
    } 
    inline virtual FUNC_DEF2* Clone() const { return new FUNC_DEF2(*this); }

  protected: 
    void (T::*_funcptr)(Arg1,Arg2); 
  };

  template<class T, class Arg1, class Arg2> 
  struct FUNC_DEF2_fastcall : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF2_fastcall(const FUNC_DEF2_fastcall& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS2<T,Arg1,Arg2>)) {}
    FUNC_DEF2_fastcall(void (BSS_FASTCALL T::*funcptr)(Arg1,Arg2)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS2<T,Arg1,Arg2>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS2<T,Arg1,Arg2>* ptr=(FUNC_ARGS2<T,Arg1,Arg2>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS2<T,Arg1,Arg2>* ptr=(FUNC_ARGS2<T,Arg1,Arg2>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
    } 
    inline virtual FUNC_DEF2_fastcall* Clone() const { return new FUNC_DEF2_fastcall(*this); }

  protected: 
    void (BSS_FASTCALL T::*_funcptr)(Arg1,Arg2); 
  };

  template<class T, class Arg1, class Arg2, class Arg3> 
  struct FUNC_ARGS3
  {
    Arg1 _arg1;
    Arg2 _arg2;
    Arg3 _arg3;
  };

  template<class T, class Arg1, class Arg2, class Arg3> 
  struct FUNC_DEF3 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF3(const FUNC_DEF3& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3>)) {}
    FUNC_DEF3(void (T::*funcptr)(Arg1,Arg2,Arg3)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2, ptr->_arg3);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
      ptr->_arg3=va_arg(vl,Arg3);
    } 
    inline virtual FUNC_DEF3* Clone() const { return new FUNC_DEF3(*this); }

  protected: 
    void (T::*_funcptr)(Arg1,Arg2,Arg3); 
  };

  template<class T, class Arg1, class Arg2, class Arg3> 
  struct FUNC_DEF3_fastcall : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF3_fastcall(const FUNC_DEF3_fastcall& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3>)) {}
    FUNC_DEF3_fastcall(void (BSS_FASTCALL T::*funcptr)(Arg1,Arg2,Arg3)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2, ptr->_arg3);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
      ptr->_arg3=va_arg(vl,Arg3);
    } 
    inline virtual FUNC_DEF3_fastcall* Clone() const { return new FUNC_DEF3_fastcall(*this); }

  protected: 
    void (BSS_FASTCALL T::*_funcptr)(Arg1,Arg2,Arg3); 
  };

  template<class T, class Arg1, class Arg2, class Arg3, class Arg4> 
  struct FUNC_ARGS4
  {
    Arg1 _arg1;
    Arg2 _arg2;
    Arg3 _arg3;
    Arg4 _arg4;
  };

  template<class T, class Arg1, class Arg2, class Arg3, class Arg4> 
  struct FUNC_DEF4 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF4(const FUNC_DEF4& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>)) {}
    FUNC_DEF4(void (T::*funcptr)(Arg1,Arg2,Arg3,Arg4)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2, ptr->_arg3, ptr->_arg4);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
      ptr->_arg3=va_arg(vl,Arg3);
      ptr->_arg4=va_arg(vl,Arg4);
    } 
    inline virtual FUNC_DEF4* Clone() const { return new FUNC_DEF4(*this); }

  protected: 
    void (T::*_funcptr)(Arg1,Arg2,Arg3,Arg4); 
  };

  template<class T, class Arg1, class Arg2, class Arg3, class Arg4> 
  struct FUNC_DEF4_fastcall : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF4_fastcall(const FUNC_DEF4_fastcall& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>)) {}
    FUNC_DEF4_fastcall(void (BSS_FASTCALL T::*funcptr)(Arg1,Arg2,Arg3,Arg4)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2, ptr->_arg3, ptr->_arg4);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
      ptr->_arg3=va_arg(vl,Arg3);
      ptr->_arg4=va_arg(vl,Arg4);
    } 
    inline virtual FUNC_DEF4_fastcall* Clone() const { return new FUNC_DEF4_fastcall(*this); }

  protected: 
    void (BSS_FASTCALL T::*_funcptr)(Arg1,Arg2,Arg3,Arg4); 
  };

  template<class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5> 
  struct FUNC_ARGS5
  {
    Arg1 _arg1;
    Arg2 _arg2;
    Arg3 _arg3;
    Arg4 _arg4;
    Arg5 _arg5;
  };

  template<class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5> 
  struct FUNC_DEF5 : public FUNC_DEF_BASE<T> 
  { 
    FUNC_DEF5(const FUNC_DEF5& copy) : _funcptr(copy._funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>)) {}
    FUNC_DEF5(void (T::*funcptr)(Arg1,Arg2,Arg3,Arg4,Arg5)) : _funcptr(funcptr), FUNC_DEF_BASE<T>(sizeof(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>)) {}
    virtual void BSS_FASTCALL Execute(T* cptr, void* memptr) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>*)memptr;
      (cptr->*_funcptr)(ptr->_arg1, ptr->_arg2, ptr->_arg3, ptr->_arg4, ptr->_arg5);
    };
    virtual void BSS_FASTCALL Fill(va_list& vl, void* mem) const
    {
      FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>* ptr=(FUNC_ARGS3<T,Arg1,Arg2,Arg3,Arg4,Arg5>*)mem;
      ptr->_arg1=va_arg(vl,Arg1);
      ptr->_arg2=va_arg(vl,Arg2);
      ptr->_arg3=va_arg(vl,Arg3);
      ptr->_arg4=va_arg(vl,Arg4);
      ptr->_arg5=va_arg(vl,Arg5);
    } 
    inline virtual FUNC_DEF5* Clone() const { return new FUNC_DEF5(*this); }

  protected: 
    void (T::*_funcptr)(Arg1,Arg2,Arg3,Arg4,Arg5); 
  };

}

#endif