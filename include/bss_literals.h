// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_LITERALS_H__
#define __BSS_LITERALS_H__

#ifndef __STANDARDIZED_LITERALS__
#define UNDERSCORE(x) x
#else
#define UNDERSCORE(x) _##x
#endif

#include <math.h>
#include <string>
#include "bss_util.h"

std::complex<long double> operator "" UNDERSCORE(i)(long double d) { return std::complex<long double>(0, d); }
std::string operator "" UNDERSCORE(s)(const char* str, size_t l) { return std::string(str,l); }
long double operator "" UNDERSCORE(deg)(long double d) { return d*(bss_util::PI/180.0); }
long double operator "" UNDERSCORE(rad)(long double d) { return d*(180.0/bss_util::PI); }
VersionType operator "" UNDERSCORE(ver)(const char* str, size_t) {
  char* context;
  VersionType r;
  r.Major = (unsigned char)(atoi(STRTOK(str,".",context))&0xFF);
  r.Minor = (unsigned char)(atoi(STRTOK(NULL,".",context))&0xFF);
  r.Revision = (unsigned short)(atoi(STRTOK(NULL,".",context))&0xFFFF);
  return r;
}
//cRational operator "" UNDERSCORE(over)(long double d) { return cRational(d,1); }

#endif