// Copyright ©2016 Black Sphere Studios
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

namespace bss_util {
  class TOMLEngine
  {
  public:
    TOMLEngine() : state(0) {}
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static void Serialize(cSerializer<TOMLEngine>& e, const T& t, const char* id);
    template<typename T>
    static void Parse(cSerializer<TOMLEngine>& e, T& t, const char* id);
    template<typename... Args>
    static void ParseMany(cSerializer<TOMLEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args);

    uint32_t state;
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
    void Serialize(cSerializer<Engine>& s)
    {
      assert(BASE::template is<TOMLTable>());
      if(s.out)
      {
        auto& v = BASE::template get<TOMLTable>();
        for(auto& e : v)
          WriteTOMLBase<TOMLTable>(s, v.GetKey(e).c_str(), v.GetValue(e), *s.out, s.engine.pretty);
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
    if(e.engine.state == 2) // If the state is 2 and not 0, this is a table array.
      ParseTOMLInternal<T, false>::DoAddCall(e, obj, s, n);
    else // Otherwise it's a standard inline array
    {
      ParseTOMLEatWhitespace(s);
      if(!s || s.get() != '[') return;
      ParseTOMLEatWhitespace(s);
      while(!!s && s.peek() != ']' && s.peek() != -1)
      {
        ParseTOMLInternal<T, false>::DoAddCall(e, obj, s, n);
        while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
        if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
        ParseTOMLEatWhitespace(s);
      }
    }
  }

  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseTOMLInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
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
  template<class T, int I, bool B> // For fixed-length arrays
  struct ParseTOMLInternal<T[I], B>
  {
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, T(&obj)[I], std::istream& s, int& n) { if(n<I) ParseTOMLBase<T>(e, obj[n++], s); }
    static void F(cSerializer<TOMLEngine>& e, T(&obj)[I], std::istream& s) { ParseTOMLArray<T[I]>(e, obj, s); }
  };
  template<class T, int I, bool B> // For fixed-length arrays
  struct ParseTOMLInternal<std::array<T,I>, B>
  {
    static inline void DoAddCall(cSerializer<TOMLEngine>& e, std::array<T, I>& obj, std::istream& s, int& n) { if(n<I) ParseTOMLBase<T>(e, obj[n++], s); }
    static void F(cSerializer<TOMLEngine>& e, std::array<T, I>& obj, std::istream& s) { ParseTOMLArray<std::array<T, I>>(e, obj, s); }
  };
  template<class T, typename Alloc>
  struct ParseTOMLInternal<std::vector<T, Alloc>, false>
  {
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

  // This function parses values and is recursive, allowing for nested inline tables or arrays, etc.
  template<>
  inline void BSS_EXPLICITSTATIC ParseTOMLBase<TOMLValue>(cSerializer<TOMLEngine>& e, TOMLValue& target, std::istream& s)
  {
    cStr buf;
    ParseTOMLEatWhitespace(s);
    switch(s.peek())
    {
    case '{': // inline table
      target = TOMLValue::TOMLTable();
      //ParseTOMLInternal<TOMLValue::TOMLTable, false>::F(e, target.get<TOMLValue::TOMLTable>(), s);
      break;
    case '[': // array
      target = TOMLValue::TOMLArray();
      //ParseJSONInternal<TOMLValue::TOMLArray, false>::F(e, target.get<TOMLValue::TOMLArray>(), s);
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
      ParseTOMLBase<cStr>(e, buf, s);
      if(strchr(buf.c_str(), '.') || strchr(buf.c_str(), 'e') || strchr(buf.c_str(), 'E'))
        target = (double)strtod(buf.c_str(), 0);
      else
        target = (int64_t)strtoll(buf.c_str(), 0, 10);
    break;
    case 't':
      if(strcmp(buf.c_str(), "true"))
        target = true;
      break;
    case 'f':
      if(strcmp(buf.c_str(), "false"))
        target = false;
      break;
    }
  }

  template<class T>
  inline static void ParseTOML(T& obj, std::istream& s) { cSerializer<TOMLEngine> e(s); e.engine.state = 3; obj.template Serialize<TOMLEngine>(e); }
  template<class T>
  inline static void ParseTOML(T& obj, const char* s) { std::istringstream ss(s); ParseTOML<T>(obj, ss); }

  // This is the primary parsing function valid only for the root of the document. It only parses [tables] and [[table arrays]].
  template<typename... Args>
  inline void BSS_EXPLICITSTATIC ParseTOMLRoot(cSerializer<TOMLEngine>& e, std::istream& s, const cTrie<uint16_t>& t, std::tuple<Args...>& args)
  {
    cStr buf;
    ParseTOMLPairs<Args...>(e, s, t, args); // First we parse any root-level key value pairs directly into our target object
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
        cSerializer<TOMLEngine>::_findparse<Args...>(e, buf.c_str(), t, args);
      while(!!s && s.peek() != '[' && s.peek() != -1) s.get(); // Eat all remaining characters until the next [
      ParseTOMLEatAllspace(s);
    }
  }

  template<typename... Args>
  inline void BSS_EXPLICITSTATIC ParseTOMLTable(cSerializer<TOMLEngine>& e, std::istream& s, const cTrie<uint16_t>& t, std::tuple<Args...>& args)
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
        cSerializer<TOMLEngine>::_findparse<Args...>(e, buf.c_str(), t, args); // This will call ParseTOMLBase on the appropriate value.
      }
      while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) s.get(); // Eat everything until the next comma or the end of the table
      if(s.peek() == ',') s.get();
    }
    if(s.peek() == '}') s.get();
  }

  template<typename... Args>
  inline void BSS_EXPLICITSTATIC ParseTOMLPairs(cSerializer<TOMLEngine>& e, std::istream& s, const cTrie<uint16_t>& t, std::tuple<Args...>& args)
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
        cSerializer<TOMLEngine>::_findparse<Args...>(e, buf.c_str(), t, args); // This will call ParseTOMLBase on the appropriate value.
        while(!!s && s.peek() != '\n' && s.peek() != '\r' && s.peek() != -1) s.get(); // Eat all remaining characters on this line that weren't parsed.
      }
      ParseTOMLEatAllspace(s);
    }
  }

  template<typename T>
  void TOMLEngine::Parse(cSerializer<TOMLEngine>& e, T& t, const char* id) {
    ParseTOMLBase<T>(e, t, *e.in);
  }

  template<typename... Args>
  void TOMLEngine::ParseMany(cSerializer<TOMLEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args)
  {
    switch(e.engine.state)
    {
    case 3: // If state is 3, this is the initial object serialization.
      ParseTOMLRoot<Args...>(e, *e.in, t, args);
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
        ParseTOMLPairs<Args...>(e, *e.in, t, args);
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
        ParseTOMLPairs<Args...>(e, *e.in, t, args);
      }
      else
        assert(false);
      break;
    case 0:
      ParseTOMLEatWhitespace(*e.in);
      if(e.in->peek() == '{')
      {
        e.in->get();
        ParseTOMLTable(e, *e.in, t, args);
      }
      break;
    }
  }

  template<typename T>
  void TOMLEngine::Serialize(cSerializer<TOMLEngine>& e, const T& t, const char* id) {}
}




#endif