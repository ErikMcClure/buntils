// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BITFIELD_H__BUN__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __BITFIELD_H__BUN__

#include "defines.h"
#include <utility>
#include <stdint.h>

namespace bun {
  namespace internal {
    template<typename T>
    struct BUN_COMPILER_DLLEXPORT _BIT_REF
    {
      inline constexpr _BIT_REF(const _BIT_REF& copy) = default;
      inline constexpr _BIT_REF(T bit, T& bits) : _bit(bit), _bits(bits) {}
      BUN_FORCEINLINE _BIT_REF& operator=(bool right)
      { 
        _bits = T_SETBIT(_bits, _bit, right, typename std::make_signed<T>::type); 
        return *this; 
      }
      BUN_FORCEINLINE _BIT_REF& operator=(const _BIT_REF& right) { return operator=((bool)right); }
      BUN_FORCEINLINE operator bool() const { return (_bits&_bit) != 0; }
      inline void flip() { _bits ^= _bit; }
      BUN_FORCEINLINE bool operator!() const { return !operator bool(); }
      BUN_FORCEINLINE bool operator==(bool right) const { return right == operator bool(); }
      BUN_FORCEINLINE bool operator!=(bool right) const { return right != operator bool(); }
      BUN_FORCEINLINE bool operator&&(bool right) const { return right && operator bool(); }
      BUN_FORCEINLINE bool operator||(bool right) const { return right || operator bool(); }
      BUN_FORCEINLINE char operator|(char right) const { return right | static_cast<char>(operator bool()); }
      BUN_FORCEINLINE char operator&(char right) const { return right & static_cast<char>(operator bool()); }
      BUN_FORCEINLINE char operator^(char right) const { return right ^ static_cast<char>(operator bool()); }
      BUN_FORCEINLINE std::pair<const T&, T> GetState() const { return std::pair<const T&, T>(_bits, _bit); }

      typedef T Ty;

    protected:
      T& _bits;
      T _bit;
    };

    template<typename T>
    struct BUN_COMPILER_DLLEXPORT _BIT_GROUP
    {
      inline constexpr _BIT_GROUP(const _BIT_GROUP& copy) = default;
      inline constexpr _BIT_GROUP(T group, T& bits) : _group(group), _bits(bits) {}
      BUN_FORCEINLINE _BIT_GROUP& operator=(T right)
      {
        _bits = ((_bits&(~_group)) | (right&_group));
        return *this;
      }
      BUN_FORCEINLINE _BIT_GROUP& operator=(const _BIT_GROUP& right) { return operator=(right._bits&right._group); }
      BUN_FORCEINLINE operator T() const { return (_bits&_group); }
      BUN_FORCEINLINE bool operator!() const { return (_bits&_group) == 0; }
      BUN_FORCEINLINE bool operator==(T right) const { return right == (_bits&_group); }
      BUN_FORCEINLINE bool operator!=(T right) const { return right != (_bits&_group); }

    protected:
      T& _bits;
      T _group;
    };

    template<typename U>
    class BUN_COMPILER_DLLEXPORT _BIT_ITER
    {
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = std::remove_const_t<U>;
      using difference_type = ptrdiff_t;
      using pointer = U*;
      using reference = U&;

      typedef typename U::Ty Ty;
    public:
      inline _BIT_ITER(const U& src) : _bits(const_cast<Ty*>(&src.GetState().first)), _bit(src.GetState().second) {}
      inline std::conditional_t<std::is_const_v<U>, bool, U> operator*() { return U(_bit, *_bits); }
      inline _BIT_ITER& operator++() { _incthis(); return *this; } //prefix
      inline _BIT_ITER operator++(int) { _BIT_ITER r(*this); ++*this; return r; } //postfix
      inline _BIT_ITER& operator--() { _decthis(); return *this; } //prefix
      inline _BIT_ITER operator--(int) { _BIT_ITER r(*this); --*this; return r; } //postfix
      inline bool operator==(const _BIT_ITER& _Right) const { return (_bits == _Right._bits) && (_bit == _Right._bit); }
      inline bool operator!=(const _BIT_ITER& _Right) const { return !operator==(_Right); }

    protected:
      void _incthis()
      {
        _bit = (_bit << 1);
        if(!_bit)
        {
          ++_bits;
          _bit = 1;
        }
      }
      void _decthis()
      {
        _bit = (_bit >> 1);
        if(!_bit)
        {
          --_bits;
          _bit = (1 << ((sizeof(Ty) << 3) - 1));
        }
      }

      Ty* _bits;
      Ty _bit;
    };
  }

  // Generic implementation of using an integral type's component bits to store flags.
  template<typename T = uint32_t>
  class BUN_COMPILER_DLLEXPORT BitField
  {
  public:
    // Initializes the bitfield with the given flag values, if any
    inline constexpr BitField(const BitField& copy) = default;
    inline constexpr explicit BitField(T bits = 0) : _bits(bits) {}
    // Sets the entire bitfield to the given value
    BUN_FORCEINLINE void Set(T bits) { _bits = bits; }
    BUN_FORCEINLINE void Set(T bits, T mask) { _bits ^= (bits ^ (_bits&mask)); }
    // Gets the entire bitfield
    BUN_FORCEINLINE T Get() const { return _bits; }
    BUN_FORCEINLINE bool Get(T bit) const { return (_bits&bit) != 0; }
    BUN_FORCEINLINE T GetGroup(T group) const { return _bits&group; }
    BUN_FORCEINLINE internal::_BIT_GROUP<T> GetGroup(T group) { return internal::_BIT_GROUP<T>(group, _bits); }

    inline BitField& operator=(T right) { _bits = right; return *this; }
    BUN_FORCEINLINE bool operator[](T bit) const { return Get(bit); }
    BUN_FORCEINLINE T operator()(T group) const { return _bits&group; }
    BUN_FORCEINLINE internal::_BIT_GROUP<T> operator()(T group) { return GetGroup(group); }
    BUN_FORCEINLINE internal::_BIT_REF<T> operator[](T bit) { return internal::_BIT_REF<T>(bit, _bits); }
    inline operator T() const { return _bits; }
    inline BitField& operator+=(T bit) { _bits |= bit; return *this; }
    inline BitField& operator-=(T bit) { _bits &= (~bit); return *this; }

  protected:
    T _bits;
  };
}

#endif
