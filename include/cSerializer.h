// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SERIALIZER_H__BSS__
#define __C_SERIALIZER_H__BSS__

#include <iostream>
#include <tuple>
#include "cTrie.h"

namespace bss_util {
  //DEFINE_MEMBER_CHECKER(Serialize); // doesn't seem to work for template functions

  template<class T>
  std::pair<const char*, T&> GenPair(const char* l, T& r) { return std::pair<const char*, T&>(l, r); }

  // Defines a universal serializer using an arbitrary serialization engine for types that implement "template<typename Engine> void Serialize(cSerializer<Engine>& e)"
  template<class Engine>
  class cSerializer
  {
  public:
    cSerializer() {}
    explicit cSerializer(std::ostream& s) : out(&s), in(0) {}
    explicit cSerializer(std::istream& s) : out(0), in(&s) {}
    ~cSerializer() { if(out) out->flush(); }
    Engine engine;
    std::ostream* out;
    std::istream* in;

    template<typename T>
    void Serialize(const T& obj, std::ostream& s, const char* name = 0)
    {
      out = &s;
      in = 0;
      Engine::template Serialize<T>(*this, const_cast<T&>(obj), name);
    }

    template<typename T>
    void Parse(T& obj, std::istream& s, const char* name = 0)
    {
      out = 0;
      in = &s;
      Engine::template Parse<T>(*this, obj, name);
    }

    template<typename T, typename... Args>
    inline void EvaluateType(std::pair<const char*, Args&>... args)
    {
      //static_assert(HAS_MEMBER(T, Serialize), "T must implement template<class E> void Serialize(cSerializer<E>&)");
      static cTrie<uint16_t> t(sizeof...(Args), (args.first)...);
      if(out) // Serializing
        int X[] = { (engine.template Serialize<Args>(*this, args.second, args.first), 0)... };
      if(in) // Parsing
      {
        if(Engine::Ordered())
          int X[] = { (engine.template Parse<Args>(*this, args.second, args.first), 0)... };
        else
        {
          auto tmp = std::make_tuple<std::pair<const char*, Args&>...>(std::move(args)...);
          Engine::template ParseMany<std::pair<const char*, Args&>...>(*this, t, tmp);
        }
      }
    }

    template<int I, typename... Args>
    struct r_findparse {
      inline static void f(cSerializer<Engine>& e, uint16_t index, const std::tuple<Args...>& args)
      {
        if(index != I)
          return r_findparse<I - 1, Args...>::f(e, index, args);
        Engine::template Parse<typename std::remove_reference<typename std::tuple_element<I, std::tuple<Args...>>::type::second_type>::type>(e, std::get<I>(args).second, std::get<I>(args).first);
      }
    };
    template<typename... Args>
    struct r_findparse<-1, Args...> {
      inline static void f(cSerializer<Engine>& e, uint16_t index, const std::tuple<Args...>& args) {}
    };
    template<typename... Args> // This function must be static due to some corner cases on certain parsers
    inline static void _findparse(cSerializer<Engine>& e, const char* key, const cTrie<uint16_t>& t, const std::tuple<Args...>& args)
    {
      r_findparse<sizeof...(Args)-1, Args...>::f(e, t[key], args);
    }
  };

  // Reference engine for serializers
  class EmptyEngine
  {
  public:
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static typename std::enable_if<std::is_class<T>::value>::type Serialize(cSerializer<EmptyEngine>& e, T& t, const char* id) { t.template Serialize<EmptyEngine>(e); }
    template<typename T>
    static typename std::enable_if<!std::is_class<T>::value>::type Serialize(cSerializer<EmptyEngine>& e, T& t, const char* id) {}
    template<typename T>
    static typename std::enable_if<std::is_class<T>::value>::type Parse(cSerializer<EmptyEngine>& e, T& t, const char* id) { t.template Serialize<EmptyEngine>(e); }
    template<typename T>
    static typename std::enable_if<!std::is_class<T>::value>::type Parse(cSerializer<EmptyEngine>& e, T& t, const char* id) {}
    template<typename... Args>
    static void ParseMany(cSerializer<EmptyEngine>& e, const cTrie<uint16_t>& t, const std::tuple<Args...>& args) { cSerializer<EmptyEngine>::_findparse<Args...>(e, "", t, args); }
  };
}

#endif
