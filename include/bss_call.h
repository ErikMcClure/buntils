// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_CALL_H__
#define __BSS_CALL_H__

#include "bss_compiler.h"

#ifndef BSS_NO_FASTCALL
#define BSS_FASTCALL BSS_COMPILER_FASTCALL
#else
#define BSS_FASTCALL BSS_COMPILER_STDCALL
#endif

#endif