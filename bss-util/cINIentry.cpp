// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cINIentry.h"
#include <sstream>
#include <iomanip>
#include <stdlib.h>

using namespace bss_util;

cINIentry::cINIentry(const cINIentry& mov) : _key(mov._key),_svalue(mov._svalue),_ivalue(mov._ivalue),_dvalue(mov._dvalue)
{
}
cINIentry::cINIentry(cINIentry&& mov) : _key(std::move(mov._key)),_svalue(std::move(mov._svalue)),_ivalue(mov._ivalue),_dvalue(mov._dvalue)
{
}
cINIentry::cINIentry() : _ivalue(0),_dvalue(0.0)//,_index(0)
{
}
cINIentry::cINIentry(const char *key, const char *svalue, int64_t ivalue, double dvalue) : _key(key),_svalue(svalue),
  _ivalue(ivalue),_dvalue(dvalue)//,_index(index)
{
}
cINIentry::cINIentry(const char* key, const char* data) : _key(key)//,_index(index)
{
  SetData(data);
}

cINIentry::~cINIentry()
{
}

cINIentry& cINIentry::operator=(cINIentry&& mov)
{
  _key=std::move(mov._key);
  _svalue=std::move(mov._svalue);
  _ivalue=mov._ivalue;
  _dvalue=mov._dvalue;
  return *this;
}

bool cINIentry::operator ==(cINIentry &other) const { return STRICMP(_key,other._key)==0 && STRICMP(_svalue,other._svalue)==0; }
bool cINIentry::operator !=(cINIentry &other) const { return STRICMP(_key,other._key)!=0 || STRICMP(_svalue,other._svalue)!=0; }

//bool cINIentry<wchar_t>::operator ==(cINIentry &other) const { return WCSICMP(_key,other._key)==0 && WCSICMP(_svalue,other._svalue)==0; }
//bool cINIentry<wchar_t>::operator !=(cINIentry &other) const { return WCSICMP(_key,other._key)!=0 || WCSICMP(_svalue,other._svalue)!=0; }

void cINIentry::SetData(const char* data)
{  
  if(!data) return;
  _svalue=data;

  if(_svalue[0] == '0' && (_svalue[1] == 'x' || _svalue[1] == 'X')) //If this is true its a hex number
  {
    uint64_t v = STRTOULL(_svalue,0,16); //We store the unsigned here so it gets properly stored in the double even if it overflows on the signed int64_t
    _ivalue = (int64_t)v;
    _dvalue = (double)v;
  }
  else if(!strchr(_svalue, '.')) //If there isn't a period in the sequence, its either a string (in which case we don't care) or an integer, so round _dvalue to the integer.
  {
    _ivalue = ATOLL(_svalue);
    _dvalue = (double)_ivalue;
  }
  else //Ok its got a . in there so its either a double or a string, so we just round _ivalue
  {
    _dvalue = atof(_svalue);
    _ivalue = (int64_t)_dvalue;
  }
}

//#define STR_RT "rt"
//#define STR_NONE ""
//#define STR_SL "["
//
//#define STR_NSES "\n%s=%s"
//#define STR_WT "wt"
//#define STR_APT "a+b"
//#define STR_LB "]"

/*
#define char char
#include "cINIstorage.inl"
#undef char
#define char wchar_t
#undef FOPEN
#define FOPEN WFOPEN
#define strlen wcslen
#define strchr wcschr
#define strrchr wcsrchr
#define fputs fputws
#define INIParser INIParserW
#define bss_findINIentry bss_wfindINIentry
#define bss_findINIsection bss_wfindINIsection
#define bss_initINI bss_winitINI
#define bss_destroyINI bss_wdestroyINI
#define bss_parseLine bss_wparseLine
#define fwrite _futfwrite

#define (s) WIDEN(s)
//#undef STR_RT
//#undef STR_WT
//#undef STR_NONE
//#undef STR_NNSL
//#undef STR_NSES
//#undef STR_APT 
//#undef STR_LB
//#define STR_RT L"rb" //So apparently text mode is assumed to be ASCII... even if you use a wchar_t function. You have to open a unicode file in binary and the parser magically works
//#define STR_WT L"wb"
//#define STR_NONE L""
//#define STR_NNSL L"\n\n["
//#define STR_NSES L"\n%s=%s"
//#define STR_APT L"a+b"
//#define STR_LB L"]"

#include "cINIstorage.inl"
#undef char
#undef FOPEN
#undef strchr*/