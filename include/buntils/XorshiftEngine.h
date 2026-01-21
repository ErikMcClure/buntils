// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_XORSHIFT_ENGINE_H__
#define __BUN_XORSHIFT_ENGINE_H__

#include "compiler.h"
#include <limits.h>
#include <stdint.h>
#include <random>

namespace bun {
  // Implementation of an xorshift64star generator. x serves as the generator state, which should initially be set to the RNG seed.
  inline uint64_t xorshift64star(uint64_t& x)
  {
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * UINT64_C(2685821657736338717);
  }

  // Implementation of 2^1024-1 period xorshift generator. x is the 16*64 bit state, plus 1 extra integer for counting indices.
  inline uint64_t xorshift1024star(uint64_t(&x)[17])
  {
    uint64_t x0 = x[x[16]];
    uint64_t x1 = x[x[16] = (x[16] + 1) & 15];
    x1 ^= x1 << 31; // a
    x1 ^= x1 >> 11; // b
    x0 ^= x0 >> 30; // c
    return (x[x[16]] = x0 ^ x1) * UINT64_C(1181783497276652981);
  }

  // Generates a seed for xorshift1024star from a 64-bit value
  inline void GenXor1024Seed(uint64_t x, uint64_t(&seed)[17])
  {
    xorshift64star(x);
    for(uint8_t i = 0; i < 16; ++i)
      seed[i] = xorshift64star(x);
    seed[16] = 0;
  }

  namespace internal {
    template<typename T>
    class xorshift_engine_base
    {
    public:
      BUN_FORCEINLINE static constexpr T base_min() { return std::numeric_limits<T>::min(); }
      BUN_FORCEINLINE static constexpr T base_max() { return std::numeric_limits<T>::max(); }
      BUN_FORCEINLINE static constexpr T base_transform(uint64_t x) { return (T)x; }
    };

    // DISABLED: There is no nice way of mapping randomly generated values to floating point values because floating point has a log2 distribution and denormals.
    // Instead, use gencanonical to map generated random integers to a [0.0,1.0) range.
    /*template<>
    class xorshift_engine_base<float>
    {
    public:
    BUN_FORCEINLINE static float base_min() { return base_transform(0xFFFFFFFFFFFFFFFF); }
    BUN_FORCEINLINE static float base_max() { return base_transform(0x7FFFFFFFFFFFFFFF); }
    BUN_FORCEINLINE static float base_transform(uint64_t x)
    {
    uint32_t y=(uint32_t)x;
    y = (y&0xBFFFFFFF)+0x1F800000; // Mask out the top exponent bit to force exponent to 0-127 range, then add 63 to the exponent to get it in [63,190] range ([-64,63] when biased)
    return *(float*)(&y); // convert our integer into a float, assuming IEEE format
    }
    };

    template<>
    class BUN_COMPILER_DLLEXPORT xorshift_engine_base<double>
    {
    public:
    BUN_FORCEINLINE static double base_min() { return base_transform(0xFFFFFFFFFFFFFFFF); }
    BUN_FORCEINLINE static double base_max() { return base_transform(0x7FFFFFFFFFFFFFFF); }
    BUN_FORCEINLINE static double base_transform(uint64_t x)
    {
    x = (x&0xBFFFFFFFFFFFFFFF)+0x1FF0000000000000; // Mask out the top exponent bit to force exponent to 0-1023 range, then add 511 to the exponent to get it in [511,1534] range ([-512,511] when biased)
    return *(double*)(&x); // convert our integer into a double, assuming IEEE format
    }
    };*/
  }

  template<typename T = uint64_t>
  class BUN_COMPILER_DLLEXPORT XorshiftEngine : protected internal::xorshift_engine_base<T>
  {
  public:
    XorshiftEngine() { seed(); }
    explicit XorshiftEngine(uint64_t s) { seed(s); }
    explicit XorshiftEngine(uint64_t s[16]) { seed(s); }
    void seed()
    {
      std::random_device rd;
      GenXor1024Seed(rd(), _state);
    }
    void seed(uint64_t s) { GenXor1024Seed(s, _state); }
    void seed(uint64_t s[16])
    {
      for(int i = 0; i < 16; ++i)
        _state[i] = s[i];
      _state[16] = 0;
    }
    void discard(unsigned long long z)
    {
      for(int i = 0; i < z; ++i)
        xorshift1024star(_state);
    }

    inline constexpr static T min() { return internal::xorshift_engine_base<T>::base_min(); }
    inline constexpr static T max() { return internal::xorshift_engine_base<T>::base_max(); }

    inline constexpr T operator()() { return internal::xorshift_engine_base<T>::base_transform(xorshift1024star(_state)); } // Truncate to return_value size.
    bool operator ==(const XorshiftEngine& r) const
    {
      for(int i = 0; i < 17; ++i)
        if(_state[i] != r._state[i])
          return false;
      return true;
    }
    bool operator !=(const XorshiftEngine& r) const { return !operator==(r); }

    using result_type = T;

  protected:
    uint64_t _state[17];
  };

  inline uint64_t XorshiftRand(uint64_t seed = 0)
  {
    static uint64_t state[17];
    if(seed) GenXor1024Seed(seed, state);
    return xorshift1024star(state);
  }
  typedef XorshiftEngine<uint64_t> XorshiftEngine64;
}

#endif