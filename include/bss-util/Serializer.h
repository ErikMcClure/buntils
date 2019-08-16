// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __SERIALIZER_H__BSS__
#define __SERIALIZER_H__BSS__

#include <iostream>
#include <tuple>
#include <array>
#include <vector>
#include <type_traits>
#include "Trie.h"
#include "Variant.h"
#include "BitField.h"
#include "Str.h"

namespace std {
  template<class... X> // Override std::end for tuples so retrieving the last element of the fake tuple "array" is syntactically valid
  constexpr auto end(std::tuple<X...>& c) -> decltype(&std::get<sizeof...(X)-1>(c)) { return &std::get<sizeof...(X)-1>(c); }
}

namespace bss {
  template<class T>
  std::pair<const char*, T&> GenPair(const char* l, T& r) { return std::pair<const char*, T&>(l, r); }

  namespace internal {
    namespace serializer {
      template<class T>
      struct PushValue
      {
        inline PushValue(PushValue&&) = delete;
        inline PushValue(const PushValue&) = delete;
        inline PushValue(T& src, T nvalue) noexcept : _value(src), _src(src) { src = nvalue; }
        inline ~PushValue() { _src = _value; }

      private:
        T _value;
        T& _src;
      };

      template<class E>
      struct ConvRef
      {
        template<class T, class BASE>
        struct Value
        {
          inline static constexpr T&& f(typename std::remove_reference<T>::type& r) { return (static_cast<T&&>(r)); }
          inline static constexpr T&& f(typename std::remove_reference<T>::type&& r) { return (static_cast<T&&>(r)); }
        };
        template<class BASE>
        struct Value<E, BASE>
        {
          inline static constexpr BASE&& f(typename std::remove_reference<E>::type& r) { return (static_cast<BASE&&>(r)); }
          inline static constexpr BASE&& f(typename std::remove_reference<E>::type&& r) { return (static_cast<BASE&&>(r)); }
        };
        template<class BASE>
        struct Value<const E, BASE>
        {
          inline static constexpr const BASE&& f(typename std::remove_reference<const E>::type& r) { return (static_cast<const BASE&&>(r)); }
          inline static constexpr const BASE&& f(typename std::remove_reference<const E>::type&& r) { return (static_cast<const BASE&&>(r)); }
        };
      };

      template<class, class = void> struct is_serializer_array : std::false_type {};
      template<class T> struct is_serializer_array<T, std::void_t<typename T::SerializerArray>> : std::bool_constant<!std::is_void_v<typename T::SerializerArray>> {};

      template<class E, class T, class V = void> struct is_serializable : std::false_type {};

      // Classifies types by action performed (value, integer, key-value pair, array)
      template<class Engine, class T>
      struct Action // Arbitrary value
      {
        static inline void Parse(Serializer<Engine>& e, T& obj, const char* id)
        {
          if constexpr(is_serializer_array<T>::value)
          {
            static_assert(is_serializable<Engine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
            obj.template Serialize<Engine>(e, id);
          }
          else if constexpr(std::is_same<bool, T>::value)
            Engine::ParseBool(e, obj, id);
          else if constexpr(std::is_arithmetic<T>::value)
            Engine::template ParseNumber<T>(e, obj, id);
          else if constexpr(std::is_enum<T>::value)
            Engine::template ParseNumber<typename make_integral<T>::type>(e, reinterpret_cast<typename make_integral<T>::type&>(obj), id);
          else
            Engine::template Parse<T>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const T& obj, const char* id)
        {
          if constexpr(is_serializer_array<T>::value)
          {
            static_assert(is_serializable<Engine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
            const_cast<T&>(obj).template Serialize<Engine>(e, id);
          }
          else if constexpr(std::is_same<bool, T>::value)
            Engine::SerializeBool(e, obj, id);
          else if constexpr(std::is_arithmetic<T>::value)
            Engine::template SerializeNumber<T>(e, obj, id);
          else if constexpr(std::is_enum<T>::value)
            Engine::template SerializeNumber<typename make_integral<T>::type>(e, reinterpret_cast<const typename make_integral<T>::type&>(obj), id);
          else
            Engine::template Serialize<T>(e, obj, id);
        }
      };

      template<class T>
      inline T FromString(const char* id)
      {
        if constexpr(std::is_arithmetic_v<T> || std::is_enum_v<T>)
        {
          std::istringstream ss(id);
          typename make_integral<T>::type key;
          ss >> key;
          return T(key);
        }
        else
          return T(id);
      }

      template<class Engine, class STORE>
      struct Action<Engine, internal::_BIT_REF<STORE>> // Bit reference used in some classes
      {
        static inline void Parse(Serializer<Engine>& e, internal::_BIT_REF<STORE>& obj, const char* id) { bool b = obj; Engine::ParseBool(e, b, id); obj = b; }
        static inline void Serialize(Serializer<Engine>& e, internal::_BIT_REF<STORE> obj, const char* id) { Engine::SerializeBool(e, obj, id); }
      };

      template<class Engine, class T, size_t I> // For fixed-length arrays
      struct Action<Engine, T[I]>
      {
        static inline void Parse(Serializer<Engine>& e, T(&obj)[I], const char* id) { Engine::template ParseArray<T[I], T, &Serializer<Engine>::template FixedAdd<T[I]>, &Serializer<Engine>::template FixedRead<T[I]>>(e, obj, id); }
        static inline void Serialize(Serializer<Engine>& e, const T(&obj)[I], const char* id) { Engine::template SerializeArray<T[I]>(e, obj, I, id); }
      };

      template<class Engine, class T, size_t I> // For fixed-length arrays
      struct Action<Engine, std::array<T, I>>
      {
        static inline void Parse(Serializer<Engine>& e, std::array<T, I>& obj, const char* id) { Engine::template ParseArray<std::array<T, I>, T, &Serializer<Engine>::template FixedAdd<std::array<T, I>>, &Serializer<Engine>::template FixedRead<std::array<T, I>>>(e, obj, id); }
        static inline void Serialize(Serializer<Engine>& e, const std::array<T, I>& obj, const char* id) { Engine::template SerializeArray<std::array<T, I>>(e, obj, I, id); }
      };

      template<class Engine, class T, typename Alloc>
      struct Action<Engine, std::vector<T, Alloc>>
      {
        static inline void Add(Serializer<Engine>& e, std::vector<T, Alloc>& obj, int& n)
        {
          obj.push_back(T());
          Action<Engine, T>::Parse(e, obj.back(), 0);
        }
        static inline bool Read(Serializer<Engine>& e, std::vector<T, Alloc>& obj, int64_t count)
        {
          obj.resize(count);
          return e.BulkRead(std::begin(obj), std::end(obj), count);
        }
        static inline void Parse(Serializer<Engine>& e, std::vector<T, Alloc>& obj, const char* id)
        {
          Engine::template ParseArray<std::vector<T, Alloc>, T, &Add, &Read>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const std::vector<T, Alloc>& obj, const char* id)
        {
          Engine::template SerializeArray<std::vector<T, Alloc>>(e, obj, obj.size(), id);
        }
      };

      template<class Engine, class... Args>
      struct Action<Engine, std::tuple<Args...>>
      {
        template<int I>
        static inline void _add(Serializer<Engine>& e, std::tuple<Args...>& obj, int n)
        {
          if(I == n)
            return Action<Engine, std::tuple_element_t<I, std::tuple<Args...>>>::Parse(e, std::get<I>(obj), 0);
          if constexpr(I > 0)
            _add<I - 1>(e, obj, n);
        }
        static inline void Add(Serializer<Engine>& e, std::tuple<Args...>& obj, int& n) { _add<sizeof...(Args)-1>(e, obj, n++); }
        static inline bool Read(Serializer<Engine>& e, std::tuple<Args...>& obj, int64_t count) { return false; }
        static inline void Parse(Serializer<Engine>& e, std::tuple<Args...>& obj, const char* id)
        {
          Engine::template ParseArray<std::tuple<Args...>, std::tuple_element_t<sizeof...(Args)-1, std::tuple<Args...>>, &Add, &Read>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const std::tuple<Args...>& obj, const char* id)
        {
          Engine::template SerializeTuple<std::tuple<Args...>>(e, obj, id, std::index_sequence_for<Args...>{});
        }
      };

      template<typename... Args>
      struct ActionVariantRef
      {
        ActionVariantRef(Variant<Args...>& ref) : _ref(ref) {}

        Variant<Args...>& _ref;
      };

      template<class Engine, typename... Args>
      struct Action<Engine, ActionVariantRef<Args...>>
      {
        template<typename A, typename... Ax>
        static inline void _parse(Serializer<Engine>& e, Variant<Args...>& obj, const char* id)
        {
          if(obj.template is<A>())
          {
            new (&obj) Variant<Args...>(); // Set _Tag to -1
            obj.template typeset<A>(); // Properly construct type
            Action<Engine, A>::Parse(e, obj.template get<A>(), id);
          }
          else if constexpr(sizeof...(Ax) > 0)
            _parse<Ax...>(e, obj, id);
        }
        template<typename A, typename... Ax>
        static inline void _serialize(Serializer<Engine>& e, const Variant<Args...>& obj, const char* id)
        {
          if(obj.template is<A>())
            Action<Engine, A>::Serialize(e, obj.template get<A>(), id);
          else if constexpr(sizeof...(Ax) > 0)
            _serialize<Ax...>(e, obj, id);
          else
            assert(obj.tag() == -1);
        }

        static inline void Parse(Serializer<Engine>& e, ActionVariantRef<Args...>& obj, const char* id)
        {
          _parse<Args...>(e, obj._ref, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const ActionVariantRef<Args...>& obj, const char* id)
        {
          _serialize<Args...>(e, obj._ref, id);
        }
      };
    }
  }

  // Defines a universal serializer using an arbitrary serialization engine for types that implement `template<typename Engine> void Serialize(Serializer<Engine>& e)`
  template<class Engine>
  class Serializer
  {
  public:
    Serializer() {}
    explicit Serializer(std::ostream& s) : out(&s), in(0) {}
    explicit Serializer(std::istream& s) : out(0), in(&s) {}
    ~Serializer() { if(out) out->flush(); }
    Engine engine;
    std::ostream* out;
    std::istream* in;

    template<class T> using ActionBind = internal::serializer::Action<Engine, T>;

    template<typename T>
    void Serialize(const T& obj, std::ostream& s, const char* name = 0)
    {
      out = &s;
      in = 0;
      Engine::Begin(*this);
      ActionBind<T>::Serialize(*this, obj, name);
      Engine::End(*this);
    }

    template<typename T>
    void Parse(T& obj, std::istream& s, const char* name = 0)
    {
      out = 0;
      in = &s;
      Engine::Begin(*this);
      ActionBind<T>::Parse(*this, obj, name);
      Engine::End(*this);
    }

    template<typename T, typename... Args>
    inline void EvaluateType(std::pair<const char*, Args&>... args)
    {
      static Trie<uint16_t> trie(sizeof...(Args), (args.first)...);

      if(out) // Serializing
        (ActionBind<Args>::Serialize(*this, args.second, args.first), ...);

      if(in) // Parsing
      {
        if(Engine::Ordered())
          (ActionBind<Args>::Parse(*this, args.second, args.first), ...);
        else
        {
          auto tmp = std::make_tuple<std::pair<const char*, Args&>...>(std::move(args)...);
          Engine::template ParseMany(*this, [&](Serializer<Engine>& e, const char* id) { Serializer<Engine>::template FindParse<Args...>(e, id, trie, tmp); });
        }
      }
    }

    template<typename T, typename F> // void(Serializer<Engine>& e, const char* id)
    inline void EvaluateKeyValue(T& obj, F && insert)
    {
      if(out) // Serializing
      {
        auto end = std::end(obj);
        for(auto begin = std::begin(obj); begin != end; ++begin)
          ActionBind<remove_cvref_t<std::tuple_element_t<1, remove_cvref_t<decltype(*begin)>>>>::Serialize(*this, std::get<1>(*begin), ToString(std::get<0>(*begin)));
      }

      if(in)
        Engine::template ParseMany(*this, std::forward<F>(insert));
    }

    template<typename T, typename E, void(*Add)(Serializer<Engine>& e, T& obj, int& n), class C, void (T::*SET)(C)>
    inline void EvaluateArray(T& obj, size_t length, const char* id)
    {
      if(out) // Serializing
        Engine::template SerializeArray<T>(*this, obj, length, id);

      if(in)
      {
        if(SET == nullptr) // Due to a bug in GCC, it doesn't think this is a constant expression
          Engine::template ParseArray<T, E, Add, &DisabledRead<T>>(*this, obj, id);
        else
          Engine::template ParseArray<T, E, Add, &DynamicRead<T, C, SET>>(*this, obj, id);
      }
    }

    template<typename T, int I>
    BSS_FORCEINLINE void EvaluateFixedArray(T(&obj)[I], const char* id)
    {
      if(out) // Serializing
        Engine::template SerializeArray<T[I]>(*this, obj, I, id);

      if(in)
        Engine::template ParseArray<T[I], T, &FixedAdd<T[I]>, &FixedRead<T[I]>>(*this, obj, id);
    }

    template<int I, typename... Args>
    struct _findparse {
      inline static void F(Serializer<Engine>& e, uint16_t index, const std::tuple<std::pair<const char*, Args&>...>& args) {
        if (index == I)
          ActionBind<std::remove_reference_t<typename std::tuple_element_t<I, std::tuple<std::pair<const char*, Args&>...>>::second_type>>::Parse(e, std::get<I>(args).second, std::get<I>(args).first);
        else
          _findparse<I - 1, Args...>::F(e, index, args);
      }
    };

    template<typename... Args>
    struct _findparse<0, Args...> {
      inline static void F(Serializer<Engine>& e, uint16_t index, const std::tuple<std::pair<const char*, Args&>...>& args) {
        if (index == 0)
          ActionBind<std::remove_reference_t<typename std::tuple_element_t<0, std::tuple<std::pair<const char*, Args&>...>>::second_type>>::Parse(e, std::get<0>(args).second, std::get<0>(args).first);
      }
    };

    /*template<int I, typename... Args>
    inline static void r_findparse(Serializer<Engine>& e, uint16_t index, const std::tuple<std::pair<const char*, Args&>...>& args)
    {
      if(index == I)
        return ActionBind<std::remove_reference_t<typename std::tuple_element_t<I, std::tuple<std::pair<const char*, Args&>...>>::second_type>>::Parse(e, std::get<I>(args).second, std::get<I>(args).first);
      if constexpr(I > 0)
        r_findparse<I - 1, Args...>(e, index, args);
    }*/
    template<typename... Args> // This function must be static due to some corner cases on certain parsers
    inline static void FindParse(Serializer<Engine>& e, const char* key, const Trie<uint16_t>& trie, const std::tuple<std::pair<const char*, Args&>...>& args)
    {
      _findparse<sizeof...(Args)-1, Args...>::F(e, trie[key], args);
      //r_findparse<sizeof...(Args)-1, Args...>(e, trie[key], args);
    }

    template<class T>
    static inline void FixedAdd(Serializer<Engine>& e, T& obj, int& n) { if(n < (std::end(obj) - std::begin(obj))) Serializer<Engine>::template ActionBind<remove_cvref_t<decltype(obj[0])>>::Parse(e, obj[n++], 0); }

    template<class T>
    static inline bool FixedRead(Serializer<Engine>& e, T& obj, int64_t count) { return e.BulkRead(std::begin(obj), std::end(obj), count); }

    template<class T>
    static inline bool DisabledRead(Serializer<Engine>& e, T& obj, int64_t count) { return false; }

    template<class T, class C, void (T::*SET)(C)>
    static inline bool DynamicRead(Serializer<Engine>& e, T& obj, int64_t count)
    {
      (obj.*SET)(count);
      return e.BulkRead(std::begin(obj), std::end(obj), count);
    }

    template<class T>
    inline bool BulkRead(T && begin, T && end, int64_t count)
    {
      typedef remove_cvref_t<decltype(*begin)> Element;
      if constexpr(std::is_trivially_constructible_v<Element> && (std::is_base_of_v<std::random_access_iterator_tag, T> || std::is_array_v<T> || std::is_pointer_v<T>))
      {
        if(begin == end)
          return false;

        auto& ref = *begin;
        in->read(reinterpret_cast<char*>(&ref), bssmin(count, end - begin) * sizeof(Element));
        return true;
      }
      else
        return false;
    }

    template<class T>
    inline bool BulkWrite(T && begin, T && end, int64_t count)
    {
      typedef remove_cvref_t<decltype(*begin)> Element;
      if constexpr(std::is_trivially_constructible_v<Element> && (std::is_base_of_v<std::random_access_iterator_tag, T> || std::is_array_v<T> || std::is_pointer_v<T>))
      {
        if(begin != end)
        {
          auto& ref = *begin;
          out->write(reinterpret_cast<const char*>(&ref), bssmin(count, end - begin) * sizeof(Element));
        }
        return true;
      }
      else
        return false;
    }
  };

  namespace internal {
    namespace serializer { // This is used by Serializer, but std::declval requires a complete type, so we have to put the partial specialization below Serializer
      template<class E, class T> struct is_serializable<E, T, std::void_t<decltype(std::declval<T>().template Serialize<E>(std::declval<Serializer<E>&>(), ""))>> : std::true_type {};
    }
  }

  // Reference engine for serializers
  class EmptyEngine
  {
  public:
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<EmptyEngine>& e) {}
    static void End(Serializer<EmptyEngine>& e) {}
    template<typename T>
    static void Serialize(Serializer<EmptyEngine>& e, const T& obj, const char* id) {}
    template<typename T>
    static void SerializeArray(Serializer<EmptyEngine>& e, const T& obj, size_t size, const char* id) {}
    template<typename T, size_t... S> // OPTIONAL. Throw an error if not supported
    static void SerializeTuple(Serializer<EmptyEngine>& e, const T& obj, const char* id, std::index_sequence<S...>) { throw std::runtime_error("Serialization of tuples not supported!"); }
    template<typename T>
    static void SerializeNumber(Serializer<EmptyEngine>& e, T obj, const char* id) {}
    static void SerializeBool(Serializer<EmptyEngine>& e, bool obj, const char* id) {}
    template<typename T>
    static void Parse(Serializer<EmptyEngine>& e, T& obj, const char* id) {}
    template<typename T, typename E, void(*Add)(Serializer<EmptyEngine>& e, T& obj, int& n), bool(*Read)(Serializer<EmptyEngine>& e, T& obj, int64_t count)>
    static void ParseArray(Serializer<EmptyEngine>& e, T& obj, const char* id) {}
    template<typename T>
    static void ParseNumber(Serializer<EmptyEngine>& e, T& obj, const char* id) {}
    static void ParseBool(Serializer<EmptyEngine>& e, bool& obj, const char* id) {}
    template<typename F>
    static void ParseMany(Serializer<EmptyEngine>& e, F && f) { f(e, ""); }
  };
}

#endif
