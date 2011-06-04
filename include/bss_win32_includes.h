// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_WIN32_INCLUDES_H__
#define __BSS_WIN32_INCLUDES_H__
#pragma pack(push)
#pragma pack(8)
#define WINVER 0x0501 //_WIN32_WINNT_WINXP   
#define _WIN32_WINNT 0x0501 
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma pack(pop)
#endif