// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_TOML_H__BSS__
#define __C_TOML_H__BSS__

#include "cHash.h"
#include "cDynArray.h"
#include "variant.h"
#include "cSerializer.h"
#include <chrono>
#include <sstream>
#include <locale>
#include <iomanip>

namespace bss_util {
  class TOMLEngine
  {
  public:
    TOMLEngine() : state(3), first(false) {}
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static void Serialize(cSerializer<TOMLEngine>& e, const T& t, const char* id);
    template<typename T>
    static void Parse(cSerializer<TOMLEngine>& e, T& t, const char* id);
    template<typename... Args>
    static void ParseMany(cSerializer<TOMLEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args);

    uint32_t state;
    cStr id;
    bool first;
  };

  struct TOMLValue;

  template<class T, class BASE>
  struct __TOMLValue__conv
  {
    inline static constexpr T&& f(typename std::remove_reference<T>::type& r) { return (static_cast<T&&>(r)); }
    inline static constexpr T&& f(typename std::remove_reference<T>::type&& r) { return (static_cast<T&&>(r)); }
  };
  template<class BASE>
  struct __TOMLValue__conv<TOMLValue, BASE>
  {
    inline static constexpr BASE&& f(std::remove_reference<TOMLValue>::type& r) { return (static_cast<BASE&&>(r)); }
    inline static constexpr BASE&& f(std::remove_reference<TOMLValue>::type&& r) { return (static_cast<BASE&&>(r)); }
  };
  template<class BASE>
  struct __TOMLValue__conv<const TOMLValue, BASE>
  {
    inline static constexpr const BASE&& f(std::remove_reference<const TOMLValue>::type& r) { return (static_cast<const BASE&&>(r)); }
    inline static constexpr const BASE&& f(std::remove_reference<const TOMLValue>::type&& r) { return (static_cast<const BASE&&>(r)); }
  };

  template<class T>
  void static WriteTOMLBase(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s);

  // Represents an arbitrary TOML value in any of the standard storage types recognized by the format.
  struct TOMLValue : public variant<cStr, double, int64_t, bool, std::chrono::system_clock::time_point, cDynArray<TOMLValue, size_t, CARRAY_SAFE>, cHash<cStr, TOMLValue, false, CARRAY_SAFE>>
  {
    typedef cDynArray<TOMLValue, size_t, CARRAY_SAFE> TOMLArray;
    typedef cHash<cStr, TOMLValue, false, CARRAY_SAFE> TOMLTable; // All objects in TOML are tables
    typedef variant<cStr, double, int64_t, bool, std::chrono::system_clock::time_point, TOMLArray, TOMLTable> BASE;

  public:
    TOMLValue() : BASE() {}
    TOMLValue(const BASE& v) : BASE(v) {}
    TOMLValue(BASE&& v) : BASE(std::move(v)) {}
    template<typename T>
    explicit TOMLValue(const T& t) : BASE(t) {}
    template<typename T>
    explicit TOMLValue(T&& t) : BASE(__TOMLValue__conv<T, BASE>::f(t)) {}
    ~TOMLValue() { }
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(__TOMLValue__conv<T, BASE>::f(right)); return *this; }

    template<typename Engine>
    void Serialize(cSerializer<Engine>& e)
    {
      assert(BASE::template is<TOMLTable>());
      if(e.out)
      {
        auto& v = BASE::template get<TOMLTable>();
        for(auto i : v)
          WriteTOMLBase<TOMLValue>(e, v.GetKey(i).c_str(), *v.GetValue(i), *e.out);
      }
    }
  };

  template<class T>
  void static ParseTOMLBase(cSerializer<TOMLEngine>& e, T& obj, std::istream& s);

  static inline void ParseTOMLEatWhitespace(std::istream& s) { while(!!s && (s.peek() == ' ' || s.peek() == '\t')) s.get(); }

  static inline void ParseTOMLEatAllspace(std::istream& s)
  {
    while(!!s && (isspace(s.peek()) || s.peek() == '#'))
      if(s.peek() == '#')
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1)
          s.get();
      else
        s.get();
  }

  inline void static ParseTOMLEatNewline(std::istream& s)
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
  inline void static ParseTOMLCharacter(std::string& out, std::istream& s)
  {
    if(s.peek() == '\'')
    {
      s.get();
      if(!!s || s.peek() == -1)
        return;
      if(MULTILINE && (s.peek() == '\n' || s.peek() == '\r'))
      {
        ParseTOMLEatNewline(s);
        ParseTOMLEatWhitespace(s);
        return;
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
  inline void static ParseTOMLString(std::string& buf, std::istream& s)
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

  template<class T, bool A>
  struct ParseTOMLInternal
  {
    static void F(cSerializer<TOMLEngine>& e, T& obj, std::istream& s) {
      obj.template Serialize<TOMLEngine>(e);
    }
  };

  template<class T> // For arithmetic base types
  struct ParseTOMLInternal<T, true>
  {
    static void F(cSerializer<TOMLEngine>& e, T& obj, std::istream& s)
    {
      if(s.peek() == '\n' || s.peek() == '\r' || s.peek() == ',') return;
      if(s.peek() == '"') // if true, we have to attempt to coerce the string to T
      {
        s.get();
        s >> obj; // grab whatever we can
        while(!!s && s.peek() != -1 && s.get() != '"'); // eat the rest of the string
        s.get(); // eat the " character
      }
      else
        s >> obj;
    }
  };

  template<class T>
  void ParseTOMLArray(cSerializer<TOMLEngine>& e, T& obj, std::istream& s)
  {
    int n = 0;
    if(e.engine.state > 0 && e.in->peek() == '.') // If this happens, we are attempted to access an array
      return ParseTOMLBase(e, ParseTOMLInternal<T, false>::GetLast(obj), s);
    if(e.engine.state == 2) // If the state is 2 and not 0, this is a table array.
      ParseTOMLInternal<T, false>::DoAddCall(e, obj, s, n);
    else // Otherwise it's a standard inline array
    {
      ParseTOMLEatWhitespace(s);
      if(!s || s.get() != '[') return;
      ParseTOMLEatAllspace(s);
      while(!!s && s.peek() != ']' && s.peek() != -1)
      {
        ParseTOMLInternal<T, false>::DoAddCall(e, obj, s, n);
        while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
        if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
        ParseTOMLEatAllspace(s);
      }
    }
  }

  template<typename F>
  inline void BSS_EXPLICITSTATIC ParseTOMLTable(cSerializer<TOMLEngine>& e, std::istream& s, F f)
  {
    ParseTOMLEatAllspace(s);
    cStr buf;
    while(!!s && s.peek() != '}' && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1)
    {
      ParseTOMLString<false, '=', 0>(buf, s);
      ParseTOMLEatWhitespace(s);
      if(!!s && s.peek() == '=')
      {
        s.get();
        e.engine.state = 0;
        f(e, buf.c_str()); // This will call _findparse by default, or a special function for TOMLValue types
      }
      while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) s.get(); // Eat everything until the next comma or the end of the table
      if(s.peek() == ',') s.get();
      ParseTOMLEatWhitespace(s);
    }
    if(s.peek() == '}') s.get();
  }

  template<typename F>
  inline void BSS_EXPLICITSTATIC ParseTOMLPairs(cSerializer<TOMLEngine>& e, std::istream& s, F f)
  {
    ParseTOMLEatAllspace(s);
    cStr buf;
    while(!!s && s.peek() != '[' && s.peek() != -1)
    {
      ParseTOMLString<false, '=', 0>(buf, s);
      ParseTOMLEatWhitespace(s);
      if(!!s && s.peek() == '=')
      {
        s.get();
        e.engine.state = 0;
        f(e, buf.c_str()); // This will call ParseTOMLBase on the appropriate value.
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1) s.get(); // Eat all remaining characters on this line that weren't parsed.
      }
      ParseTOMLEatAllspace(s);
    }
  }

  // This is the primary parsing function valid only for the root of the document. It only parses [tables] and [[table arrays]].
  template<typename F>
  inline void BSS_EXPLICITSTATIC ParseTOMLRoot(cSerializer<TOMLEngine>& e, std::istream& s, F f)
  {
    cStr buf;
    ParseTOMLPairs(e, s, f); // First we parse any root-level key value pairs directly into our target object
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
      while(!!s && s.peek() != '[' && s.peek() != -1) s.get(); // Eat all remaining characters until the next [
      ParseTOMLEatAllspace(s);
    }
  }

  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseTOMLInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline T& GetLast(cDynArray<T, CType, ArrayType, Alloc>& obj) { assert(obj.Length()); return obj.Back(); }
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.AddConstruct();
      ParseTOMLBase<T>(e, obj.Back(), s);
    }
    static void F(cSerializer<TOMLEngine>& e, cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      ParseTOMLArray<cDynArray<T, CType, ArrayType, Alloc>>(e, obj, s);
    }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseTOMLInternal<cArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline T& GetLast(cArray<T, CType, ArrayType, Alloc>& obj) { assert(obj.Capacity()); return obj.Back(); }
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.SetCapacity(obj.Capacity() + 1);
      ParseTOMLBase<T>(e, obj.Back(), s);
    }
    static void F(cSerializer<TOMLEngine>& e, cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      ParseTOMLArray<cArray<T, CType, ArrayType, Alloc>>(e, obj, s);
    }
  };
  template<class T, size_t I, bool B> // For fixed-length arrays
  struct ParseTOMLInternal<T[I], B>
  {
    static inline T& GetLast(T(&obj)[I]) { return obj[0]; }
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, T(&obj)[I], std::istream& s, int& n) { if(n<I) ParseTOMLBase<T>(e, obj[n++], s); }
    static void F(cSerializer<TOMLEngine>& e, T(&obj)[I], std::istream& s) { ParseTOMLArray<T[I]>(e, obj, s); }
  };
  template<class T, size_t I, bool B> // For fixed-length arrays
  struct ParseTOMLInternal<std::array<T,I>, B>
  {
    static inline T& GetLast(std::array<T, I>& obj) { return obj[0]; }
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, std::array<T, I>& obj, std::istream& s, int& n) { if(n<I) ParseTOMLBase<T>(e, obj[n++], s); }
    static void F(cSerializer<TOMLEngine>& e, std::array<T, I>& obj, std::istream& s) { ParseTOMLArray<std::array<T, I>>(e, obj, s); }
  };
  template<class T, typename Alloc>
  struct ParseTOMLInternal<std::vector<T, Alloc>, false>
  {
    static inline T& GetLast(std::vector<T, Alloc>& obj) { assert(obj.size()); return obj.back(); }
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int& n)
    {
      obj.push_back(T());
      ParseTOMLBase<T>(e, obj.back(), s);
    }
    static void F(cSerializer<TOMLEngine>& e, std::vector<T, Alloc>& obj, std::istream& s)
    {
      ParseTOMLArray<std::vector<T, Alloc>>(e, obj, s);
    }
  };

  template<class T>
  inline void static ParseTOMLBase(cSerializer<TOMLEngine>& e, T& obj, std::istream& s) {
    ParseTOMLInternal<T, std::is_arithmetic<T>::value>::F(e, obj, s);
  }
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<std::string>(cSerializer<TOMLEngine>& e, std::string& target, std::istream& s) { ParseTOMLString<true, 0, 0>(target, s); }
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<cStr>(cSerializer<TOMLEngine>& e, cStr& target, std::istream& s) { ParseTOMLBase<std::string>(e, target, s); }
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<bool>(cSerializer<TOMLEngine>& e, bool& target, std::istream& s)
  {
    static const char* val = "true";
    ParseTOMLEatWhitespace(s);
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

#ifdef BSS_COMPILER_HAS_TIME_GET
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<std::chrono::system_clock::time_point>(cSerializer<TOMLEngine>& e, std::chrono::system_clock::time_point& target, std::istream& s)
  {
    using days = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
    static const char DATE[] = "%Y-%m-%d";
    static const char TIME[] = "%H:%M:%S";
    static const char ZONE[] = "%H:%M";
    const std::time_get<char>& tg = std::use_facet<std::time_get<char>>(s.getloc());
    std::tm t = { 0 };
    std::ios_base::iostate err = std::ios_base::goodbit;
    ParseTOMLEatWhitespace(s);
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
              offset = (sign == '+' ? 1 : -1) * (std::chrono::hours { tz.tm_hour } +std::chrono::minutes { tz.tm_min });

            target = std::chrono::system_clock::time_point {
              days { days_from_civil(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday) } +
              std::chrono::hours { t.tm_hour } +std::chrono::minutes { t.tm_min } +std::chrono::seconds { t.tm_sec } -
              offset };
            return;
          }
          else if(sign == 'Z')
            s.get();
        }
      }

      target = std::chrono::system_clock::time_point { days { days_from_civil(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday) } };
    }
  }
#endif

  // This function parses values and is recursive, allowing for nested inline tables or arrays, etc.
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<TOMLValue>(cSerializer<TOMLEngine>& e, TOMLValue& target, std::istream& s)
  {
    auto f = [&target](cSerializer<TOMLEngine>& e, const char* id) {
      auto& table = target.get<TOMLValue::TOMLTable>();
      TOMLValue* v = table[id];
      if(!v)
        v = table.GetValue(table.Insert(id, TOMLValue(TOMLValue::TOMLTable())));
      if(v)
        ParseTOMLBase<TOMLValue>(e, *v, *e.in);
    };
    switch(e.engine.state)
    {
    case 3:
      target = TOMLValue::TOMLTable();
      ParseTOMLRoot(e, *e.in, f);
      return;
    case 1:
      if(e.in->peek() == '.')
      {
        e.in->get();
        cStr buf;
        ParseTOMLString<false, '.', ']'>(buf, *e.in);
        TOMLValue* v = target.get<TOMLValue::TOMLTable>()[buf];
        if(v)
          ParseTOMLBase<TOMLValue>(e, *v, *e.in);
      }
      else if(e.in->peek() == ']')
      {
        e.in->get();
        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      return;
    case 2:
      if(e.in->peek() == ']')
      {
        e.in->get();
        if(e.in->peek() == ']')
          e.in->get();
        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      return;
    case 0:
      break;
    }

    ParseTOMLEatWhitespace(s);
    switch(s.peek())
    {
    case '{': // inline table
      s.get();
      target = TOMLValue::TOMLTable();
      ParseTOMLTable(e, *e.in, f);
      break;
    case '[': // array
      target = TOMLValue::TOMLArray();
      ParseTOMLInternal<TOMLValue::TOMLArray, false>::F(e, target.get<TOMLValue::TOMLArray>(), s);
      break;
    case '"': // Single or multi-line string
    case '\'': // Single or multi-line literal string
      target = cStr();
      ParseTOMLBase<cStr>(e, target.get<cStr>(), s);
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
      ParseTOMLBase<double>(e, target.get<double>(), s);
      double ipart;
      if(modf(target.get<double>(), &ipart) == 0.0)
        target = (int64_t)ipart;
      break;
    case 't':
    case 'f':
      target = false;
      ParseTOMLBase<bool>(e, target.get<bool>(), s);
      break;
    }
  }

  template<class T>
  inline static void ParseTOML(T& obj, std::istream& s) { cSerializer<TOMLEngine> e; e.Parse(obj, s, 0); }
  template<class T>
  inline static void ParseTOML(T& obj, const char* s) { std::istringstream ss(s); ParseTOML<T>(obj, ss); }

  template<typename T>
  void TOMLEngine::Parse(cSerializer<TOMLEngine>& e, T& t, const char* id) {
    ParseTOMLBase<T>(e, t, *e.in);
  }

  template<typename... Args>
  void TOMLEngine::ParseMany(cSerializer<TOMLEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args)
  {
    auto f = [&t, &args](cSerializer<TOMLEngine>& e, const char* id) { cSerializer<TOMLEngine>::_findparse<Args...>(e, id, t, args); };
    switch(e.engine.state)
    {
    case 3: // If state is 3, this is the initial object serialization.
      ParseTOMLRoot(e, *e.in, f);
      break;
    case 1:
      if(e.in->peek() == '.')
      {
        e.in->get();
        cStr buf;
        ParseTOMLString<false, '.', ']'>(buf, *e.in);
        cSerializer<TOMLEngine>::_findparse<Args...>(e, buf.c_str(), t, args);
      }
      else if(e.in->peek() == ']')
      {
        e.in->get();
        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      break;
    case 2:
      if(e.in->peek() == ']')
      {
        e.in->get();
        if(e.in->peek() == ']')
          e.in->get();
        ParseTOMLPairs(e, *e.in, f);
      }
      else
        assert(false);
      break;
    case 0:
      ParseTOMLEatWhitespace(*e.in);
      if(e.in->peek() == '{')
      {
        e.in->get();
        ParseTOMLTable(e, *e.in, f);
      }
      break;
    }
  }

  static void WriteTOMLId(cSerializer<TOMLEngine>& e, const char* id, std::ostream& s) {
    if(!id || e.engine.state == 2)
    {
      if(e.engine.first)
        e.engine.first = false;
      else
        s << ", ";
    }
    if(id) s << id << " = ";
  }

  template<class T>
  inline static void WriteTOMLTables(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
  {
    cStr last = e.engine.id;
    if(e.engine.id.size())
      e.engine.id += '.';
    e.engine.id += id;
    const_cast<T&>(obj).template Serialize<TOMLEngine>(e);
    e.engine.id = last;
  }

  template<class T>
  inline static void WriteTOMLArray(cSerializer<TOMLEngine>& e, const char* id, const T* obj, size_t size, std::ostream& s)
  {
    if(e.engine.state == 1) return;
    WriteTOMLId(e, id, s);
    s << '[';
    e.engine.first = true;
    for(uint32_t i = 0; i < size; ++i)
      WriteTOMLBase(e, 0, obj[i], s);
    s << ']';
    if(e.engine.state != 2 && id) s << std::endl;
  }

  template<class T>
  inline static void WriteTOMLPairs(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
  {
    if(e.engine.state == 2) // if we're in state 2 we're writing an inline table
    {
      WriteTOMLId(e, id, s);
      s << "{ ";
    }
    else if(id)
    {
      s << '[';
      if(e.engine.id.size())
        s << e.engine.id << '.';
      s << id << ']' << std::endl;
    }

    e.engine.first = true;
    const_cast<T&>(obj).template Serialize<TOMLEngine>(e);

    if(e.engine.state == 2)
      s << " }";
    else
      s << std::endl;
  }

  template<class T, bool B>
  struct WriteTOMLInternal
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
    {
      if(e.engine.state == 2 || (!id && !e.engine.state))
      {
        uint32_t state = e.engine.state;
        e.engine.state = 2;
        WriteTOMLPairs(e, id, obj, s);
        e.engine.state = state;
      }
      else if(!!e.engine.state)
      {
        e.engine.state = 0;
        WriteTOMLPairs(e, id, obj, s);
        e.engine.state = 1;
        WriteTOMLTables(e, id, obj, s);
        e.engine.state = 1;
      }
    }
  };

  template<class T>
  struct WriteTOMLInternal<T, true>
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s)
    {
      if(e.engine.state == 1) return;
      WriteTOMLId(e, id, s);
      s << obj;
      if(e.engine.state != 2 && id) s << std::endl;
    }
  };

  template<class T, size_t I, bool B>
  struct WriteTOMLInternal<T[I], B>
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const T(&obj)[I], std::ostream& s) { WriteTOMLArray<T>(e, id, obj, I, s); }
  };
  template<class T, size_t I, bool B>
  struct WriteTOMLInternal<std::array<T, I>, B>
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const std::array<T, I>& obj, std::ostream& s) { WriteTOMLArray<T>(e, id, obj.data(), I, s); }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteTOMLInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const cDynArray<T, CType, ArrayType, Alloc>& obj, std::ostream& s) { WriteTOMLArray<T>(e, id, obj, obj.Length(), s); }
  };
  template<class T, typename Alloc>
  struct WriteTOMLInternal<std::vector<T, Alloc>, false>
  {
    static void F(cSerializer<TOMLEngine>& e, const char* id, const std::vector<T, Alloc>& obj, std::ostream& s) { WriteTOMLArray<T>(e, id, obj.data(), obj.size(), s); }
  };

  template<class T>
  void static WriteTOMLBase(cSerializer<TOMLEngine>& e, const char* id, const T& obj, std::ostream& s) { WriteTOMLInternal<T, std::is_arithmetic<T>::value>::F(e, id, obj, s); }

  template<>
  BSS_EXPLICITSTATIC void WriteTOMLBase<std::string>(cSerializer<TOMLEngine>& e, const char* id, const std::string& obj, std::ostream& s)
  {
    if(e.engine.state == 1) return;
    WriteTOMLId(e, id, s);
    s << '"';
    for(uint32_t i = 0; i < obj.size(); ++i)
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
    if(e.engine.state != 2 && id) s << std::endl;
  }

  template<>
  BSS_EXPLICITSTATIC void WriteTOMLBase<cStr>(cSerializer<TOMLEngine>& e, const char* id, const cStr& obj, std::ostream& s) { WriteTOMLBase<std::string>(e, id, obj, s); }
  template<>
  BSS_EXPLICITSTATIC void WriteTOMLBase<bool>(cSerializer<TOMLEngine>& e, const char* id, const bool& obj, std::ostream& s)
  { 
    if(e.engine.state == 1) return;
    WriteTOMLId(e, id, s);
    s << (obj ? "true" : "false");
    if(e.engine.state != 2 && id) s << std::endl;
  }
#ifdef BSS_COMPILER_HAS_TIME_GET
  template<>
  BSS_EXPLICITSTATIC void WriteTOMLBase<std::chrono::system_clock::time_point>(cSerializer<TOMLEngine>& e, const char* id, const std::chrono::system_clock::time_point& obj, std::ostream& s)
  {
    if(e.engine.state == 1) return;
    WriteTOMLId(e, id, s);
    time_t time = std::chrono::system_clock::to_time_t(obj);
    s << std::put_time(gmtime(&time), "%Y-%m-%dT%H:%M:%S+00:00");
    if(e.engine.state != 2 && id) s << std::endl;
  }
#endif

  template<class T>
  inline static void WriteTOML(const T& obj, std::ostream& s) { cSerializer<TOMLEngine> e; e.engine.state = 1; e.Serialize<T>(obj, s, 0); }
  template<class T>
  inline static void WriteTOML(const T& obj, const char* file) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteTOML<T>(obj, fs); }

  template<typename T>
  void TOMLEngine::Serialize(cSerializer<TOMLEngine>& e, const T& t, const char* id) {
    WriteTOMLBase<T>(e, id, t, *e.out);
  }
}




#endif