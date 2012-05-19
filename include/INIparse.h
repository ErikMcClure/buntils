// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __INIPARSE_H__BSS__
#define __INIPARSE_H__BSS__

#include "bss_dlldef.h"
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

//typedef struct {
//  FILE* file;
//  wchar_t* curvalue;
//  wchar_t* curkey;
//  wchar_t* cursection;
//  char newsection; //set to 1 if a new section was found during parsing
//  wchar_t buf[MAXLINELENGTH];
//} INIParserW;

typedef struct {
  const void* start;
  const void* end;
} INICHUNK;

extern char BSS_FASTCALL bss_initINI(INIParser* init, FILE* stream);
extern char BSS_FASTCALL bss_destroyINI(INIParser* destroy);
extern char BSS_FASTCALL bss_parseLine(INIParser* parse);
extern INICHUNK BSS_FASTCALL bss_findINIsection(const void* data, size_t length, const char* section, unsigned int instance);
extern INICHUNK BSS_FASTCALL bss_findINIentry(INICHUNK section, const char* key, unsigned int instance);
//extern char BSS_FASTCALL bss_winitINI(INIParserW* init, FILE* stream);
//extern char BSS_FASTCALL bss_wdestroyINI(INIParserW* destroy);
//extern char BSS_FASTCALL bss_wparseLine(INIParserW* parse);
//extern INICHUNK BSS_FASTCALL bss_wfindINIsection(const void* data, unsigned length, const wchar_t* section, unsigned int instance);
//extern INICHUNK BSS_FASTCALL bss_wfindINIentry(INICHUNK section, const wchar_t* key, unsigned int instance);

#ifdef  __cplusplus
}
#endif

#endif