// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __TOML_H__BSS__
#define __TOML_H__BSS__

#include "Serializer.h"
#include "DynArray.h"
#include "Variant.h"
#include "Hash.h"
#include <chrono>
#include <sstream>
#include <locale>
#include <iomanip>

namespace bss {
  class TOMLEngine
  {
  public:
    TOMLEngine() : state(STATE_BEGIN), first(false) {}
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<TOMLEngine>& e) {}
    static void End(Serializer<TOMLEngine>& e) {}
    template<typename T>
    static void Parse(Serializer<TOMLEngine>& e, T& obj, const char* id);
    template<typename F>
    static void ParseMany(Serializer<TOMLEngine>& e, F && f);
    template<typename T, typename E, void (*Add)(Serializer<TOMLEngine>& e, T& obj, int& n), bool (*Read)(Serializer<TOMLEngine>& e, T& obj, int64_t count)>
    static void ParseArray(Serializer<TOMLEngine>& e, T& obj, const char* id);
    template<typename T>
    static void ParseNumber(Serializer<TOMLEngine>& e, T& obj, const char* id);
    static void ParseBool(Serializer<TOMLEngine>& e, bool& target, const char* id)
    {
      static const char* val = "true";
      std::istream& s = *e.in;
      TOMLEngine::ParseTOMLEatWhitespace(s);
      int pos = 0;
      if(s.peek() >= '0' && s.peek() <= '9')
      {
        uint64_t num;
        s >> num;
        target = num != 0; // If it's numeric, record the value as false if 0 and true otherwise.
        return;
      }
      while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != ']' && s.peek() != -1 && pos < 4)
      {
        if(s.get() != val[pos++])
        {
          target = false;
          return;
        }
      }

      target = true;
    }

    template<typename T>
    static void Serialize(Serializer<TOMLEngine>& e, const T& obj, const char* id);
    template<typename T>
    static void SerializeArray(Serializer<TOMLEngine>& e, const T& obj, size_t size, const char* id);
    template<typename T, size_t... S>
    static void SerializeTuple(Serializer<TOMLEngine>& e, const T& obj, const char* id, std::index_sequence<S...>);
    template<typename T>
    static void SerializeNumber(Serializer<TOMLEngine>& e, T obj, const char* id);
    static void SerializeBool(Serializer<TOMLEngine>& e, bool obj, const char* id)
    {
      if(e.engine.state == 1)
        return;
      std::ostream& s = *e.out;
      WriteTOMLId(e, id, s);
      s << (obj ? "true" : "false");
      if(e.engine.state != STATE_INLINE_TABLE && id)
        s << std::endl;
    }

    static void ParseTOMLEatWhitespace(std::istream& s) { while(!!s && (s.peek() == ' ' || s.peek() == '\t')) s.get(); }
    static void ParseTOMLEatAllspace(std::istream& s)
    {
      while(!!s && (isspace(s.peek()) || s.peek() == '#'))
        if(s.peek() == '#')
          while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1)
            s.get();
        else
          s.get();
    }
    static void ParseTOMLEatNewline(std::istream& s)
    {
      if(s.peek() == '\n')
        s.get();
      if(s.peek() == '\r')
      {
        s.get();
        if(s.peek() == '\n')
          s.get();
      }
    }
    template<bool MULTILINE>
    static void ParseTOMLCharacter(std::string& out, std::istream& s);
    template<bool MULTILINE, char END, char END2>
    static void ParseTOMLString(std::string& buf, std::istream& s);
    template<typename F>
    static void ParseTOMLTable(Serializer<TOMLEngine>& e, std::istream& s, F && f);
    template<typename F>
    static void ParseTOMLPairs(Serializer<TOMLEngine>& e, std::istream& s, F && f);
    template<typename F>
    static void ParseTOMLRoot(Serializer<TOMLEngine>& e, std::istream& s, F && f);

    static void WriteTOMLId(Serializer<TOMLEngine>& e, const char* id, std::ostream& s)
    {
      if(!id || e.engine.state == STATE_INLINE_TABLE)
      {
        if(e.engine.first)
          e.engine.first = false;
        else
          s << ", ";
      }
      if(id) s << id << " = ";
    }
    template<typename T>
    static void WriteTOMLTables(Serializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s);
    template<typename T>
    static void WriteTOMLPairs(Serializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s);

    template<class T>
    struct TOMLWrapper
    {
      TOMLWrapper(T& ref, const char* id) : Ref(ref), ID(id) {}
      T& Ref;
      const char* ID;

      template<typename Engine>
      void Serialize(Serializer<Engine>& s, const char*)
      {
        s.template EvaluateType<TOMLWrapper>(GenPair(ID, Ref));
      }
    };

    enum STATE : uint16_t { STATE_NORMAL, STATE_TABLE, STATE_INLINE_TABLE, STATE_BEGIN };
    uint16_t state;
    Str lastid;
    bool first;
  };

  template<bool MULTILINE>
  void TOMLEngine::ParseTOMLCharacter(std::string& out, std::istream& s)
  {
    if(s.peek() == '\\')
    {
      s.get();
      if(!s || s.peek() == -1)
        return;
      if constexpr(MULTILINE)
      {
        if(s.peek() == '\n' || s.peek() == '\r')
        {
          ParseTOMLEatNewline(s);
          ParseTOMLEatWhitespace(s);
          return;
        }
      }
      char buf[9];
      switch(s.get())
      {
      case 'b': out += '\b'; break;
      case 't': out += '\t'; break;
      case 'n': out += '\n'; break;
      case 'f': out += '\f'; break;
      case 'r': out += '\r'; break;
      case '"': out += '"'; break;
      case '\\': out += '\\'; break;
      case 'u':
        for(int i = 0; i < 4; ++i) buf[i] = s.get();
        buf[4] = 0;
        OutputUnicode(out, strtoul(buf, 0, 16));
        break;
      case 'U':
        for(int i = 0; i < 8; ++i) buf[i] = s.get();
        buf[8] = 0;
        OutputUnicode(out, strtoul(buf, 0, 16));
        break;
      default:
        assert(false);
        break;
      }
    }
    else
      out += s.get();
  }

  template<bool MULTILINE, char END, char END2>
  void TOMLEngine::ParseTOMLString(std::string& buf, std::istream& s)
  {
    buf.clear();
    ParseTOMLEatWhitespace(s);
    switch(s.peek())
    {
    case '"':
      s.get();
      if(s.peek() == '"')
      {
        s.get();
        if(s.peek() == '"' && MULTILINE) // If true this is a multiline string
        {
          s.get();
          ParseTOMLEatNewline(s);
          while(!!s)
          {
            if(s.peek() == '"')
            {
              s.get();
              if(s.peek() == '"')
              {
                s.get();
                if(s.peek() == '"')
                  break;
                buf += '"';
              }
              buf += '"';
            }
            else
              ParseTOMLCharacter<MULTILINE>(buf, s);
          }
        }
      } // Otherwise it's an empty string
      else
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != '"' && s.peek() != -1)
          ParseTOMLCharacter<MULTILINE>(buf, s);
      if(s.peek() == '"')
        s.get();
      break;
    case '\'':
      s.get();
      if(s.peek() == '\'')
      {
        s.get();
        if(s.peek() == '\'' && MULTILINE) // If true this is a multiline literal string
        {
          s.get();
          ParseTOMLEatNewline(s);
          while(!!s)
          {
            char c = s.get();
            if(c == '\'' && s.peek() == '\'')
            {
              c = s.get();
              if(s.peek() == '\'')
                break;
              buf += '\'';
            }
            buf += c;
          }
        } // Otherwise it's an empty literal string
      }
      else
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != '\'' && s.peek() != -1) buf += s.get();
      if(s.peek() == '\'')
        s.get();
      break;
    default: // This is a bare string, which cannot be multiline
    {
      while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != END && s.peek() != END2 && s.peek() != -1) buf += s.get();
      const char* str = buf.data(); // strip whitespace from the end
      const char* inter = str + buf.size();
      if(str != inter)
      {
        for(; inter > str && *inter < 33; --inter);
        buf.resize((inter - str) + 1);
      }
    }
    break;
    }
  }

  template<typename F>
  void TOMLEngine::ParseTOMLTable(Serializer<TOMLEngine>& e, std::istream& s, F && f)
  {
    ParseTOMLEatAllspace(s);
    Str buf;
    while(!!s && s.peek() != '}' && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1)
    {
      TOMLEngine::ParseTOMLString<false, '=', 0>(buf, s);
      ParseTOMLEatWhitespace(s);
      if(!!s && s.peek() == '=')
      {
        s.get();
        internal::serializer::PushValue<decltype(e.engine.state)> push(e.engine.state, STATE_NORMAL);
        f(e, buf.c_str()); // This will call FindParse by default, or a special function for TOMLValue types
      }
      while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) s.get(); // Eat everything until the next comma or the end of the table
      if(s.peek() == ',') s.get();
      ParseTOMLEatWhitespace(s);
    }
    if(s.peek() == '}') s.get();
  }

  template<typename F>
  void TOMLEngine::ParseTOMLPairs(Serializer<TOMLEngine>& e, std::istream& s, F && f)
  {
    ParseTOMLEatAllspace(s);
    Str buf;
    while(!!s && s.peek() != '[' && s.peek() != -1)
    {
      TOMLEngine::ParseTOMLString<false, '=', 0>(buf, s);
      ParseTOMLEatWhitespace(s);
      if(!!s && s.peek() == '=')
      {
        s.get();
        internal::serializer::PushValue<decltype(e.engine.state)> push(e.engine.state, STATE_NORMAL);
        f(e, buf.c_str()); // This will call ParseTOMLBase on the appropriate value.
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1) s.get(); // Eat all remaining characters on this line that weren't parsed.
      }
      ParseTOMLEatAllspace(s);
    }
  }

  // This is the primary parsing function valid only for the root of the document. It only parses [tables] and [[table arrays]].
  template<typename F>
  void TOMLEngine::ParseTOMLRoot(Serializer<TOMLEngine>& e, std::istream& s, F && f)
  {
    Str buf;
    ParseTOMLPairs<F>(e, s, f); // First we parse any root-level key value pairs directly into our target object
    while(!!s && s.get() == '[') // Then, we start resolving tables
    {
      char isarray = s.peek() == '[';
      if(isarray)
        s.get();

      // We must navigate through as many nested tables as there are dots, drilling down through the serializer resolver. Then we call ParseTOMLPairs on the final value.
      e.engine.state = isarray + 1;
      ParseTOMLString<false, ']', '.'>(buf, s);
      ParseTOMLEatWhitespace(s);
      if(!!s && (s.peek() == ']' || s.peek() == '.'))
        f(e, buf.c_str());
      while(!!s && s.peek() != '[' && s.peek() != -1) // eat lines until we find one that starts with [
      {
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1)
          s.get();
        ParseTOMLEatNewline(s);
      }
      ParseTOMLEatAllspace(s);
    }
  }

  template<class T>
  inline void ParseTOMLBase(Serializer<TOMLEngine>& e, T& obj, std::istream& s);

  // Represents an arbitrary TOML value in any of the standard storage types recognized by the format.
  struct TOMLValue : public Variant<Str, double, int64_t, bool, std::chrono::system_clock::time_point, DynArray<TOMLValue, size_t, ARRAY_SAFE>, Hash<Str, TOMLValue, ARRAY_SAFE>>
  {
    typedef DynArray<TOMLValue, size_t, ARRAY_SAFE> TOMLArray;
    typedef Hash<Str, TOMLValue, ARRAY_SAFE> TOMLTable; // All objects in TOML are tables
    typedef Variant<Str, double, int64_t, bool, std::chrono::system_clock::time_point, TOMLArray, TOMLTable> BASE;

  public:
    TOMLValue() : BASE() {}
    TOMLValue(const BASE& v) : BASE(v) {}
    TOMLValue(BASE&& v) : BASE(std::move(v)) {}
    template<typename T>
    explicit TOMLValue(const T& v) : BASE(v) {}
    template<typename T>
    explicit TOMLValue(T&& v) : BASE(internal::serializer::ConvRef<TOMLValue>::Value<T, BASE>::f(v)) {}
    ~TOMLValue() {}
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(internal::serializer::ConvRef<TOMLValue>::Value<T, BASE>::f(right)); return *this; }
  };

  template<typename T>
  void TOMLEngine::ParseNumber(Serializer<TOMLEngine>& e, T& obj, const char* id)
  {
    std::istream& s = *e.in;
    if(s.peek() == '\n' || s.peek() == '\r' || s.peek() == ',')
      return;
    typename std::conditional<sizeof(T) <= 2, int, T>::type o; // Prevent the stream from trying to read a number as an actual character
    if(s.peek() == '"') // if true, we have to attempt to coerce the string to T
    {
      s.get();
      s >> o; // grab whatever we can
      while(!!s && s.peek() != -1 && s.get() != '"'); // eat the rest of the string
      s.get(); // eat the " character
    }
    else
      s >> o;
    obj = o;
  };

  template<class T>
  inline void ParseTOMLBase(Serializer<TOMLEngine>& e, T& obj, std::istream& s) 
  { 
    static_assert(internal::serializer::is_serializable<TOMLEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
    obj.template Serialize<TOMLEngine>(e, 0);
  }
  template<>
  inline void ParseTOMLBase<std::string>(Serializer<TOMLEngine>& e, std::string& target, std::istream& s) { TOMLEngine::ParseTOMLString<true, 0, 0>(target, s); }
  template<>
  inline void ParseTOMLBase<Str>(Serializer<TOMLEngine>& e, Str& target, std::istream& s) { ParseTOMLBase<std::string>(e, target, s); }

#ifdef BSS_COMPILER_HAS_TIME_GET
  template<>
  inline void ParseTOMLBase<std::chrono::system_clock::time_point>(Serializer<TOMLEngine>& e, std::chrono::system_clock::time_point& target, std::istream& s)
  {
    using days = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
    static const char DATE[] = "%Y-%m-%d";
    static const char TIME[] = "%H:%M:%S";
    static const char ZONE[] = "%H:%M";
    const std::time_get<char>& tg = std::use_facet<std::time_get<char>>(s.getloc());

    std::tm t = { 0 };
    std::ios_base::iostate err = std::ios_base::goodbit;
    TOMLEngine::ParseTOMLEatWhitespace(s);
    tg.get(s, 0, s, err, &t, DATE, std::end(DATE) - 1);

    if(err == std::ios_base::goodbit)
    {
      if(s.peek() == 'T')
      {
        s.get();
        tg.get(s, 0, s, err, &t, TIME, std::end(TIME) - 1);

        if(err == std::ios_base::goodbit)
        {
          char sign = s.peek();

          if(sign == '+' || sign == '-')
          {
            s.get();
            std::tm tz = { 0 };
            tg.get(s, 0, s, err, &tz, ZONE, std::end(ZONE) - 1);
            std::chrono::minutes offset(0);

            if(err == std::ios_base::goodbit)
              offset = (sign == '+' ? 1 : -1) * (std::chrono::hours{ tz.tm_hour } +std::chrono::minutes{ tz.tm_min });

            target = std::chrono::system_clock::time_point{
              days { DaysFromCivil(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday) } +
              std::chrono::hours { t.tm_hour } +std::chrono::minutes { t.tm_min } +std::chrono::seconds { t.tm_sec } -
              offset };

            return;
          }
          else if(sign == 'Z')
            s.get();
        }
      }

      target = std::chrono::system_clock::time_point{ days { DaysFromCivil(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday) } };
    }
  }
#endif

  // This function parses values and is recursive, allowing for nested inline tables or arrays, etc.
  template<>
  inline void ParseTOMLBase<TOMLValue>(Serializer<TOMLEngine>& e, TOMLValue& target, std::istream& s)
  {
    auto test = e.in->peek();
    switch(e.engine.state)
    {
    case TOMLEngine::STATE_BEGIN:
      target = TOMLValue::TOMLTable();
      Serializer<TOMLEngine>::ActionBind<TOMLValue::TOMLTable>::Parse(e, target.get<TOMLValue::TOMLTable>(), 0);
      return;
    case TOMLEngine::STATE_TABLE:
      if(e.in->peek() == '.')
      {
        e.in->get();
        Str buf;
        TOMLEngine::ParseTOMLString<false, '.', ']'>(buf, *e.in);
        TOMLValue* v = target.get<TOMLValue::TOMLTable>()[buf];
        if(v)
          ParseTOMLBase<TOMLValue>(e, *v, *e.in);
      }
      else if(e.in->peek() == ']')
      {
        target = TOMLValue::TOMLTable();
        Serializer<TOMLEngine>::ActionBind<TOMLValue::TOMLTable>::Parse(e, target.get<TOMLValue::TOMLTable>(), 0);
      }
      else
        assert(false);
      return;
    case TOMLEngine::STATE_INLINE_TABLE:
      if(e.in->peek() == '.')
      {
        e.in->get();
        Str buf;
        TOMLEngine::ParseTOMLString<false, '.', ']'>(buf, *e.in);
        TOMLValue* v = target.get<TOMLValue::TOMLTable>()[buf];
        if(v)
          ParseTOMLBase<TOMLValue>(e, *v, *e.in);
      }
      else if(e.in->peek() == ']')
      {
        target = TOMLValue::TOMLTable();
        Serializer<TOMLEngine>::ActionBind<TOMLValue::TOMLTable>::Parse(e, target.get<TOMLValue::TOMLTable>(), 0);
      }
      else
        assert(false);
      return;
    case 0:
      break;
    }

    TOMLEngine::ParseTOMLEatWhitespace(s);
    switch(s.peek())
    {
    case '{': // inline table
      target = TOMLValue::TOMLTable();
      Serializer<TOMLEngine>::ActionBind<TOMLValue::TOMLTable>::Parse(e, target.get<TOMLValue::TOMLTable>(), 0);
      break;
    case '[': // array
      target = TOMLValue::TOMLArray();
      Serializer<TOMLEngine>::ActionBind<TOMLValue::TOMLArray>::Parse(e, target.get<TOMLValue::TOMLArray>(), 0);
      break;
    case '"': // Single or multi-line string
    case '\'': // Single or multi-line literal string
      target = Str();
      ParseTOMLBase<Str>(e, target.get<Str>(), s);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
    case '-':
    case '+':
      target = 0.0;
      TOMLEngine::ParseNumber<double>(e, target.get<double>(), 0);
      double ipart;

      if(modf(target.get<double>(), &ipart) == 0.0)
        target = (int64_t)ipart;

      break;
    case 't':
    case 'f':
      target = false;
      TOMLEngine::ParseBool(e, target.get<bool>(), 0);
      break;
    }
  }

  template<class T>
  inline void ParseTOML(T& obj, std::istream& s) { Serializer<TOMLEngine> e; e.Parse(obj, s, 0); }
  template<class T>
  inline void ParseTOML(T& obj, const char* s) { std::istringstream ss(s); ParseTOML<T>(obj, ss); }

  template<typename T>
  void TOMLEngine::Parse(Serializer<TOMLEngine>& e, T& obj, const char* id)
  {
    if(e.engine.state == STATE_BEGIN && id && id[0])
    {
      TOMLWrapper<T> wrap(obj, id);
      ParseTOMLBase<TOMLWrapper<T>>(e, wrap, *e.in);
    }
    else
      ParseTOMLBase<T>(e, obj, *e.in);
  }

  template<typename T, typename E, void (*Add)(Serializer<TOMLEngine>& e, T& obj, int& n), bool (*Read)(Serializer<TOMLEngine>& e, T& obj, int64_t count)>
  void TOMLEngine::ParseArray(Serializer<TOMLEngine>& e, T& obj, const char* id)
  {
    int n = 0;
    std::istream& s = *e.in;
    if(e.engine.state > STATE_NORMAL && e.in->peek() == '.') // If this happens, we are attempting to access an array
    {
      auto end = std::end(obj);
      return Serializer<TOMLEngine>::ActionBind<E>::Parse(e, *--end, id);
    }
    if(e.engine.state == STATE_INLINE_TABLE) // If the state is 2 and not 0, this is a table array.
      Add(e, obj, n);
    else // Otherwise it's a standard inline array
    {
      ParseTOMLEatWhitespace(s);
      if(!s || s.get() != '[') return;
      ParseTOMLEatAllspace(s);
      while(!!s && s.peek() != ']' && s.peek() != -1)
      {
        Add(e, obj, n);
        while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
        if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
        ParseTOMLEatAllspace(s);
      }
    }
  }

  template<typename F>
  void TOMLEngine::ParseMany(Serializer<TOMLEngine>& e, F && f)
  {
    switch(e.engine.state)
    {
    case STATE_BEGIN: // this is the initial object serialization.
      ParseTOMLRoot(e, *e.in, f);
      break;
    case STATE_TABLE:
      if(e.in->peek() == '.')
      {
        e.in->get();
        Str buf;
        ParseTOMLString<false, '.', ']'>(buf, *e.in);
        f(e, buf.c_str());
      }
      else if(e.in->peek() == ']')
      {
        e.in->get();
        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      break;
    case STATE_INLINE_TABLE:
      if(e.in->peek() == '.')
      {
        e.in->get();
        Str buf;
        ParseTOMLString<false, '.', ']'>(buf, *e.in);
        f(e, buf.c_str());
      }
      else if(e.in->peek() == ']')
      {
        e.in->get();

        if(e.in->peek() == ']')
          e.in->get();

        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      break;
    case STATE_NORMAL:
      ParseTOMLEatWhitespace(*e.in);

      if(e.in->peek() == '{')
      {
        e.in->get();
        ParseTOMLTable(e, *e.in, f);
      }
      break;
    }
  }

  template<typename T>
  void TOMLEngine::SerializeArray(Serializer<TOMLEngine>& e, const T& obj, size_t size, const char* id)
  {
    if(e.engine.state == STATE_TABLE)
      return;

    std::ostream& s = *e.out;
    WriteTOMLId(e, id, s);
    internal::serializer::PushValue<bool> push(e.engine.first, true);
    s << '[';

    auto begin = std::begin(obj);
    auto end = std::end(obj);
    for(; begin != end; ++begin)
      Serializer<TOMLEngine>::ActionBind<remove_cvref_t<decltype(*begin)>>::Serialize(e, *begin, 0);

    s << ']';
    if(e.engine.state != STATE_INLINE_TABLE && id)
      s << std::endl;
  }

  template<typename T, size_t... S>
  void TOMLEngine::SerializeTuple(Serializer<TOMLEngine>& e, const T& obj, const char* id, std::index_sequence<S...>)
  {
    if(e.engine.state == STATE_TABLE)
      return;

    std::ostream& s = *e.out;
    WriteTOMLId(e, id, s);
    internal::serializer::PushValue<bool> push(e.engine.first, true);
    s << '[';

    (Serializer<TOMLEngine>::ActionBind<std::tuple_element_t<S, T>>::Serialize(e, std::get<S>(obj), 0), ...);

    s << ']';
    if(e.engine.state != STATE_INLINE_TABLE && id)
      s << std::endl;
  }

  template<class T>
  void TOMLEngine::SerializeNumber(Serializer<TOMLEngine>& e, T obj, const char* id)
  {
    if(e.engine.state == STATE_TABLE)
      return;
    std::ostream& s = *e.out;
    WriteTOMLId(e, id, s);
    s << (typename std::conditional<sizeof(T) <= 2, int, T>::type)obj; // Converts all smaller numbers to integers to avoid character insertion bullshit
    if(e.engine.state != STATE_INLINE_TABLE && id)
      s << std::endl;
  }

  template<class T>
  void TOMLEngine::WriteTOMLTables(Serializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
  {
    Str last = e.engine.lastid;

    if(e.engine.lastid.size())
      e.engine.lastid += '.';
    e.engine.lastid += id;
    static_assert(internal::serializer::is_serializable<TOMLEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
    const_cast<T&>(obj).template Serialize<TOMLEngine>(e, id);
    e.engine.lastid = last;
  }

  template<class T>
  void TOMLEngine::WriteTOMLPairs(Serializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
  {
    if(e.engine.state == STATE_INLINE_TABLE) // if we're in state 2 we're writing an inline table
    {
      WriteTOMLId(e, id, s);
      s << "{ ";
    }
    else if(id)
    {
      s << '[';
      if(e.engine.lastid.size())
        s << e.engine.lastid << '.';
      s << id << ']' << std::endl;
    }

    e.engine.first = true;
    static_assert(internal::serializer::is_serializable<TOMLEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
    const_cast<T&>(obj).template Serialize<TOMLEngine>(e, id);

    if(e.engine.state == STATE_INLINE_TABLE)
      s << " }";
    else
      s << std::endl;
  }

  template<class T>
  inline void WriteTOML(const T& obj, std::ostream& s) { Serializer<TOMLEngine> e; internal::serializer::PushValue<decltype(e.engine.state)> push(e.engine.state, TOMLEngine::STATE_TABLE); e.Serialize<T>(obj, s, 0); }
  template<class T>
  inline void WriteTOML(const T& obj, const char* file) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteTOML<T>(obj, fs); }

  template<typename T>
  void TOMLEngine::Serialize(Serializer<TOMLEngine>& e, const T& obj, const char* id)
  {
    std::ostream& s = *e.out;

    if constexpr(std::is_base_of<std::string, T>::value)
    {
      if(e.engine.state == STATE_TABLE)
        return;

      TOMLEngine::WriteTOMLId(e, id, s);
      s << '"';

      for(size_t i = 0; i < obj.size(); ++i)
      {
        switch(obj[i])
        {
        case '"': s << "\\\""; break;
        case '\\': s << "\\\\"; break;
        case '\b': s << "\\b"; break;
        case '\f': s << "\\f"; break;
        case '\n': s << "\\n"; break;
        case '\r': s << "\\r"; break;
        case '\t': s << "\\t"; break;
        default: s << obj[i]; break;
        }
      }

      s << '"';
      if(e.engine.state != STATE_INLINE_TABLE && id)
        s << std::endl;
    }
#ifdef BSS_COMPILER_HAS_TIME_GET
    else if constexpr(std::is_base_of<std::chrono::system_clock::time_point, T>::value)
    {
      if(e.engine.state == STATE_TABLE)
        return;

      TOMLEngine::WriteTOMLId(e, id, s);
      time_t time = std::chrono::system_clock::to_time_t(obj);
      s << std::put_time(gmtime(&time), "%Y-%m-%dT%H:%M:%S+00:00");
      if(e.engine.state != STATE_INLINE_TABLE && id)
        s << std::endl;
    }
#endif
    else
    {
      if(e.engine.state == STATE_INLINE_TABLE || (!id && !e.engine.state))
      {
        internal::serializer::PushValue<decltype(e.engine.state)> push(e.engine.state, STATE_INLINE_TABLE);
        TOMLEngine::WriteTOMLPairs(e, id, obj, s);
      }
      else if(!!e.engine.state)
      {
        e.engine.state = STATE_NORMAL;
        TOMLEngine::WriteTOMLPairs(e, id, obj, s);
        e.engine.state = STATE_TABLE;
        TOMLEngine::WriteTOMLTables(e, id, obj, s);
        e.engine.state = STATE_TABLE;
      }
    }
  }
}




#endif