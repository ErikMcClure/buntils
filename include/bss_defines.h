// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DEFINES_H__
#define __BSS_DEFINES_H__

#include "bss_compiler.h"

#define DELETE_SAFE(p) if(p) { delete p; p = 0; }
#define SAFE_DELETE DELETE_SAFE
#define DELETE_SAFE_ARRAY(p) if(p) { delete [] p; p = 0; }
#define DELETE_ARRAY_SAFE DELETE_SAFE_ARRAY
#define SAFE_DELETE_ARRAY DELETE_SAFE_ARRAY
#define CLONEME(c) inline virtual c* Clone() const { return new c(*this); }
#define CLONEMEZERO(c) inline virtual c* Clone() const { return 0; }
#define CLONEMEABSTRACT(c) inline virtual c* Clone() const=0

//sometimes the std versions of these are a bit overboard, so this redefines the MS version, except it will no longer cause conflicts everywhere
#define bssmax(a,b)            (((a) > (b)) ? (a) : (b))
#define bssmin(a,b)            (((a) < (b)) ? (a) : (b))
#define bssclamp(var,min,max) bssmax(min,bssmin(var,max))

/* Use the following defines to force TODO items as warnings in the compile. VC++ currently does not support "messages" despite having them as a category in the error list. */
#define MAKESTRING2(x) #x
#define MAKESTRING(x) MAKESTRING2(x)
#define TODO __FILE__ "(" MAKESTRING(__LINE__) ") : warning (TODO): "
#define NOTICE __FILE__ "(" MAKESTRING(__LINE__) ") : warning (NOTICE): "
#define WIDEN2(x) L ## x 
#define WIDEN(x) WIDEN2(x) 
#ifndef __WFILE__
#define __WFILE__ WIDEN(__FILE__) 
#endif

//Usage: #pragma message(TODO "Clean up code here")

#ifndef RANDFLOATGEN
#define RANDFLOATGEN(min,max) ((max - min) * (double)rand()/(double)RAND_MAX + min)
#endif
#ifndef RANDINTGEN
#define RANDINTGEN(min,max) (min+(rand()%(max-min)))
#endif
#ifndef RANDBOOLGEN
#define RANDBOOLGEN() (rand()>(RAND_MAX>>1))
#endif

//These can be used to define properties
#define CLASS_PROP(typeref, varname, funcname) CLASS_PROP_READONLY(typeref,varname,funcname) CLASS_PROP_WRITEONLY(typeref,varname,funcname)
#define CLASS_PROP_VAL(typeref, varname, funcname) CLASS_PROP_READONLY(typeref,varname,funcname) CLASS_PROP_WRITEONLY_VAL(typeref,varname,funcname)
#define CLASS_PROP_READONLY(typeref, varname, funcname) inline typeref Get##funcname() const { return varname; }
#define CLASS_PROP_WRITEONLY(typeref, varname, funcname) inline void Set##funcname(const typeref m##varname##) { varname=m##varname##; }
#define CLASS_PROP_WRITEONLY_VAL(typeref, varname, funcname) inline void Set##funcname(typeref m##varname##) { varname=m##varname##; }

#define SAFESHIFT(v,s) ((s>0)?(v<<s):(v>>(-s))) //positive number shifts left, negative number shifts right, prevents undefined behavior.
#define VARCLAMP(var,max,min) ((var>max)?(max):((var<min)?(min):(var)))
#define T_GETBIT(type, bit) ((type)(1<<((bit)%(sizeof(type)<<3))))
//#define T_GETBITRANGE(type, low, high) ((type)((((2<<((high)-(low)))-1)<<(low))|(((2<<((high)-(low)))-1)>>(-(low)))|(((2<<((high)-(low)))-1)<<((low)%(sizeof(type)<<3)))))
#define T_GETBITRANGE(type, low, high) ((type)(SAFESHIFT(((2<<((high)-(low)))-1),low)|(((2<<((high)-(low)))-1)<<((low)%(sizeof(type)<<3)))))

//unsigned shortcuts
#ifndef BSS_NOSHORTTYPES
#ifdef  __cplusplus
namespace bss_util {
#endif
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned __int64 uint64;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;
#ifdef  __cplusplus
}
#endif
#endif

#endif