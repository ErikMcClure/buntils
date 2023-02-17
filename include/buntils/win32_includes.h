// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_WIN32_INCLUDES_H__
#define __BUN_WIN32_INCLUDES_H__
#pragma pack(push)
#pragma pack(8)
#define WINVER 0x0501 //_WIN32_WINNT_WINXP   
#define _WIN32_WINNT 0x0501
#define NTDDI_VERSION 0x05010300 //NTDDI_WINXPSP3 
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX // Some compilers enable this by default
#define NOMINMAX
#endif
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#include <windows.h>
#pragma pack(pop)
#endif
