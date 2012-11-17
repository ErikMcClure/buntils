// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INIENTRY_H__BSS__
#define __C_INIENTRY_H__BSS__

#include "cStr.h"
#include "bss_dlldef.h"

namespace bss_util {
  // Stores an INI entry and allows it to be accessed via multiple type translations
  struct BSS_DLLEXPORT cINIentry
  {
    cINIentry(const cINIentry& copy);
    cINIentry(cINIentry&& mov);
    cINIentry();
    cINIentry(const char* key, const char* svalue, __int64 ivalue, double dvalue);
    cINIentry(const char* key, const char* data);
    ~cINIentry();
    void SetData(const char* data);
    inline BSS_FORCEINLINE const char* GetKey() const { return _key; }
    //inline BSS_FORCEINLINE unsigned int GetIndex() const { return _index; }
    inline BSS_FORCEINLINE const char* GetString() const { return _svalue; }
    inline BSS_FORCEINLINE __int64 GetInt() const { return _ivalue; }
    inline BSS_FORCEINLINE double GetDouble() const { return _dvalue; }
    inline BSS_FORCEINLINE bool IsValid() const { return !_key.empty(); }

    inline BSS_FORCEINLINE operator bool() const { return _ivalue!=0; }
    inline BSS_FORCEINLINE operator char() const { return (char)_ivalue; }
    inline BSS_FORCEINLINE operator short() const { return (short)_ivalue; }
    inline BSS_FORCEINLINE operator int() const { return (int)_ivalue; }
    inline BSS_FORCEINLINE operator long() const { return (long)_ivalue; }
    inline BSS_FORCEINLINE operator __int64() const { return (__int64)_ivalue; }
    inline BSS_FORCEINLINE operator unsigned char() const { return (unsigned char)_ivalue; }
    inline BSS_FORCEINLINE operator unsigned short() const { return (unsigned short)_ivalue; }
    inline BSS_FORCEINLINE operator unsigned int() const { return (unsigned int)_ivalue; }
    inline BSS_FORCEINLINE operator unsigned long() const { return (unsigned long)_ivalue; }
    inline BSS_FORCEINLINE operator unsigned __int64() const { return (unsigned __int64)_ivalue; }
    inline BSS_FORCEINLINE operator float() const { return (float)_dvalue; }
    inline BSS_FORCEINLINE operator double() const { return _dvalue; }
    inline BSS_FORCEINLINE operator const char*() const { return _svalue; }

    bool operator ==(cINIentry &other) const; //these can't be inlined because the compare function is different.
    bool operator !=(cINIentry &other) const;
    cINIentry& operator=(cINIentry&& mov);

  private:
    cStr _key;
    cStr _svalue;
    __int64 _ivalue;
    double _dvalue;
  };
}

#endif