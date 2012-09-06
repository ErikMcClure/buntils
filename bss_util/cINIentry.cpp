// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cINIentry.h"
#include <sstream>
#include <iomanip>

using namespace bss_util;

cINIentry::cINIentry(cINIentry&& mov) : _key(std::move(mov._key)),_svalue(std::move(mov._svalue)),_lvalue(mov._lvalue),_dvalue(mov._dvalue)
{
}
cINIentry::cINIentry() : _lvalue(0),_dvalue(0.0)//,_index(0)
{
}
cINIentry::cINIentry(const char *key, const char *svalue, long lvalue, double dvalue) : _key(key),_svalue(svalue),
  _lvalue(lvalue),_dvalue(dvalue)//,_index(index)
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
  _lvalue=mov._lvalue;
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
    std::string s(_svalue);
    unsigned long v;
    std::istringstream iss(s);
    iss >> std::setbase(0) >> v;
    std::setbase(10); //reset base to 10
    _lvalue = (long)v;
    _dvalue = (double)v;
  }
  else if(!strchr(_svalue, '.')) //If there isn't a period in the sequence, its either a string (in which case we don't care) or an integer, so set _dvalue to invalid.
  {
    _lvalue = atol(_svalue);
    _dvalue = (double)_lvalue;
  }
  else //Ok its got a . in there so its either a double or a string, so we just round _lvalue
  {
    _dvalue = atof(_svalue);
    _lvalue = (long)_dvalue;
  }
}

//
//const char* cINIentry::GetKey() const { return _key; }
//const char* cINIentry::GetString() const { return _svalue; }
//long cINIentry::GetLong() const { return _lvalue; }
//double cINIentry::GetDouble() const { return _dvalue; }
//
//cINIentry::operator bool() const { return _lvalue!=0; }
//cINIentry::operator short() const { return (short)_lvalue; }
//cINIentry::operator int() const { return (int)_lvalue; }
//cINIentry::operator long() const { return (long)_lvalue; }
//cINIentry::operator unsigned short() const { return (unsigned short)_lvalue; }
//cINIentry::operator unsigned int() const { return (unsigned int)_lvalue; }
//cINIentry::operator unsigned long() const { return (unsigned long)_lvalue; }
//cINIentry::operator float() const { return (float)_dvalue; }
//cINIentry::operator double() const { return _dvalue; }
//cINIentry::operator const char*() const { return _svalue; }

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