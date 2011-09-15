// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COMPILER_H__
#define __BSS_COMPILER_H__

// Compiler detection and macro generation
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC) // Intel C++ compiler
#define BSS_COMPILER_DLLEXPORT __declspec(dllexport)
#define BSS_COMPILER_DLLIMPORT __declspec(dllimport)
#define MEMBARRIER_READWRITE __memory_barrier()
#define MEMBARRIER_READ MEMBARRIER_READWRITE
#define MEMBARRIER_WRITE MEMBARRIER_READWRITE

# elif defined __GNUC__ // GCC
#define BSS_COMPILER_DLLEXPORT __attribute__((dllexport))
#define BSS_COMPILER_DLLIMPORT __attribute__((dllimport))
#define MEMBARRIER_READWRITE __asm__ __volatile__ ("" ::: "memory");
#define MEMBARRIER_READ MEMBARRIER_READWRITE
#define MEMBARRIER_WRITE MEMBARRIER_READWRITE
#define BSS_COMPILER_FASTCALL __attribute__((fastcall))
#define BSS_COMPILER_STDCALL __attribute__((BSS_COMPILER_STDCALL__))

#elif defined _MSC_VER // VC++
#define BSS_COMPILER_DLLEXPORT __declspec(dllexport)
#define BSS_COMPILER_DLLIMPORT __declspec(dllimport)
#define MEMBARRIER_READWRITE _ReadWriteBarrier()
#define MEMBARRIER_READ _ReadBarrier()
#define MEMBARRIER_WRITE _WriteBarrier()
#define BSS_COMPILER_FASTCALL __fastcall
#define BSS_COMPILER_STDCALL __stdcall

#endif

// CPU Architecture (possible pre-defined macros found on http://predef.sourceforge.net/prearch.html)
#if defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_LP64)
#define BSS_CPU_x86_64  //x86-64 architecture
#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64)
#define BSS_CPU_IA_64 //Itanium (IA-64) architecture
#elif defined(_M_IX86) || defined(__i386) || defined(__i386__) || defined(__X86__) || defined(_X86_) || defined(__I86__) || defined(__THW_INTEL__) || defined(__INTEL__)
#define BSS_CPU_x86  //x86 architecture
#else
#define BSS_CPU_UNKNOWN //Unknown CPU architecture (should force architecture independent C implementations for all utilities)
#endif


#endif