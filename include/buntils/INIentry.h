// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __INIENTRY_H__BUN__
#define __INIENTRY_H__BUN__

#include "Str.h"

namespace bun {
  // Stores an INI entry and allows it to be accessed via multiple type translations
  struct BUN_DLLEXPORT INIentry
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
    BUN_FORCEINLINE const char* GetKey() const { return _key; }
    //BUN_FORCEINLINE uint32_t GetIndex() const { return _index; }
    BUN_FORCEINLINE const char* GetString() const { return _svalue; }
    BUN_FORCEINLINE int64_t GetInt() const { return _ivalue; }
    BUN_FORCEINLINE double GetDouble() const { return _dvalue; }
    BUN_FORCEINLINE bool IsValid() const { return !_key.empty(); }

    BUN_FORCEINLINE operator bool() const { return _ivalue != 0; }
    BUN_FORCEINLINE operator char() const { return static_cast<char>(_ivalue); }
    BUN_FORCEINLINE operator short() const { return static_cast<short>(_ivalue); }
    BUN_FORCEINLINE operator int() const { return static_cast<int>(_ivalue); }
    BUN_FORCEINLINE operator int64_t() const { return static_cast<int64_t>(_ivalue); }
    BUN_FORCEINLINE operator uint8_t() const { return static_cast<uint8_t>(_ivalue); }
    BUN_FORCEINLINE operator uint16_t() const { return static_cast<uint16_t>(_ivalue); }
    BUN_FORCEINLINE operator uint32_t() const { return static_cast<uint32_t>(_ivalue); }
    BUN_FORCEINLINE operator uint64_t() const { return static_cast<uint64_t>(_ivalue); }
    BUN_FORCEINLINE operator float() const { return static_cast<float>(_dvalue); }
    BUN_FORCEINLINE operator double() const { return _dvalue; }
    BUN_FORCEINLINE operator const char*() const { return _svalue; }

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
