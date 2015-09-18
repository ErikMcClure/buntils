// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __INIPARSE_H__BSS__
#define __INIPARSE_H__BSS__

#include "bss_defines.h"
#include <wchar.h>
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define MAXLINELENGTH 1024

typedef struct {
  //const char* file;
  //const char* curloc;
  //const char* end;
  //unsigned int length;
  FILE* file;
  char* curvalue;
  char* curkey;
  char* cursection;
  char newsection; //set to 1 if a new section was found during parsing
  char buf[MAXLINELENGTH];
} INIParser;

typedef struct {
  const void* start;
  const void* end;
} INICHUNK;

extern char BSS_FASTCALL bss_initINI(INIParser* init, FILE* stream);
extern char BSS_FASTCALL bss_destroyINI(INIParser* destroy);
extern char BSS_FASTCALL bss_parseLine(INIParser* parse);
extern INICHUNK BSS_FASTCALL bss_findINIsection(const void* data, size_t length, const char* section, size_t instance);
extern INICHUNK BSS_FASTCALL bss_findINIentry(INICHUNK section, const char* key, size_t instance);

#ifdef  __cplusplus
}
#endif

#endif
