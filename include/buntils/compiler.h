// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_COMPILER_H__
#define __BUN_COMPILER_H__

// CPU Architecture (possible pre-defined macros found on http://predef.sourceforge.net/prearch.html)
#if defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(__x86_64__) || defined(__x86_64) || defined(_LP64)
#define BUN_CPU_x86_64  //x86-64 architecture
#define BUN_64BIT
#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64)
#define BUN_CPU_IA_64 //Itanium (IA-64) architecture
#define BUN_64BIT
#elif defined(_M_IX86) || defined(__i386) || defined(__i386__) || defined(__X86__) || defined(_X86_) || defined(__I86__) || defined(__THW_INTEL__) || defined(__INTEL__)
#define BUN_CPU_x86  //x86 architecture
#define BUN_32BIT
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || defined(_ARM)
#define BUN_CPU_ARM //ARM architecture
//#ifndef(???) //ARMv8 will support 64-bit so we'll have to detect that somehow, and it's the first to make NEON standardized.
#define BUN_32BIT
//#else
//#define BUN_64BIT
//#endif
#elif defined(__mips__) || defined(mips) || defined(_MIPS_ISA) || defined(__mips) || defined(__MIPS__)
#define BUN_CPU_MIPS
#define BUN_64BIT
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(_M_PPC) || defined(_ARCH_PPC)
#define BUN_CPU_POWERPC
#define BUN_32BIT
#else
#define BUN_CPU_UNKNOWN //Unknown CPU architecture (should force architecture independent C implementations for all utilities)
#endif

// Compiler detection and macro generation
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC) // Intel C++ compiler
#define BUN_COMPILER_INTEL
#define BUN_COMPILER_DLLEXPORT __declspec(dllexport)
#define BUN_COMPILER_DLLIMPORT __declspec(dllimport)
#define BUN_TEMPLATE_DLLEXPORT
#define MEMBARRIER_READWRITE __memory_barrier()
#define MEMBARRIER_READ MEMBARRIER_READWRITE
#define MEMBARRIER_WRITE MEMBARRIER_READWRITE
#define BUN_COMPILER_FASTCALL
#define BUN_COMPILER_STDCALL
#define BUN_COMPILER_NAKED
#define BUN_FORCEINLINE inline
#define BUN_RESTRICT __restrict__
#define BUN_ALIGNED(sn, n) sn
#define MSC_FASTCALL BUN_COMPILER_FASTCALL
#define GCC_FASTCALL 
#define BUN_UNREACHABLE() 
#define BUN_ASSUME(x) 
#define BUN_SSE_ENABLED
#define BUN_COMPILER_HAS_TIME_GET

#elif defined(__clang__) // Clang (must be before GCC, because clang also pretends it's GCC)
#define BUN_COMPILER_CLANG
#define BUN_COMPILER_DLLEXPORT __attribute__((dllexport))
#define BUN_COMPILER_DLLIMPORT __attribute__((dllimport))
#define BUN_TEMPLATE_DLLEXPORT
#define MEMBARRIER_READWRITE __asm__ __volatile__ ("" ::: "memory");
#define MEMBARRIER_READ MEMBARRIER_READWRITE
#define MEMBARRIER_WRITE MEMBARRIER_READWRITE
#define BUN_COMPILER_FASTCALL __attribute__((fastcall))
#define BUN_COMPILER_STDCALL __attribute__((stdcall))
#define BUN_COMPILER_NAKED __attribute__((naked)) // Will only work on ARM, AVR, MCORE, RX and SPU. 
#define BUN_FORCEINLINE __attribute__((always_inline)) inline
#define BUN_RESTRICT __restrict__
#define BUN_ALIGN(n) __attribute__((aligned(n)))
#define BUN_ALIGNED(sn, n) sn BUN_ALIGN(n)
#define BUN_COMPILER_HAS_TIME_GET

#define MSC_FASTCALL 
#define GCC_FASTCALL BUN_COMPILER_FASTCALL
//#define BUN_SSE_ENABLED

#elif defined __GNUC__ // GCC
#define BUN_COMPILER_GCC
#define BUN_COMPILER_DLLEXPORT __attribute__((dllexport))
#define BUN_COMPILER_DLLIMPORT __attribute__((dllimport))
#define BUN_TEMPLATE_DLLEXPORT
#define MEMBARRIER_READWRITE __asm__ __volatile__ ("" ::: "memory");
#define MEMBARRIER_READ MEMBARRIER_READWRITE
#define MEMBARRIER_WRITE MEMBARRIER_READWRITE
#define BUN_COMPILER_FASTCALL __attribute__((fastcall))
#define BUN_COMPILER_STDCALL __attribute__((stdcall))
#define BUN_COMPILER_NAKED __attribute__((naked)) // Will only work on ARM, AVR, MCORE, RX and SPU. 
#define BUN_FORCEINLINE __attribute__((always_inline)) inline
#define BUN_RESTRICT __restrict__
#define BUN_ALIGN(n) __attribute__((aligned(n)))
#define BUN_ALIGNED(sn, n) sn BUN_ALIGN(n)

#if __GNUC__ >= 5 && __GNUC_MINOR__ >= 1
#define BUN_COMPILER_HAS_TIME_GET
#endif

#define MSC_FASTCALL 
#define GCC_FASTCALL BUN_COMPILER_FASTCALL
#define BUN_SSE_ENABLED

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_unreachable) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define PORTABLE_UNREACHABLE() __builtin_unreachable()
#define PORTABLE_ASSUME(x) do { if (!(x)) { __builtin_unreachable(); } } while (0)
#endif

#elif defined _MSC_VER // VC++
#define BUN_COMPILER_MSC
#define BUN_COMPILER_DLLEXPORT __declspec(dllexport)
#define BUN_COMPILER_DLLIMPORT __declspec(dllimport)
#define BUN_TEMPLATE_DLLEXPORT BUN_COMPILER_DLLEXPORT
#define MEMBARRIER_READWRITE _ReadWriteBarrier()
#define MEMBARRIER_READ _ReadBarrier()
#define MEMBARRIER_WRITE _WriteBarrier()
#define BUN_COMPILER_FASTCALL __fastcall
#define BUN_COMPILER_STDCALL __stdcall
#define BUN_COMPILER_NAKED __declspec(naked) 
#define BUN_FORCEINLINE __forceinline
#define BUN_RESTRICT __restrict
#define BUN_ALIGN(n) __declspec(align(n))
#define BUN_ALIGNED(sn, n) BUN_ALIGN(n) sn
#define BUN_VERIFY_HEAP _ASSERTE(_CrtCheckMemory())
#define FUNCPTRCC(m,CC) CC m
#define MSC_FASTCALL BUN_COMPILER_FASTCALL
#define GCC_FASTCALL 
#define BUN_SSE_ENABLED
#define BUN_COMPILER_HAS_TIME_GET

#define BUN_UNREACHABLE() __assume(0)
#define BUN_ASSUME(x) __assume(x)
#if (_MANAGED == 1) || (_M_CEE == 1)
#define MSC_MANAGED
#endif
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#define BUN_PLATFORM_MINGW // Should also define WIN32, use only for minGW specific bugs
#endif

#define BUN_ALIGNED_STRUCT(n) struct BUN_ALIGN(n)
#define BUN_ALIGNED_CLASS(n) class BUN_ALIGN(n)
#define BUN_ALIGNED_UNION(n) union BUN_ALIGN(n)

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define BUN_PLATFORM_WIN32
#elif defined(_POSIX_VERSION) || defined(_XOPEN_VERSION) || defined(unix) || defined(__unix__) || defined(__unix)
#define BUN_PLATFORM_POSIX
#endif

#ifdef _WIN32_WCE
#define BUN_PLATFORM_WIN32_CE // Implies WIN32
#elif defined(__APPLE__) || defined(__MACH__) || defined(macintosh) || defined(Macintosh)
#define BUN_PLATFORM_APPLE // Should also define POSIX, use only for Apple OS specific features
#elif defined(__CYGWIN__)
#define BUN_PLATFORM_CYGWIN // Should also define POSIX, use only to deal with Cygwin weirdness
#elif defined(__ANDROID__) || defined(__ANDROID_API__) 
#define BUN_PLATFORM_ANDROID // Should also define POSIX, use for Android specific features.
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(BSD) // Also defines POSIX
#define BUN_PLATFORM_BSD // Should also define POSIX
#elif defined(sun) || defined(__sun) 
# define BUN_PLATFORM_SOLARIS
# if !defined(__SVR4) && !defined(__svr4__)
#   define BUN_PLATFORM_SUNOS
# endif
#endif

#if defined(__linux__) || defined(__linux)
#define BUN_PLATFORM_LINUX // Should also define POSIX, use only for linux specific features
#endif

#if !(defined(BUN_PLATFORM_WIN32) || defined(BUN_PLATFORM_POSIX) || defined(BUN_PLATFORM_WIN32_CE) || defined(BUN_PLATFORM_APPLE))
#error "Unknown Platform"
#endif

// Endianness detection
#if defined(BUN_PLATFORM_WIN32) || defined(BUN_PLATFORM_WIN32_CE) || defined(BUN_CPU_x86_64) || defined(BUN_CPU_x86) || defined(BUN_CPU_IA_64) // Windows, x86, x86_64 and itanium all only run in little-endian (except on HP-UX but we don't support that)
# define BUN_ENDIAN_LITTLE
#elif defined(BUN_CPU_ARM)
# ifdef BUN_PLATFORM_LINUX
#   define BUN_ENDIAN_LITTLE
# endif
#elif defined(BUN_CPU_POWERPC)
# ifdef BUN_PLATFORM_SOLARIS
#   define BUN_ENDIAN_LITTLE
# elif defined(BUN_PLATFORM_APPLE) || defined(BUN_PLATFORM_BSD) || defined(BUN_PLATFORM_LINUX)
#   define BUN_ENDIAN_BIG
# endif
#elif defined(BUN_CPU_MIPS) // MIPS is a bitch to detect endianness on
# ifdef BUN_PLATFORM_LINUX
#   define BUN_ENDIAN_BIG
# endif
#endif

#if !defined(BUN_ENDIAN_LITTLE) && !defined(BUN_ENDIAN_BIG)
#error "Unknown Endianness"
#endif

// Debug detection
#ifdef BUN_COMPILER_GCC
#ifndef NDEBUG
#define BUN_DEBUG
#endif
#else
#if defined(DEBUG) || defined(_DEBUG)
#define BUN_DEBUG
#endif
#endif

#ifdef BUN_DISABLE_SSE 
#undef BUN_SSE_ENABLED
#endif

#ifdef BUN_UTIL_EXPORTS
#define BUN_DLLEXPORT BUN_COMPILER_DLLEXPORT
#else
#ifndef BUN_STATIC_LIB
#define BUN_DLLEXPORT BUN_COMPILER_DLLIMPORT
#else
#define BUN_DLLEXPORT
#endif
#endif

#pragma warning(push)
#pragma warning(disable:4201)
typedef union BUN_VERSION_INFO {
  struct {
    unsigned short Build;
    unsigned short Revision;
    unsigned short Minor;
    unsigned short Major;
  };
  unsigned short v[4];
  unsigned long long version;
} bun_VersionInfo;
#pragma warning(pop)

#endif
