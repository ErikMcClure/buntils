// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BITFIELD_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __BITFIELD_H__BSS__

#include "defines.h"
#include <utility>
#include <stdint.h>

namespace bss {
  namespace internal {
    template<typename T>
    struct BSS_COMPILER_DLLEXPORT _BIT_REF
    {
      inline _BIT_REF(T bit, T& bits) : _bit(bit), _bits(bits) {}
      BSS_FORCEINLINE _BIT_REF& operator=(bool right)
      { 
        _bits = T_SETBIT(_bits, _bit, right, typename std::make_signed<T>::type); 
        return *this; 
      }
      BSS_FORCEINLINE _BIT_REF& operator=(const _BIT_REF& right) { return operator=((bool)right); }
      BSS_FORCEINLINE operator bool() const { return (_bits&_bit) != 0; }
      inline void flip() { _bits ^= _bit; }
      BSS_FORCEINLINE bool operator!() const { return !operator bool(); }
      BSS_FORCEINLINE bool operator==(bool right) const { return right == operator bool(); }
      BSS_FORCEINLINE bool operator!=(bool right) const { return right != operator bool(); }
      BSS_FORCEINLINE bool operator&&(bool right) const { return right && operator bool(); }
      BSS_FORCEINLINE bool operator||(bool right) const { return right || operator bool(); }
      BSS_FORCEINLINE char operator|(char right) const { return right | (char)operator bool(); }
      BSS_FORCEINLINE char operator&(char right) const { return right & (char)operator bool(); }
      BSS_FORCEINLINE char operator^(char right) const { return right ^ (char)operator bool(); }
      BSS_FORCEINLINE std::pair<const T&, T> GetState() const { return std::pair<const T&, T>(_bits, _bit); }

    protected:
      T& _bits;
      T _bit;
    };

    template<typename T>
    struct BSS_COMPILER_DLLEXPORT _BIT_GROUP
    {
      inline _BIT_GROUP(T group, T& bits) : _group(group), _bits(bits) {}
      BSS_FORCEINLINE _BIT_GROUP& operator=(T right)
      {
        _bits = ((_bits&(~_group)) | (right&_group));
        return *this;
      }
      BSS_FORCEINLINE _BIT_GROUP& operator=(const _BIT_GROUP& right) { return operator=(right._bits&right._group); }
      BSS_FORCEINLINE operator T() const { return (_bits&_group); }
      BSS_FORCEINLINE bool operator!() const { return (_bits&_group) == 0; }
      BSS_FORCEINLINE bool operator==(T right) const { return right == (_bits&_group); }
      BSS_FORCEINLINE bool operator!=(T right) const { return right != (_bits&_group); }

    protected:
      T& _bits;
      T _group;
    };
  }

  // Generic implementation of using an integral type's component bits to store flags.
  template<typename T = uint32_t>
  class BSS_COMPILER_DLLEXPORT BitField
  {
  public:
    // Initializes the bitfield with the given flag values, if any
    inline explicit BitField(T bits = 0) : _bits(bits) {}
    // Sets the entire bitfield to the given value
    BSS_FORCEINLINE void Set(T bits) { _bits = bits; }
    BSS_FORCEINLINE void Set(T bits, T mask) { _bits ^= (bits ^ (_bits&mask)); }
    // Gets the entire bitfield
    BSS_FORCEINLINE T Get() const { return _bits; }
    BSS_FORCEINLINE bool Get(T bit) const { return (_bits&bit) != 0; }
    BSS_FORCEINLINE T GetGroup(T group) const { return _bits&group; }
    BSS_FORCEINLINE internal::_BIT_GROUP<T> GetGroup(T group) { return internal::_BIT_GROUP<T>(group, _bits); }

    inline BitField& operator=(T right) { _bits = right; return *this; }
    BSS_FORCEINLINE bool operator[](T bit) const { return Get(bit); }
    BSS_FORCEINLINE T operator()(T group) const { return _bits&group; }
    BSS_FORCEINLINE internal::_BIT_GROUP<T> operator()(T group) { return GetGroup(group); }
    BSS_FORCEINLINE internal::_BIT_REF<T> operator[](T bit) { return internal::_BIT_REF<T>(bit, _bits); }
    inline operator T() const { return _bits; }
    inline BitField& operator+=(T bit) { _bits |= bit; return *this; }
    inline BitField& operator-=(T bit) { _bits &= (~bit); return *this; }

  protected:
    T _bits;
  };
}

#endif
