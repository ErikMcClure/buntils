// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/bss_util.h"
#include "bss-util/INIentry.h"
#include <sstream>
#include <iomanip>
#include <stdlib.h>

using namespace bss;

INIentry::INIentry() : _ivalue(0), _dvalue(0.0)
{}
INIentry::INIentry(const char *key, const char *svalue, int64_t ivalue, double dvalue) : _key(key), _svalue(svalue),
_ivalue(ivalue), _dvalue(dvalue)
{}
INIentry::INIentry(const char* key, const char* data) : _key(key) { Set(data); }

INIentry::~INIentry()
{}

bool INIentry::operator ==(INIentry &other) const { return STRICMP(_key, other._key) == 0 && STRICMP(_svalue, other._svalue) == 0 && _ivalue == other._ivalue && _dvalue == other._dvalue; }

//bool INIentry<wchar_t>::operator ==(INIentry &other) const { return WCSICMP(_key,other._key)==0 && WCSICMP(_svalue,other._svalue)==0; }
//bool INIentry<wchar_t>::operator !=(INIentry &other) const { return WCSICMP(_key,other._key)!=0 || WCSICMP(_svalue,other._svalue)!=0; }

void INIentry::Set(const char* data)
{
  if(!data) return;
  _svalue = data;

  if(_svalue[0] == '0' && (_svalue[1] == 'x' || _svalue[1] == 'X')) //If this is true its a hex number
  {
    uint64_t v = STRTOULL(_svalue, 0, 16); //We store the unsigned here so it gets properly stored in the double even if it overflows on the signed int64_t
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
void INIentry::SetInt(int64_t i)
{
  _ivalue = i;
  _dvalue = (double)i;
  _svalue = std::to_string(i);
}
void INIentry::SetFloat(double d)
{
  _ivalue = (int64_t)d;
  _dvalue = d;
  _svalue = std::to_string(d);
}
