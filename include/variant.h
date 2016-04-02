// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __VARIANT_H__BSS__
#define __VARIANT_H__BSS__

#include "bss_defines.h"
#include <type_traits>
#include <assert.h>

#ifdef BSS_VARIADIC_TEMPLATES

namespace bss_util {

	template<typename V, typename... Tx>
	struct _variant_op;

	template<typename V, typename T, typename... Tx>
	struct _variant_op<V, T, Tx...>
	{
		inline static void destroy(int tag, char* store)
		{
			if(tag == V::template pubgetpos<T>::value)
				reinterpret_cast<T*>(store)->~T();
			else
				_variant_op<V, Tx...>::destroy(tag, store);
		}
		template<class U>
		inline static void construct(int tag, char* store, const typename std::remove_reference<U>::type& v)
		{
			if(tag == V::template pubgetpos<T>::value)
				new(store) T(*reinterpret_cast<const T*>(v._store));
			else
				_variant_op<V, Tx...>::template construct<typename std::remove_reference<U>::type>(tag, store, v);
		}
		template<class U>
		inline static void construct(int tag, char* store, typename std::remove_reference<U>::type&& v)
		{
			int test = V::template pubgetpos<T>::value;
			if(tag == V::template pubgetpos<T>::value)
				new(store) T((T&&)(*reinterpret_cast<T*>(v._store)));
			else
				_variant_op<V, Tx...>::template construct<U>(tag, store, std::move(v));
		}
		template<class U>
		inline static void assign(int tag, char* store, const typename std::remove_reference<U>::type& v)
		{
			if(tag == V::template pubgetpos<T>::value)
				*reinterpret_cast<T*>(store) = *reinterpret_cast<const T*>(v._store);
			else
				_variant_op<V, Tx...>::template assign<typename std::remove_reference<U>::type>(tag, store, v);
		}
		template<class U>
		inline static void assign(int tag, char* store, typename std::remove_reference<U>::type&& v)
		{
			if(tag == V::template pubgetpos<T>::value)
				*reinterpret_cast<T*>(store) = (T&&)(*reinterpret_cast<const T*>(v._store));
			else
				_variant_op<V, Tx...>::template assign<U>(tag, store, std::move(v));
		}
		template<class U>
		inline static U convert(int tag, char* store)
		{
			if(tag == V::template pubgetpos<T>::value)
				return static_cast<U>(*reinterpret_cast<T*>(store));
			else
				return _variant_op<V, Tx...>::template convert<U>(tag, store);
		}
		template<class U>
		inline static U* convertP(int tag, char* store)
		{
			if(tag == V::template pubgetpos<T>::value)
				return static_cast<U*>(reinterpret_cast<T*>(store));
			else
				return _variant_op<V, Tx...>::template convertP<U>(tag, store);
		}
	};

	template<typename V>
	struct _variant_op<V>
	{
		inline static void destroy(int tag, char* store) { assert(tag == -1); }
		template<class U>
		inline static void construct(int tag, char* store, const U& v) { assert(tag == -1); }
		template<class U>
		inline static void construct(int tag, char* store, U && v) { assert(tag == -1); }
		template<class U>
		inline static void assign(int tag, char* store, const U& v) { assert(false); }
		template<class U>
		inline static void assign(int tag, char* store, U && v) { assert(false); }
		template<class U>
		inline static U convert(int tag, char* store) { assert(false); return *reinterpret_cast<U*>(store); }
		template<class U>
		inline static U* convertP(int tag, char* store) { assert(false); return reinterpret_cast<U*>(store); }
	};

  // algebriac union using variadic templates
  template<typename Arg, typename... Args>
  class BSS_COMPILER_DLLEXPORT variant
  {
    typedef variant<Arg, Args...> SELF;

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

    template<typename T, typename U>
    struct resolve
    {
      inline static void construct(variant& src, U && v)
      { 
        static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in variant");
        src._tag = getpos<T, Arg, Args...>::value;
        new(src._store) T(std::forward<U>(v));
      }
      inline static void assign(variant& src, U && v)
      {
        static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in variant");
        if(src._tag == getpos<T, Arg, Args...>::value)
          *reinterpret_cast<T*>(src._store) = std::forward<U>(v);
        else
        {
          src._destruct();
          src._construct<U>(std::forward<U>(v));
        }
      }
    };

    template<typename U>
    struct resolve<variant, U>
    {
      inline static void construct(variant& src, U && v) { src._tag = v._tag; _variant_op<SELF, Arg, Args...>::template construct<U>(src._tag, src._store, std::forward<U>(v)); }
      inline static void assign(variant& src, U && v) { 
        if(src._tag == v._tag)
          _variant_op<SELF, Arg, Args...>::template assign<U>(src._tag, src._store, std::forward<U>(v));
        else
        {
          src._destruct();
          src._tag = v._tag;
          _variant_op<SELF, Arg, Args...>::template construct<U>(src._tag, src._store, std::forward<U>(v));
        }
      }
    };

  public:
    variant() : _tag(-1) {}
    variant(const variant& v) : _tag(v._tag) { _variant_op<SELF, Arg, Args...>::template construct<variant>(_tag, _store, v); }
    variant(variant&& v) : _tag(v._tag) { _variant_op<SELF, Arg, Args...>::template construct<variant>(_tag, _store, std::move(v)); }
    template<typename T>
    explicit variant(const T& t) { _construct(t); }
    template<typename T>
    explicit variant(T&& t) { _construct(std::forward<T>(t)); }
    ~variant() { _destruct(); }
    variant& operator=(const variant& right) { _assign(right); return *this; }
    variant& operator=(variant&& right) { _assign(std::move(right)); return *this; }
    template<typename T>
    variant& operator=(const T& right) { _assign(right); return *this; }
    template<typename T>
    variant& operator=(T&& right) { _assign(std::forward<T>(right)); return *this; }

    template<typename T>
    T& get()
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in variant");
      assert((getpos<T, Arg, Args...>::value == _tag));
      return *reinterpret_cast<T*>(_store);
    }
    template<typename T>
    const T& get() const
    {
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in variant");
      assert((getpos<T, Arg, Args...>::value == _tag));
      return *reinterpret_cast<const T*>(_store);
    }
    template<typename T>
    const T convert() const { return _variant_op<SELF, Arg, Args...>::template convert<T>(_tag, _store); }
    template<typename T>
    T convert() { return _variant_op<SELF, Arg, Args...>::template convert<T>(_tag, _store); }
    template<typename T>
    T* convertP() { return _variant_op<SELF, Arg, Args...>::template convertP<T>(_tag, _store); }
    template<typename T>
    inline static bool contains() { return getpos<T, Arg, Args...>::value != -1; }
    template<typename T>
    inline bool is() const { return getpos<T, Arg, Args...>::value == _tag; }
    template<typename T>
    inline void typeset()
    { 
      static_assert(getpos<T, Arg, Args...>::value != -1, "Type does not exist in variant");
      _destruct();
      _tag = getpos<T, Arg, Args...>::value;
      new(_store) T();
    }
    inline int tag() const { return _tag; }

    template<typename T>
    static int type() { return getpos<T, Arg, Args...>::value; }

    template<typename T>
    struct Type { static constexpr int value = getpos<T, Arg, Args...>::value; };

    template<typename T>
    struct pubgetpos : getpos<T, Arg, Args...> { };

  protected:
    template<typename U>
    inline void _construct(U && v) { resolve<typename std::remove_const<typename std::remove_reference<U>::type>::type, U>::construct(*this, std::forward<U>(v)); }
    template<typename U>
    inline void _assign(U && v) { resolve<typename std::remove_const<typename std::remove_reference<U>::type>::type, U>::assign(*this, std::forward<U>(v)); }
    inline void _destruct() { _variant_op<SELF, Arg, Args...>::destroy(_tag, _store); }

    int _tag;
    union
    {
      char _store[max_sizeof<Arg, Args...>::value]; // actual storage
      typename max_alignof<Arg, Args...>::type _align; // simply ensures we have the correct alignment for the type with the largest alignment
    };

    template<typename V, typename... Tx>
    friend struct _variant_op;
  };
}

#endif
#endif
