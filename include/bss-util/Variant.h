// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __VARIANT_H__BSS__
#define __VARIANT_H__BSS__

#include "compiler.h"
#include <type_traits>
#include <assert.h>
#include <stddef.h>
#include <utility>

namespace bss {
  template<class Engine>
  class Serializer;

  namespace internal {
    namespace serializer {
      template<typename... Args>
      struct ActionVariantRef;
    }
  }

  // algebriac union using variadic templates
  template<typename Arg, typename... Args>
  class BSS_COMPILER_DLLEXPORT Variant
  {
    typedef Variant<Arg, Args...> SELF;

    // Compile-time max sizeof
    template<typename A, typename... Ax>
    struct max_sizeof { static constexpr size_t value = (max_sizeof<Ax...>::value > sizeof(A)) ? max_sizeof<Ax...>::value : sizeof(A); };

    template<typename A>
    struct max_sizeof<A> { static constexpr size_t value = sizeof(A); };

    // Compile-time max alignment
    template<typename A, typename... Ax>
    struct max_alignof {
      static constexpr size_t value = (max_alignof<Ax...>::value > alignof(A)) ? max_alignof<Ax...>::value : alignof(A);
      typedef std::conditional<(max_alignof<Ax...>::value > alignof(A)), A, typename max_alignof<Ax...>::type> type;
    };

    template<typename A>
    struct max_alignof<A> { static constexpr size_t value = alignof(A); typedef A type; };

    template<typename X, typename... Ts> struct getpos;

    template<typename X>
    struct getpos<X> { static const int value = -1; };

    template<typename X, typename... Ts>
    struct getpos<X, X, Ts...> { static const int value = 0; };

    template<typename X, typename T, typename... Ts>
    struct getpos<X, T, Ts...> { static const int value = getpos<X, Ts...>::value != -1 ? getpos<X, Ts...>::value + 1 : -1; };

  public:
    Variant() : _tag(-1) {}
    Variant(const Variant& v) : _tag(v._tag) { _constructU<Variant, Arg, Args...>(v); }
    Variant(Variant&& v) : _tag(v._tag) { _constructU<Variant, Arg, Args...>(std::move(v)); }
    template<typename T>
    explicit Variant(const T& v) { _construct(v); }
    template<typename T>
    explicit Variant(T&& v) { _construct(std::forward<T>(v)); }
    ~Variant() { _destruct(); }
    Variant& operator=(const Variant& right) { _assign(right); return *this; }
    Variant& operator=(Variant&& right) { _assign(std::move(right)); return *this; }
    template<typename T>
    Variant& operator=(const T& right) { _assign(right); return *this; }
    template<typename T>
    Variant& operator=(T&& right) { _assign(std::forward<T>(right)); return *this; }

    template<typename T>
    T& get()
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in Variant");
      assert((getpos<T, Arg, Args...>::value == _tag));
      return *reinterpret_cast<T*>(_store);
    }
    template<typename T>
    const T& get() const
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in Variant");
      assert((getpos<T, Arg, Args...>::value == _tag));
      return *reinterpret_cast<const T*>(_store);
    }
    template<typename T>
    const T convert() const { return _convert<T, Arg, Args...>(); }
    template<typename T>
    T convert() { return _convert<T, Arg, Args...>(); }
    template<typename T>
    T* convertP() { return _convertP<T, Arg, Args...>(); }
    template<typename T>
    inline static bool contains() { return getpos<T, Arg, Args...>::value != -1; }
    template<typename T>
    inline bool is() const { return getpos<T, Arg, Args...>::value == _tag; }
    template<typename T>
    inline void typeset()
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in Variant");
      _destruct();
      _tag = getpos<T, Arg, Args...>::value;
      new(_store) T();
    }
    inline int tag() const { return _tag; }

    template<typename T>
    static int type() { return getpos<T, Arg, Args...>::value; }

    template<typename T>
    struct Type { static constexpr int value = getpos<T, Arg, Args...>::value; };

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      internal::serializer::ActionVariantRef<Arg, Args...> ref(*this);
      e.template EvaluateType<Variant>(std::pair<const char*, int&>("t", _tag), std::pair<const char*, internal::serializer::ActionVariantRef<Arg, Args...>&>("o", ref));
    }

  protected:
    template<typename U>
    inline void _construct(U && v)
    { 
      typedef remove_cvref_t<U> T;
      if constexpr(std::is_same<T, Variant>::value)
      {
        _tag = v._tag;
        _constructU<U, Arg, Args...>(std::forward<U>(v));
      }
      else
      {
        static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in Variant");
        _tag = getpos<T, Arg, Args...>::value;
        new(_store) T(std::forward<U>(v));
      }
    }

    template<typename U>
    inline void _assign(U && v)
    { 
      typedef remove_cvref_t<U> T;
      if constexpr(std::is_same<T, Variant>::value)
      {
        if(_tag == v._tag)
          _assignU<U, Arg, Args...>(std::forward<U>(v));
        else
        {
          _destruct();
          _tag = v._tag;
          _constructU<U, Arg, Args...>(std::forward<U>(v));
        }
      }
      else
      {
        static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in Variant");

        if(_tag == getpos<T, Arg, Args...>::value)
          *reinterpret_cast<T*>(_store) = std::forward<U>(v);
        else
        {
          _destruct();
          _construct<U>(std::forward<U>(v));
        }
      }
    }

    inline void _destruct() { _destroy<Arg, Args...>(); }
    template<typename T, typename... Tx>
    inline void _destroy()
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "impossible result in destroy");
      if(_tag == getpos<T, Arg, Args...>::value)
        reinterpret_cast<T*>(_store)->~T();
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          _destroy<Tx...>();
        else
          assert(_tag == -1);
      }
    }
    template<class U, typename T, typename... Tx>
    inline void _constructU(const typename std::remove_reference<U>::type& v)
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        new(_store) T(*reinterpret_cast<const T*>(v._store));
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          _constructU<typename std::remove_reference<U>::type, Tx...>(v);
        else
          assert(_tag == -1);
      }
    }
    template<class U, typename T, typename... Tx>
    inline void _constructU(typename std::remove_reference<U>::type&& v)
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        new(_store) T((T&&)(*reinterpret_cast<T*>(v._store)));
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          _constructU<U, Tx...>(std::move(v));
        else
          assert(_tag == -1);
      }
    }
    template<class U, typename T, typename... Tx>
    inline void _assignU(const typename std::remove_reference<U>::type& v)
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        *reinterpret_cast<T*>(_store) = *reinterpret_cast<const T*>(v._store);
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          _assignU<typename std::remove_reference<U>::type, Tx...>(v);
        else
          assert(false);
      }
    }
    template<class U, typename T, typename... Tx>
    inline void _assignU(typename std::remove_reference<U>::type&& v)
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        *reinterpret_cast<T*>(_store) = (T&&)(*reinterpret_cast<const T*>(v._store));
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          _assignU<U, Tx...>(std::move(v));
        else
          assert(false);
      }
    }
    template<class U, typename T, typename... Tx>
    inline U _convert()
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        return static_cast<U>(*reinterpret_cast<T*>(_store));
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          return _convert<U, Tx...>();
        else
          assert(false);
        return *reinterpret_cast<U*>(_store);
      }
    }
    template<class U, typename T, typename... Tx>
    inline U* _convertP()
    {
      if(_tag == getpos<T, Arg, Args...>::value)
        return static_cast<U*>(reinterpret_cast<T*>(_store));
      else
      {
        if constexpr(sizeof...(Tx) > 0)
          return _convertP<U, Tx...>();
        else
          assert(false);
        return reinterpret_cast<U*>(_store);
      }
    }

    int _tag;
    union
    {
      char _store[max_sizeof<Arg, Args...>::value]; // actual storage
      typename max_alignof<Arg, Args...>::type _align; // simply ensures we have the correct alignment for the type with the largest alignment
    };
  };
}

#endif
