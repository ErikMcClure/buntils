// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __SERIALIZER_H__BSS__
#define __SERIALIZER_H__BSS__

#include <iostream>
#include <tuple>
#include <array>
#include <vector>
#include "Trie.h"
#include "Variant.h"
#include "Hash.h"

namespace bss {
  //DEFINE_MEMBER_CHECKER(Serialize); // doesn't seem to work for template functions

  template<class T>
  std::pair<const char*, T&> GenPair(const char* l, T& r) { return std::pair<const char*, T&>(l, r); }
  
  template<class Engine>
  class Serializer;

  namespace internal {
    namespace serializer {
      template<class T>
      struct Swapper
      {
        explicit Swapper(T v) : value(v) {}
        T Set(T v) { T old = value; value = v; return old; }
        T value;
      };

      template<class E>
      struct ConvRef {
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

      // Classifies types by action performed (value, integer, key-value pair, array)
      template<class Engine, class T, bool B>
      struct Action // Arbitrary value
      {
        static inline void Parse(Serializer<Engine>& e, T& t, const char* id) { Engine::template Parse<T>(e, t, id); }
        static inline void Serialize(Serializer<Engine>& e, const T& t, const char* id) { Engine::template Serialize<T>(e, t, id); }
      };

      template<class Engine, typename... Args>
      struct KeyValueFind
      {
        KeyValueFind(const Trie<uint16_t>& t, const std::tuple<Args...>& args) : _t(t), _args(args) {}
        BSS_FORCEINLINE void operator()(Serializer<Engine>& e, const char* id) { Serializer<Engine>::template FindParse<Args...>(e, id, _t, _args); }

      protected:
        const Trie<uint16_t>& _t;
        const std::tuple<Args...>& _args;
      };

      template<class Engine, class Key, class Value>
      struct KeyValueArray
      {
        typedef DynArray<std::pair<Key, Value>, size_t, ARRAY_SAFE> T;
        KeyValueArray(T& obj) : _obj(obj) {}
        BSS_FORCEINLINE void operator()(Serializer<Engine>& e, const char* id)
        {
          std::pair<Key, Value> pair;
          pair.first = id;
          Action<Engine, Value, std::is_arithmetic<Value>::value>::Parse(e, pair.second, id);
          _obj.Add(pair);
        }

      protected:
        T& _obj;
      };

      template<class T, bool B>
      struct Extract {
        inline static typename T::DATA* Get(T& obj, const char* id) { return obj.PointerValue(obj.Iterator(id)); }
        inline static typename T::DATA* Insert(T& obj, const char* id) { return obj.PointerValue(obj.Insert(id, typename T::DATA())); }
      };
      template<class T>
      struct Extract<T, true> { // If Key is arithmetic, convert to an integer from the string
        inline static typename T::KEY GetKey(const char* id) {
          std::istringstream ss(id);
          typename T::KEY key;
          ss >> key;
          return key;
        }
        inline static typename T::DATA* Get(T& obj, const char* id) { return obj.PointerValue(obj.Iterator(GetKey(id))); }
        inline static typename T::DATA* Insert(T& obj, const char* id) { return obj.PointerValue(obj.Insert(GetKey(id), typename T::DATA())); }
      };

      template<class Engine, class Key, class Data, khint_t(*__hash_func)(const Key&), bool(*__hash_equal)(const Key&, const Key&), ARRAY_TYPE ArrayType, typename Alloc>
      struct KeyValueInsert
      {
        typedef HashBase<Key, Data, true, __hash_func, __hash_equal, ArrayType, Alloc> T;

        KeyValueInsert(T& obj) : _obj(obj) {}
        BSS_FORCEINLINE void operator()(Serializer<Engine>& e, const char* id)
        {
          Data* v = Extract<T, std::is_arithmetic<Key>::value>::Get(_obj, id);
          if(!v)
            v = Extract<T, std::is_arithmetic<Key>::value>::Insert(_obj, id);
          if(v)
            Action<Engine, Data, std::is_arithmetic<Data>::value>::Parse(e, *v, id);
        }

      protected:
        T& _obj;
      };

      template<class Engine, class T>
      struct Action<Engine, T, true> // Integer or float value (seperated to simplify template overloads)
      {
        static inline void Parse(Serializer<Engine>& e, T& t, const char* id) { Engine::template ParseNumber<T>(e, t, id); }
        static inline void Serialize(Serializer<Engine>& e, T t, const char* id) { Engine::template SerializeNumber<T>(e, t, id); }
      };

      template<class Engine>
      struct Action<Engine, bool, true> // Boolean value (is classified as an integer and must be captured here)
      {
        static inline void Parse(Serializer<Engine>& e, bool& t, const char* id) { Engine::ParseBool(e, t, id); }
        static inline void Serialize(Serializer<Engine>& e, bool t, const char* id) { Engine::SerializeBool(e, t, id); }
      };

      template<class Engine, class STORE>
      struct Action<Engine, internal::_BIT_REF<STORE>, false> // Bit reference used in some classes
      {
        static inline void Parse(Serializer<Engine>& e, internal::_BIT_REF<STORE>& t, const char* id) { bool b = t; Engine::ParseBool(e, b, id); t = b; }
        static inline void Serialize(Serializer<Engine>& e, internal::_BIT_REF<STORE> t, const char* id) { Engine::SerializeBool(e, t, id); }
      };

      template<class Engine, class T, size_t I, bool B> // For fixed-length arrays
      struct Action<Engine, T[I], B>
      {
        static inline T& Last(T(&obj)[I]) { return obj[0]; }
        static inline void Add(Serializer<Engine>& e, T(&obj)[I], int& n) { if(n < I) Action<Engine, T, std::is_arithmetic<T>::value>::Parse(e, obj[n++], 0); }
        static inline void Parse(Serializer<Engine>& e, T(&obj)[I], const char* id) { Engine::template ParseArray<T[I], T>(e, obj, id); }
        static inline void Serialize(Serializer<Engine>& e, const T(&obj)[I], const char* id) { Engine::template SerializeArray<T[I]>(e, obj, I, id); }
      };
      template<class Engine, class T, size_t I, bool B> // For fixed-length arrays
      struct Action<Engine, std::array<T, I>, B>
      {
        static inline T& Last(std::array<T, I>& obj) { return obj[0]; }
        static inline void Add(Serializer<Engine>& e, std::array<T, I>& obj, int& n) { if(n < I) Action<Engine, T, std::is_arithmetic<T>::value>::Parse(e, obj[n++], 0); }
        static inline void Parse(Serializer<Engine>& e, std::array<T, I>& obj, const char* id) { Engine::template ParseArray<std::array<T, I>, T>(e, obj, id); }
        static inline void Serialize(Serializer<Engine>& e, const std::array<T, I>& obj, const char* id) { Engine::template SerializeArray<std::array<T, I>>(e, obj, I, id); }
      };
      template<class Engine, class T, typename Alloc>
      struct Action<Engine, std::vector<T, Alloc>, false>
      {
        static inline T& Last(std::vector<T, Alloc>& obj) { assert(obj.size()); return obj.back(); }
        static inline void Add(Serializer<Engine>& e, std::vector<T, Alloc>& obj, int& n)
        {
          obj.push_back(T());
          Action<Engine, T, std::is_arithmetic<T>::value>::Parse(e, obj.back(), 0);
        }
        static inline void Parse(Serializer<Engine>& e, std::vector<T, Alloc>& obj, const char* id)
        {
          Engine::template ParseArray<std::vector<T, Alloc>, T>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const std::vector<T, Alloc>& obj, const char* id)
        {
          Engine::template SerializeArray<std::vector<T, Alloc>>(e, obj, obj.size(), id);
        }
      };
      template<class Engine, class E, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, DynArray<E, CType, ArrayType, Alloc>, false>
      {
        static inline E& Last(DynArray<E, CType, ArrayType, Alloc>& obj) { assert(obj.Length()); return obj.Back(); }
        static inline void Add(Serializer<Engine>& e, DynArray<E, CType, ArrayType, Alloc>& obj, int& n)
        {
          obj.AddConstruct();
          Action<Engine, E, std::is_arithmetic<E>::value>::Parse(e, obj.Back(), 0);
        }
        static inline void Parse(Serializer<Engine>& e, DynArray<E, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template ParseArray<DynArray<E, CType, ArrayType, Alloc>, E>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const DynArray<E, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template SerializeArray<DynArray<E, CType, ArrayType, Alloc>>(e, obj, obj.Length(), id);
        }
      };
      template<class Engine, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, DynArray<bool, CType, ArrayType, Alloc>, false>
      {
        static inline bool& Last(DynArray<bool, CType, ArrayType, Alloc>& obj) { assert(false); return *((bool*)~0); } // this should never be called on a boolean array
        static inline void Add(Serializer<Engine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, int& n)
        {
          bool b;
          Action<Engine, bool, std::is_arithmetic<bool>::value>::Parse(e, b, 0);
          obj.Add(b);
        }
        static inline void Parse(Serializer<Engine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template ParseArray<DynArray<bool, CType, ArrayType, Alloc>, bool>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const DynArray<bool, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template SerializeArray<DynArray<bool, CType, ArrayType, Alloc>>(e, obj, obj.Length(), id);
        }
      };

      template<class Engine, class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, Array<T, CType, ArrayType, Alloc>, false>
      {
        static inline T& Last(Array<T, CType, ArrayType, Alloc>& obj) { assert(obj.Capacity()); return obj.Back(); }
        static inline void Add(Serializer<Engine>& e, Array<T, CType, ArrayType, Alloc>& obj, int& n)
        {
          obj.SetCapacity(obj.Capacity() + 1);
          Action<Engine, T, std::is_arithmetic<T>::value>::Parse(e, obj.Back(), 0);
        }
        static inline void Parse(Serializer<Engine>& e, Array<T, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template ParseArray<Array<T, CType, ArrayType, Alloc>, T>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const Array<T, CType, ArrayType, Alloc>& obj, const char* id)
        {
          Engine::template SerializeArray<Array<T, CType, ArrayType, Alloc>>(e, obj, obj.Capacity(), id);
        }
      };
      template<class Engine, class Key, class Data, khint_t(*__hash_func)(const Key&), bool(*__hash_equal)(const Key&, const Key&), ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, HashBase<Key, Data, false, __hash_func, __hash_equal, ArrayType, Alloc>, false> // Hashes that are just sets can be treated as arrays.
      {
        typedef HashBase<Key, Data, false, __hash_func, __hash_equal, ArrayType, Alloc> E;
        static inline Key& Last(E& obj) { assert(obj.Length()); return *obj.begin(); }
        static inline void Add(Serializer<Engine>& e, E& obj, int& n)
        {
          Key key;
          Action<Engine, Key, std::is_arithmetic<Key>::value>::Parse(e, key, 0);
          obj.Insert(std::move(key));
        }
        static inline void Parse(Serializer<Engine>& e, E& obj, const char* id)
        {
          Engine::template ParseArray<E, Key>(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const E& obj, const char* id)
        {
          Engine::template SerializeArray<E>(e, obj, obj.Length(), id);
        }
      };

      template<class Engine, class Key, class Data, khint_t(*__hash_func)(const Key&), bool(*__hash_equal)(const Key&, const Key&), ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, HashBase<Key, Data, true, __hash_func, __hash_equal, ArrayType, Alloc>, false>
      {
        typedef HashBase<Key, Data, true, __hash_func, __hash_equal, ArrayType, Alloc> E;
        typedef KeyValueInsert<Engine, Key, Data, __hash_func, __hash_equal, ArrayType, Alloc> F;
        static inline Key& Last(E& obj) { assert(false); return *obj.begin(); }
        static inline void Add(Serializer<Engine>& e, E& obj, int& n) { assert(false); }
        static inline void Parse(Serializer<Engine>& e, E& obj, const char* id)
        {
          Engine::template ParseMany<F>(e, F(obj));
        }
        static inline void Serialize(Serializer<Engine>& e, const E& t, const char* id) { Engine::template Serialize<E>(e, t, id); }
      };

      // Forward Hash<> specializations to the underlying HashBase specializations
      template<class Engine, typename K, typename T, bool ins, ARRAY_TYPE ArrayType, typename Alloc>
      struct Action<Engine, Hash<K, T, ins, ArrayType, Alloc>, false> : Action<Engine, typename Hash<K, T, ins, ArrayType, Alloc>::BASE, false> {};

      template<class Engine, class T, typename Arg, typename... Args>
      struct __ActionVariant
      {
        static inline void Parse(Serializer<Engine>& e, T& obj, const char* id)
        {
          if(obj.template is<Arg>())
          {
            new (&obj) T(); // Set _Tag to -1
            obj.template typeset<Arg>(); // Properly construct type
            Action<Engine, Arg, std::is_arithmetic<Arg>::value>::Parse(e, obj.template get<Arg>(), id);
          }
          else
            __ActionVariant<Engine, T, Args...>::Parse(e, obj, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const T& obj, const char* id)
        {
          if(obj.template is<Arg>())
            Action<Engine, Arg, std::is_arithmetic<Arg>::value>::Serialize(e, obj.template get<Arg>(), id);
          else
            __ActionVariant<Engine, T, Args...>::Serialize(e, obj, id);
        }
      };
      template<class Engine, class T, typename Arg>
      struct __ActionVariant<Engine, T, Arg>
      {
        static inline void Parse(Serializer<Engine>& e, T& obj, const char* id)
        {
          if(obj.template is<Arg>())
          {
            new (&obj) T();
            obj.template typeset<Arg>();
            Action<Engine, Arg, std::is_arithmetic<Arg>::value>::Parse(e, obj.template get<Arg>(), id);
          }
        }
        static inline void Serialize(Serializer<Engine>& e, const T& obj, const char* id)
        {
          if(obj.template is<Arg>())
            Action<Engine, Arg, std::is_arithmetic<Arg>::value>::Serialize(e, obj.template get<Arg>(), id);
          else
            assert(obj.tag() == -1);
        }
      };

      template<typename... Args>
      struct ActionVariantRef
      {
        ActionVariantRef(Variant<Args...>& ref) : _ref(ref) {}

        Variant<Args...>& _ref;
      };

      template<class Engine, typename... Args>
      struct Action<Engine, ActionVariantRef<Args...>, false>
      {
        static inline void Parse(Serializer<Engine>& e, ActionVariantRef<Args...>& obj, const char* id)
        {
          __ActionVariant<Engine, Variant<Args...>, Args...>::Parse(e, obj._ref, id);
        }
        static inline void Serialize(Serializer<Engine>& e, const ActionVariantRef<Args...>& obj, const char* id)
        {
          __ActionVariant<Engine, Variant<Args...>, Args...>::Serialize(e, obj._ref, id);
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


    template<class T> using ActionBind = internal::serializer::Action<Engine, T, std::is_arithmetic<T>::value>;

    template<typename T>
    void Serialize(const T& obj, std::ostream& s, const char* name = 0)
    {
      out = &s;
      in = 0;
      ActionBind<T>::Serialize(*this, obj, name);
    }

    template<typename T>
    void Parse(T& obj, std::istream& s, const char* name = 0)
    {
      out = 0;
      in = &s;
      ActionBind<T>::Parse(*this, obj, name);
    }

    template<typename T, typename... Args>
    inline void EvaluateType(std::pair<const char*, Args&>... args)
    {
      //static_assert(HAS_MEMBER(T, Serialize), "T must implement template<class E> void Serialize(Serializer<E>&)");
      static Trie<uint16_t> t(sizeof...(Args), (args.first)...);

      if(out) // Serializing
        int X[] = { (ActionBind<Args>::Serialize(*this, args.second, args.first), 0)... };

      if(in) // Parsing
      {
        if(Engine::Ordered())
          int X[] = { (ActionBind<Args>::Parse(*this, args.second, args.first), 0)... };
        else
        {
          auto tmp = std::make_tuple<std::pair<const char*, Args&>...>(std::move(args)...);
          Engine::template ParseMany<internal::serializer::KeyValueFind<Engine, std::pair<const char*, Args&>...>>(*this, internal::serializer::KeyValueFind<Engine, std::pair<const char*, Args&>...>(t, tmp));
        }
      }
    }

    template<int I, typename... Args>
    struct r_findparse {
      inline static void f(Serializer<Engine>& e, uint16_t index, const std::tuple<Args...>& args)
      {
        if(index != I)
          return r_findparse<I - 1, Args...>::f(e, index, args);
        ActionBind<typename std::remove_reference<typename std::tuple_element<I, std::tuple<Args...>>::type::second_type>::type>::Parse(e, std::get<I>(args).second, std::get<I>(args).first);
      }
    };
    template<typename... Args>
    struct r_findparse<-1, Args...> {
      inline static void f(Serializer<Engine>& e, uint16_t index, const std::tuple<Args...>& args) {}
    };
    template<typename... Args> // This function must be static due to some corner cases on certain parsers
    inline static void FindParse(Serializer<Engine>& e, const char* key, const Trie<uint16_t>& t, const std::tuple<Args...>& args)
    {
      r_findparse<sizeof...(Args)-1, Args...>::f(e, t[key], args);
    }

    template<class T>
    struct Bulk { };

    template<class T, size_t I>
    struct Bulk<T[I]>
    {
      static inline bool Read(Serializer<Engine>& e, T(&obj)[I], int64_t count)
      {
        if(count != (I * sizeof(T)))
          return false;

        e.in->read((char*)obj, count);
        return true;
      }
      static inline bool Write(Serializer<Engine>& e, const T(&obj)[I], int64_t count)
      {
        e.out->write((const char*)obj, count * sizeof(T));
        return true;
      }
    };

    template<class T, size_t I>
    struct Bulk<std::array<T, I>>
    {
      static inline bool Read(Serializer<Engine>& e, std::array<T, I>& obj, int64_t count)
      {
        if(count != (I * sizeof(T)))
          return false;

        e.in->read((char*)obj, count);
        return true;
      }
      static inline bool Write(Serializer<Engine>& e, const std::array<T, I>& obj, int64_t count)
      {
        e.out->write((const char*)obj.data(), count * sizeof(T));
        return true;
      }
    };
    template<class T, typename Alloc>
    struct Bulk<std::vector<T, Alloc>>
    {
      static inline bool Read(Serializer<Engine>& e, std::vector<T, Alloc>& obj, int64_t count)
      {
        if(count % sizeof(T) != 0)
          return false;

        obj.resize(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        e.in->read((char*)obj.data(), count);
        return true;
      }
      static inline bool Write(Serializer<Engine>& e, const std::vector<T, Alloc>& obj, int64_t count)
      {
        e.out->write((const char*)obj.data(), count * sizeof(T));
        return true;
      }
    };
    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct Bulk<DynArray<T, CType, ArrayType, Alloc>>
    {
      static inline bool Read(Serializer<Engine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, int64_t count)
      {
        if(ArrayType == ARRAY_SAFE || count % sizeof(T) != 0)
          return false;

        obj.SetLength(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        e.in->read((char*)(T*)obj, count);
        return true;
      }
      static inline bool Write(Serializer<Engine>& e, const DynArray<T, CType, ArrayType, Alloc>& obj, int64_t count)
      {
        e.out->write((const char*)obj.begin(), count * sizeof(T));
        return true;
      }
    };
    template<typename Alloc> // Can't bulk read in booleans because these are compressed to bit-efficient representations by the dynamic array objects
    struct Bulk<std::vector<bool, Alloc>> { 
      static inline bool Read(Serializer<Engine>& e, std::vector<bool, Alloc>& obj, int64_t count) { return false; }
      static inline bool Write(Serializer<Engine>& e, const std::vector<bool, Alloc>& obj, int64_t count) { return false; }
    };
    template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct Bulk<DynArray<bool, CType, ArrayType, Alloc>> { 
      static inline bool Read(Serializer<Engine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, int64_t count) { return false; }
      static inline bool Write(Serializer<Engine>& e, const DynArray<bool, CType, ArrayType, Alloc>& obj, int64_t count) { return false; }
    };
  };

  // Reference engine for serializers
  class EmptyEngine
  {
  public:
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static void Serialize(Serializer<EmptyEngine>& e, const T& t, const char* id) { }
    template<typename T>
    static void SerializeArray(Serializer<EmptyEngine>& e, const T& t, size_t size, const char* id) { }
    template<typename T>
    static void SerializeNumber(Serializer<EmptyEngine>& e, T t, const char* id) { }
    static void SerializeBool(Serializer<EmptyEngine>& e, bool t, const char* id) { }
    template<typename T>
    static void Parse(Serializer<EmptyEngine>& e, T& t, const char* id) { }
    template<typename T, typename E, E& (*Last)(T&), void (*Add)(Serializer<EmptyEngine>&, T&, int&)>
    static void ParseArray(Serializer<EmptyEngine>& e, T& obj, const char* id) {}
    template<typename T>
    static void ParseNumber(Serializer<EmptyEngine>& e, T& t, const char* id) {}
    static void ParseBool(Serializer<EmptyEngine>& e, bool& t, const char* id) {}
    template<typename F>
    static void ParseMany(Serializer<EmptyEngine>& e, F && f) { f(e, ""); }
  };
}

#endif
