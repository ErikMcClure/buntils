// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __INIENTRY_H__BSS__
#define __INIENTRY_H__BSS__

#include "Str.h"

namespace bss {
  // Stores an INI entry and allows it to be accessed via multiple type translations
  struct BSS_DLLEXPORT INIentry
  {
    INIentry(const INIentry&) = default;
    INIentry(INIentry&&) = default;
    INIentry();
    INIentry(const char* key, const char* svalue, int64_t ivalue, double dvalue);
    INIentry(const char* key, const char* data);
    ~INIentry();
    void Set(const char* data);
    void SetInt(int64_t data);
    void SetFloat(double data);
    BSS_FORCEINLINE const char* GetKey() const { return _key; }
    //BSS_FORCEINLINE uint32_t GetIndex() const { return _index; }
    BSS_FORCEINLINE const char* GetString() const { return _svalue; }
    BSS_FORCEINLINE int64_t GetInt() const { return _ivalue; }
    BSS_FORCEINLINE double GetDouble() const { return _dvalue; }
    BSS_FORCEINLINE bool IsValid() const { return !_key.empty(); }

    BSS_FORCEINLINE operator bool() const { return _ivalue != 0; }
    BSS_FORCEINLINE operator char() const { return (char)_ivalue; }
    BSS_FORCEINLINE operator short() const { return (short)_ivalue; }
    BSS_FORCEINLINE operator int() const { return (int)_ivalue; }
    BSS_FORCEINLINE operator int64_t() const { return (int64_t)_ivalue; }
    BSS_FORCEINLINE operator uint8_t() const { return (uint8_t)_ivalue; }
    BSS_FORCEINLINE operator uint16_t() const { return (uint16_t)_ivalue; }
    BSS_FORCEINLINE operator uint32_t() const { return (uint32_t)_ivalue; }
    BSS_FORCEINLINE operator uint64_t() const { return (uint64_t)_ivalue; }
    BSS_FORCEINLINE operator float() const { return (float)_dvalue; }
    BSS_FORCEINLINE operator double() const { return _dvalue; }
    BSS_FORCEINLINE operator const char*() const { return _svalue; }

    bool operator ==(INIentry &other) const; //these can't be inlined because the compare function is different.
    inline bool operator !=(INIentry &other) const { return !operator==(other); }
    INIentry& operator=(INIentry&&) = default;
    INIentry& operator=(const INIentry&) = default;
    inline INIentry& operator=(const char* s) { Set(s); return *this; }
    inline INIentry& operator=(int64_t s) { SetInt(s); return *this; }
    inline INIentry& operator=(double s) { SetFloat(s); return *this; }

  private:
    Str _key;
    Str _svalue;
    int64_t _ivalue;
    double _dvalue;
  };
}

#endif
