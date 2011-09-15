// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_DEF_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_DEF_H__BSS__

#include "bss_call.h"

#define SPAWNME(c) inline virtual c* BSS_FASTCALL Spawn() const { return new c(*this); };
#define SPAWNMEABSTRACT(c) inline virtual c* BSS_FASTCALL Spawn() const=0;

#define DECL_DEF(c,b,s) struct c : b \
  { \
    inline virtual s* BSS_FASTCALL Spawn() const { return new s(*this); } \
    inline virtual c* BSS_FASTCALL Clone() const { return new c(*this); } 

template<class base>
struct BSS_COMPILER_DLLEXPORT cDef {
  inline virtual base* BSS_FASTCALL Spawn() const { return 0; }; //This creates a new instance of whatever class this definition defines
  inline virtual cDef<base>* BSS_FASTCALL Clone() const=0;
  virtual ~cDef<base>() {}
};

#endif