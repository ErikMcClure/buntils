// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __INIPARSE_H__BUN__
#define __INIPARSE_H__BUN__

#include "defines.h"
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

  extern char bun_InitINI(INIParser* init, FILE* stream);
  extern char bun_DestroyINI(INIParser* destroy);
  extern char bun_ParseLine(INIParser* parse);
  extern INICHUNK bun_FindINISection(const void* data, size_t length, const char* section, size_t instance);
  extern INICHUNK bun_FindINIEntry(INICHUNK section, const char* key, size_t instance);

#ifdef  __cplusplus
}
#endif

#endif
