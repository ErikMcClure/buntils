// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "INIparse.h"
#include "bss_util_c.h"
#include <malloc.h>
#include <string.h>

const char VALIDASCII=33; //technically this works for unicode too

int _minfunc(int a, int b)
{
  return a<b?a:b;
}

/* this is a wrapper function that decodes a utf-8 file but still behaves like fgets */
void _fgetutf8(wchar_t* buf, int max, FILE* file)
{
  char ibuf[MAXLINELENGTH]; //we actually ignore max here because we know it will be this - we accept it so this is compatible with the other call
  fgets(ibuf,--max,file); //we subtract one from max so we have room for the null terminator in the unicode string
  max=UTF8Decode2BytesUnicode(ibuf,buf);
  buf[max]=0;
}

#define CHAR char
#include "INIParser.inl"

//This is a truly amazing abuse of #define to work around the fact that C has no templates or overloads, because copy+pasting the code would in fact be even worse.
#define bss_initINI bss_winitINI
#define bss_destroyINI bss_wdestroyINI
#define _trimlstr _wtrimlstr
#define _trimrstr _wtrimrstr
#define _nextkeychar _wnextkeychar
#define _validatesection _wvalidatesection
#define _validatekey _wvalidatekey
#define _validatevalue _wvalidatevalue
#define _savestr _wsavestr
#define _newsection _wnewsection
#define _newkeyvaluepair _wnewkeyvaluepair
#define _nextkeychar _wnextkeychar
#define bss_parseLine bss_wparseLine
#define _snextkeychar _swnextkeychar
#define _svalidatesection _swvalidatesection
#define _svalidatevalue _swvalidatevalue
#define _trimrstralt _wtrimrstralt
#define comparevalues wcomparevalues
#define bss_findINIsection bss_wfindINIsection
#define bss_findINIentry bss_wfindINIentry

#define INIParser INIParserW
#define strlen wcslen
#define _strnicmp _wcsnicmp
#define fgets _fgetutf8
#define strchr wcschr
#undef CHAR
#define CHAR wchar_t

#include "INIParser.inl"