// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INIENTRY_H__BSS__
#define __C_INIENTRY_H__BSS__

#include "cStr.h"

namespace bss_util {
  // Stores an INI entry and allows it to be accessed via multiple type translations
  struct BSS_DLLEXPORT cINIentry
  {
    cINIentry(const cINIentry& copy);
    cINIentry(cINIentry&& mov);
    cINIentry();
    cINIentry(const char* key, const char* svalue, int64_t ivalue, double dvalue);
    cINIentry(const char* key, const char* data);
    ~cINIentry();
    void SetData(const char* data);
    BSS_FORCEINLINE const char* GetKey() const { return _key; }
    //BSS_FORCEINLINE uint32_t GetIndex() const { return _index; }
    BSS_FORCEINLINE const char* GetString() const { return _svalue; }
    BSS_FORCEINLINE int64_t GetInt() const { return _ivalue; }
    BSS_FORCEINLINE double GetDouble() const { return _dvalue; }
    BSS_FORCEINLINE bool IsValid() const { return !_key.empty(); }

    BSS_FORCEINLINE operator bool() const { return _ivalue!=0; }
    BSS_FORCEINLINE operator char() const { return (char)_ivalue; }
    BSS_FORCEINLINE operator short() const { return (short)_ivalue; }
    BSS_FORCEINLINE operator int() const { return (int)_ivalue; }
    BSS_FORCEINLINE operator long() const { return (long)_ivalue; }
    BSS_FORCEINLINE operator int64_t() const { return (int64_t)_ivalue; }
    BSS_FORCEINLINE operator uint8_t() const { return (uint8_t)_ivalue; }
    BSS_FORCEINLINE operator uint16_t() const { return (uint16_t)_ivalue; }
    BSS_FORCEINLINE operator uint32_t() const { return (uint32_t)_ivalue; }
    BSS_FORCEINLINE operator unsigned long() const { return (unsigned long)_ivalue; }
    BSS_FORCEINLINE operator uint64_t() const { return (uint64_t)_ivalue; }
    BSS_FORCEINLINE operator float() const { return (float)_dvalue; }
    BSS_FORCEINLINE operator double() const { return _dvalue; }
    BSS_FORCEINLINE operator const char*() const { return _svalue; }

    bool operator ==(cINIentry &other) const; //these can't be inlined because the compare function is different.
    bool operator !=(cINIentry &other) const;
    cINIentry& operator=(cINIentry&& mov);

  private:
    cStr _key;
    cStr _svalue;
    int64_t _ivalue;
    double _dvalue;
  };
}

#endif
