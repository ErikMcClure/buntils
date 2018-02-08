// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BITSTREAM_H__BSS__
#define __BITSTREAM_H__BSS__

#include "bss_util.h"
#include "Str.h"
#include <iostream>

namespace bss {
  // This wraps around an existing stream and flushes to it. Capable of writing and reading with single-bit precision.
  template<typename STREAM = std::iostream>
  class BSS_COMPILER_DLLEXPORT BitStream
  {
  public:
    BitStream(const BitStream& copy) : _base(copy._base), _roffset(copy._roffset), _woffset(copy._woffset), _buf(copy._buf) {}
    BitStream(BitStream&& mov) : _base(mov._base), _roffset(mov._roffset), _woffset(mov._woffset), _buf(mov._buf) { mov._base = 0; }
    explicit BitStream(STREAM& base) : _base(&base), _roffset(0), _woffset(0), _buf(0) {}
    ~BitStream() { if(_base) Flush(); }
    void Write(const void* source, int bits)
    {
      const uint8_t* d = (const uint8_t*)source;
      int k = 0;
      int i;

      for(i = bits; i >= 8; i -= 8)
      {
        _buf |= (d[k] << _woffset);
        _base->put(_buf);
        _buf = (d[k++] >> (8 - _woffset));
      }

      if(!i)
        return;

      _buf |= (d[k] << _woffset);

      if((_woffset + i) >= 8) // We ran over our current character
      {
        _base->put(_buf);
        _buf = (d[k] >> (8 - _woffset));
      }

      _woffset = ((_woffset + i) & 7);
      _buf = (_buf&((1 << _woffset) - 1)); // This zeros out all bits we didn't assign in _buf
    }
    template<typename T>
    void Write(const T& source) { Write(&source, std::conditional<std::is_same<T, bool>::value, std::integral_constant<int, 1>, std::integral_constant<int, (sizeof(T) << 3)>>::type::value); }
    void Read(void* dest, int bits)
    {
      uint8_t* d = (uint8_t*)dest;
      uint8_t b;
      int k = 0;
      int i;

      for(i = bits; i >= 8; i -= 8)
      {
        d[k] = (_base->peek() >> _roffset);
        _base->get(*((char*)&b));
        d[k++] |= (_base->peek() << (8 - _roffset));
      }

      if(!i)
        return;

      d[k] |= (_base->peek() >> _roffset);

      if((_roffset + i) >= 8) // We ran over our current character
      {
        _base->get(*((char*)&b));
        if((_roffset + i) > 8) // We do this so we don't accidentally peek into an invalid character
          d[k] |= (_base->peek() << (8 - _roffset));
      }

      _roffset = ((_roffset + i) & 7);
      d[k] = (d[k] & ((1 << i) - 1)); // This zeros out all bits we didn't assign
    }
    template<typename T>
    void Read(T& dest) { Read(&dest, std::conditional<std::is_same<T, bool>::value, std::integral_constant<int, 1>, std::integral_constant<int, (sizeof(T) << 3)>>::type::value); }
    void Flush() { if(_woffset) _flush(_base, _buf); }

    BitStream& operator=(const BitStream& copy) { _base = copy._base; return *this; }
    BitStream& operator=(BitStream&& mov) { _base = mov._base; mov._base = 0; return *this; }
    template<typename T>
    BitStream& operator<<(const T& v) { Write<T>(v); return *this; }
    template<typename T>
    BitStream& operator>>(T& v) { Read<T>(v); return *this; }

  protected:
    template<bool U = std::is_base_of<std::ostream, STREAM>::value>
    BSS_FORCEINLINE static typename std::enable_if<U, void>::type _flush(STREAM* s, uint8_t buf) { s->write((char*)&buf, 1); }
    template<bool U = std::is_base_of<std::ostream, STREAM>::value>
    BSS_FORCEINLINE static typename std::enable_if<!U, void>::type _flush(STREAM* s, uint8_t buf) { }

    STREAM* _base;
    uint8_t _buf;
    uint8_t _roffset; // Offset from current reading position in bits
    uint8_t _woffset; // Offset from current writing position in bits
  };

  // This is a function that takes a basic type and serializes it, converting it to little-endian format if necessary
  template<class T>
  void bssSerialize(T d, std::ostream& s)
  {
#ifdef BSS_ENDIAN_BIG
    FlipEndian((char*)&d, sizeof(T));
#endif
    s.write((const char*)&d, sizeof(T));
  }
  template<>
  void bssSerialize<const char*>(const char* d, std::ostream& s)
  {
    size_t len = strlen(d);
    bssSerialize(len, s);
    s.write(d, len);
  }

  template<class T>
  void bssDeserialize(T& d, std::istream& s)
  {
    s.read((char*)&d, sizeof(T));
#ifdef BSS_ENDIAN_BIG
    FlipEndian((char*)&d, sizeof(T));
#endif
  }
  template<>
  void bssDeserialize<std::string>(std::string& d, std::istream& s)
  {
    size_t len;
    bssDeserialize(len, s);
    d.resize(len);
    s.read(d.data(), len);
  }
  template<>
  void bssDeserialize<Str>(Str& d, std::istream& s) { bssDeserialize<std::string>(d, s); }
}

#endif