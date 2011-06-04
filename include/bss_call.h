// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_CALL_H__
#define __BSS_CALL_H__

#ifndef BSS_NO_FASTCALL
#define BSS_FASTCALL __fastcall
#else
#define BSS_FASTCALL __stdcall
#endif

#endif