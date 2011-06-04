// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DLLDEF_H__
#define __BSS_DLLDEF_H__

#include "bss_call.h"

#ifndef BSS_STATIC_LIB
#ifdef BSS_UTIL_EXPORTS
#define BSS_DLLEXPORT __declspec(dllexport)
#else
#define BSS_DLLEXPORT __declspec(dllimport)
#endif
#else
#define BSS_DLLEXPORT
#endif

#endif