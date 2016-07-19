// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_JSON_H__BSS__
#define __C_JSON_H__BSS__

#include "cDynArray.h"
#include "cStr.h"
#include "bss_util.h"
#include "variant.h"
#include "cSerializer.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <utility>

namespace bss_util {
  class JSONEngine
  {
  public:
    JSONEngine() : pretty(0) {}
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static void Serialize(cSerializer<JSONEngine>& e, const T& t, const char* id);
    template<typename T>
    static void Parse(cSerializer<JSONEngine>& e, T& t, const char* id);
    template<typename... Args>
    static void ParseMany(cSerializer<JSONEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args);

    uint32_t pretty;
  };

  template<class T>
  void static ParseJSONBase(cSerializer<JSONEngine>& e, T& obj, std::istream& s);

  struct JSONValue;
  template<>
  void BSS_EXPLICITSTATIC ParseJSONBase<JSONValue>(cSerializer<JSONEngine>& e, JSONValue& target, std::istream& s);

  template<class T>
  void static WriteJSONBase(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty);

  template<>
  void BSS_EXPLICITSTATIC WriteJSONBase<JSONValue>(cSerializer<JSONEngine>& e, const char* id, const JSONValue& obj, std::ostream& s, uint32_t& pretty);

  template<class T, class BASE>
  struct __JSONValue__conv
  {
    inline static constexpr T&& f(typename std::remove_reference<T>::type& r) { return (static_cast<T&&>(r)); }
    inline static constexpr T&& f(typename std::remove_reference<T>::type&& r) { return (static_cast<T&&>(r)); }
  };
  template<class BASE>
  struct __JSONValue__conv<JSONValue, BASE>
  {
    inline static constexpr BASE&& f(std::remove_reference<JSONValue>::type& r) { return (static_cast<BASE&&>(r)); }
    inline static constexpr BASE&& f(std::remove_reference<JSONValue>::type&& r) { return (static_cast<BASE&&>(r)); }
  };
  template<class BASE>
  struct __JSONValue__conv<const JSONValue, BASE>
  {
    inline static constexpr const BASE&& f(std::remove_reference<const JSONValue>::type& r) { return (static_cast<const BASE&&>(r)); }
    inline static constexpr const BASE&& f(std::remove_reference<const JSONValue>::type&& r) { return (static_cast<const BASE&&>(r)); }
  };

  struct JSONValue : public variant<cStr, double, int64_t, bool, cDynArray<JSONValue, size_t, CARRAY_SAFE>, cDynArray<std::pair<cStr, JSONValue>, size_t, CARRAY_SAFE>>
  {
    typedef cDynArray<JSONValue, size_t, CARRAY_SAFE> JSONArray;
    typedef cDynArray<std::pair<cStr, JSONValue>, size_t, CARRAY_SAFE> JSONObject;
    typedef variant<cStr, double, int64_t, bool, JSONArray, JSONObject> BASE;

  public:
    JSONValue() : BASE() {}
    JSONValue(const BASE& v) : BASE(v) {}
    JSONValue(BASE&& v) : BASE(std::move(v)) {}
    template<typename T>
    explicit JSONValue(const T& t) : BASE(t) {}
    template<typename T>
    explicit JSONValue(T&& t) : BASE(__JSONValue__conv<T, BASE>::f(t)) {}
    ~JSONValue() { }
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(__JSONValue__conv<T, BASE>::f(right)); return *this; }

    template<typename Engine>
    void Serialize(cSerializer<Engine>& s)
    {
      assert(BASE::template is<JSONObject>());
      if(s.out)
      {
        auto& v = BASE::template get<JSONObject>();
        for(auto& e : v)
          WriteJSONBase<JSONValue>(s, e.first.c_str(), e.second, *s.out, s.engine.pretty);
      }
    }
  };

  static inline void ParseJSONEatWhitespace(std::istream& s) { while(!!s && isspace(s.peek())) s.get(); }
  static inline void ParseJSONEatCharacter(std::string& str, std::istream& src) // this processes escape characters when reading through strings
  {
    char c = src.get();
    if(c != '\\')
    {
      str += c;
      return;
    }
    switch(src.get())
    {
    case '"': str += '"'; break;
    case '\\': str += '\\'; break;
    case '/': str += '/'; break;
    case 'b': str += '\b'; break;
    case 'f': str += '\f'; break;
    case 'n': str += '\n'; break;
    case 'r': str += '\r'; break;
    case 't': str += '\t'; break;
    case 'u':
      char buf[5];
      for(int i = 0; i < 4; ++i) buf[i] = src.get();
      buf[4] = 0;
      OutputUnicode(str, strtoul(buf, 0, 16));
      break;
    }
  }

  template<typename F>
  static void ParseJSONObject(cSerializer<JSONEngine>& e, F f) {
    std::istream& s = *e.in;
    cStr buf;
    ParseJSONEatWhitespace(s);
    if(!s || s.get() != '{') return;
    ParseJSONEatWhitespace(s);
    while(!!s && s.peek() != '}' && s.peek() != -1)
    {
      if(!s || s.get() != '"' || s.peek() == -1) continue;
      buf.clear(); // clear buffer to hold name
      while(!!s && s.peek() != '"' && s.peek() != -1) ParseJSONEatCharacter(buf, s);
      if(s) s.get(); // eat " character
      ParseJSONEatWhitespace(s);
      if(s.get() != ':') continue;
      ParseJSONEatWhitespace(s);
      f(e, buf.c_str());
      //cSerializer<JSONEngine>::_findparse(e, buf.c_str(), t, args);
      while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) s.get(); // eat everything up to a , or } character
      if(!!s && s.peek() == ',') s.get(); // Only eat comma if it's there.
      ParseJSONEatWhitespace(s);
    }
    if(s.peek() == '}') s.get(); // eat the closing brace
  }
  
  template<typename T>
  void JSONEngine::Parse(cSerializer<JSONEngine>& e, T& t, const char* id) { ParseJSONBase<T>(e, t, *e.in); }
  template<typename... Args>
  void JSONEngine::ParseMany(cSerializer<JSONEngine>& e, const cTrie<uint16_t>& t, std::tuple<Args...>& args) {
    ParseJSONObject(e, [&t, &args](cSerializer<JSONEngine>& e, const char* id) { cSerializer<JSONEngine>::_findparse(e, id, t, args); });
  }

  template<class T, bool B>
  struct ParseJSONInternal
  {
    static void F(cSerializer<JSONEngine>& e, T& obj, std::istream& s) { obj.template Serialize<JSONEngine>(e); }
  };
  template<>
  struct ParseJSONInternal<JSONValue::JSONObject, false>
  {
    static void F(cSerializer<JSONEngine>& e, JSONValue::JSONObject& obj, std::istream& s) {
      ParseJSONObject(e, [&obj](cSerializer<JSONEngine>& e, const char* id) {
        std::pair<cStr, JSONValue> pair;
        pair.first = id;
        ParseJSONBase<JSONValue>(e, pair.second, *e.in);
        obj.Add(pair);
      });
    }
  };
  template<class T> // For arithmetic base types
  struct ParseJSONInternal<T, true>
  {
    static void F(cSerializer<JSONEngine>& e, T& obj, std::istream& s)
    {
      if(s.peek() == ',' || s.peek() == ']' || s.peek() == '}') return;
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
  void ParseJSONArray(cSerializer<JSONEngine>& e, T& obj, std::istream& s)
  {
    ParseJSONEatWhitespace(s);
    if(!s || s.get() != '[') return;
    ParseJSONEatWhitespace(s);
    int n = 0;
    while(!!s && s.peek() != ']' && s.peek() != -1)
    {
      ParseJSONInternal<T, false>::DoAddCall(e, obj, s, n);
      while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
      if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
      ParseJSONEatWhitespace(s);
    }
  }
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline void DoAddCall(cSerializer<JSONEngine>& e, cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.Add(T());
      ParseJSONBase<T>(e, obj.Back(), s);
    }
    static void F(cSerializer<JSONEngine>& e, cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.Clear();
      ParseJSONArray<cDynArray<T, CType, ArrayType, Alloc>>(e, obj, s);
    }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline void DoAddCall(cSerializer<JSONEngine>& e, cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.SetCapacity(obj.Capacity() + 1);
      ParseJSONBase<T>(e, obj.Back(), s);
    }
    static void F(cSerializer<JSONEngine>& e, cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.SetCapacity(0);
      ParseJSONArray<cArray<T, CType, ArrayType, Alloc>>(e, obj, s);
    }
  };
  template<class T, int I, bool B> // For fixed-length arrays
  struct ParseJSONInternal<T[I], B>
  {
    static inline void DoAddCall(cSerializer<JSONEngine>& e, T(&obj)[I], std::istream& s, int& n) { if(n<I) ParseJSONBase<T>(e, obj[n++], s); }
    static void F(cSerializer<JSONEngine>& e, T(&obj)[I], std::istream& s) { ParseJSONArray<T[I]>(e, obj, s); }
  };
  template<class T, typename Alloc>
  struct ParseJSONInternal<std::vector<T, Alloc>, false>
  {
    static inline void DoAddCall(cSerializer<JSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int& n)
    {
      obj.push_back(T());
      ParseJSONBase<T>(e, obj.back(), s);
    }
    static void F(cSerializer<JSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s)
    {
      obj.clear();
      ParseJSONArray<std::vector<T, Alloc>>(e, obj, s);
    }
  };
  template<>
  struct ParseJSONInternal<JSONValue::JSONArray, false>
  {
    static inline void DoAddCall(cSerializer<JSONEngine>& e, JSONValue::JSONArray& obj, std::istream& s, int& n) { obj.SetLength(obj.Length() + 1); ParseJSONBase<JSONValue>(e, obj.Back(), s); }
    static void F(cSerializer<JSONEngine>& e, JSONValue::JSONArray& obj, std::istream& s) { obj.Clear(); ParseJSONArray<JSONValue::JSONArray>(e, obj, s); }
  };

  template<class T>
  inline void static ParseJSONBase(cSerializer<JSONEngine>& e, T& obj, std::istream& s) { ParseJSONInternal<T, std::is_arithmetic<T>::value>::F(e, obj, s); }
  template<class T>
  inline static void ParseJSON(T& obj, std::istream& s) { cSerializer<JSONEngine> e; e.Parse<T>(obj, s, 0); }
  template<class T>
  inline static void ParseJSON(T& obj, const char* s) { std::istringstream ss(s); ParseJSON<T>(obj, ss); }

  template<>
  inline void BSS_EXPLICITSTATIC ParseJSONBase<std::string>(cSerializer<JSONEngine>& e, std::string& target, std::istream& s)
  {
    target.clear();
    if(s.peek() == ',' || s.peek() == ']' || s.peek() == '}') return;
    char c = s.get();
    if(c != '"')
    {
      target += c;
      while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != ']' && s.peek() != -1) target += s.get();
      return;
    }

    while(!!s && s.peek() != '"' && s.peek() != -1)
      ParseJSONEatCharacter(target, s);
    s.get(); // eat last " character
  }
  template<>
  inline void BSS_EXPLICITSTATIC ParseJSONBase<cStr>(cSerializer<JSONEngine>& e, cStr& target, std::istream& s) { ParseJSONBase<std::string>(e, target, s); }
  template<>
  inline void BSS_EXPLICITSTATIC ParseJSONBase<bool>(cSerializer<JSONEngine>& e, bool& target, std::istream& s)
  {
    static const char* val = "true";
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
  inline void BSS_EXPLICITSTATIC ParseJSONBase<JSONValue>(cSerializer<JSONEngine>& e, JSONValue& target, std::istream& s)
  {
    ParseJSONEatWhitespace(s);
    switch(s.peek())
    {
    case '{':
      target = JSONValue::JSONObject();
      ParseJSONInternal<JSONValue::JSONObject, false>::F(e, target.get<JSONValue::JSONObject>(), s);
      break;
    case '[':
      target = JSONValue::JSONArray();
      ParseJSONInternal<JSONValue::JSONArray, false>::F(e, target.get<JSONValue::JSONArray>(), s);
      break;
    case '"':
      target = cStr();
      ParseJSONBase<cStr>(e, target.get<cStr>(), s);
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
    {
      cStr buf;
      ParseJSONBase<cStr>(e, buf, s);
      const char* dot = strchr(buf.c_str(), '.');
      if(dot)
        target = (double)strtod(buf.c_str(), 0);
      else
        target = (int64_t)strtoll(buf.c_str(), 0, 10);
    }
    break;
    default:
    {
      cStr buf;
      ParseJSONBase<cStr>(e, buf, s);
      if(!STRICMP(buf.c_str(), "true")) target = true;
      else if(!STRICMP(buf.c_str(), "false")) target = false;
      else if(!STRICMP(buf.c_str(), "null")) break;
      else target = buf;
      break;
    }
    }
  }

  static bool WriteJSONIsPretty(uint32_t pretty) { return (pretty&(~0x80000000))>0; }
  static void WriteJSONTabs(std::ostream& s, uint32_t pretty)
  {
    uint32_t count = (pretty&(~0x80000000)) - 1;
    for(uint32_t i = 0; i < count; ++i)
      s << '\t';
  }

  static void WriteJSONId(const char* id, std::ostream& s, uint32_t pretty)
  {
    if(id)
    {
      if(WriteJSONIsPretty(pretty))
      {
        s << std::endl;
        WriteJSONTabs(s, pretty);
      }
      s << '"' << id << '"' << ": ";
    }
  }

  static void WriteJSONComma(std::ostream& s, uint32_t& pretty)
  {
    if(pretty & 0x80000000) s << ',';
    pretty |= 0x80000000;
  }

  static uint32_t WriteJSONPretty(uint32_t pretty) { return (pretty&(~0x80000000)) + ((pretty&(~0x80000000))>0); }

  template<class T>
  static void WriteJSONObject(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty)
  {
    //static_assert(HAS_MEMBER(T, SerializeJSON), "T must implement void SerializeJSON(std::ostream&, uint32_t&) const");
    WriteJSONComma(s, pretty);
    if(!id && WriteJSONIsPretty(pretty)) s << std::endl;
    WriteJSONId(id, s, pretty);
    if(!id && WriteJSONIsPretty(pretty)) WriteJSONTabs(s, pretty);
    s << '{';
    uint32_t oldpretty = pretty;
    e.engine.pretty = WriteJSONPretty(pretty);
    const_cast<T&>(obj).template Serialize<JSONEngine>(e);
    e.engine.pretty = oldpretty;
    if(WriteJSONIsPretty(pretty)) {
      s << std::endl;
      WriteJSONTabs(s, pretty);
    }
    s << '}';
    s.flush();
  }

  template<class T, bool B>
  struct WriteJSONInternal
  {
    static void F(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty) { WriteJSONObject<T>(e, id, obj, s, pretty); }
  };

  template<class T>
  struct WriteJSONInternal<T, true>
  {
    static void F(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty)
    {
      WriteJSONComma(s, pretty);
      WriteJSONId(id, s, pretty);
      s << obj;
    }
  };

  template<class T>
  void static WriteJSONBase(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty);

  template<class T>
  void WriteJSONArray(cSerializer<JSONEngine>& e, const char* id, const T* obj, size_t size, std::ostream& s, uint32_t& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    uint32_t npretty = WriteJSONPretty(pretty);
    s << '[';
    for(uint32_t i = 0; i < size; ++i)
      WriteJSONBase(e, 0, obj[i], s, npretty);
    s << ']';
  }

  template<class T, int I, bool B>
  struct WriteJSONInternal<T[I], B>
  {
    static void F(cSerializer<JSONEngine>& e, const char* id, const T(&obj)[I], std::ostream& s, uint32_t& pretty) { WriteJSONArray<T>(e, id, obj, I, s, pretty); }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteJSONInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static void F(cSerializer<JSONEngine>& e, const char* id, const cDynArray<T, CType, ArrayType, Alloc>& obj, std::ostream& s, uint32_t& pretty) { WriteJSONArray<T>(e, id, obj, obj.Length(), s, pretty); }
  };
  template<class T, typename Alloc>
  struct WriteJSONInternal<std::vector<T, Alloc>, false>
  {
    static void F(cSerializer<JSONEngine>& e, const char* id, const std::vector<T, Alloc>& obj, std::ostream& s, uint32_t& pretty) { WriteJSONArray<T>(e, id, obj.data(), obj.size(), s, pretty); }
  };

  // To enable pretty output, set pretty to 1. The upper two bits are used as flags, so if you need more than 1073741823 levels of indentation... what the hell are you doing?!
  template<class T>
  void static WriteJSONBase(cSerializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, uint32_t& pretty) { WriteJSONInternal<T, std::is_arithmetic<T>::value>::F(e, id, obj, s, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSONBase<std::string>(cSerializer<JSONEngine>& e, const char* id, const std::string& obj, std::ostream& s, uint32_t& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    s << '"';
    for(uint32_t i = 0; i < obj.size(); ++i)
    {
      switch(obj[i])
      {
      case '"': s << "\\\""; break;
      case '\\': s << "\\\\"; break;
      case '/': s << "\\/"; break;
      case '\b': s << "\\b"; break;
      case '\f': s << "\\f"; break;
      case '\n': s << "\\n"; break;
      case '\r': s << "\\r"; break;
      case '\t': s << "\\t"; break;
      default: s << obj[i]; break;
      }
    }
    s << '"';
  }

  template<>
  void BSS_EXPLICITSTATIC WriteJSONBase<cStr>(cSerializer<JSONEngine>& e, const char* id, const cStr& obj, std::ostream& s, uint32_t& pretty) { WriteJSONBase<std::string>(e, id, obj, s, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSONBase<bool>(cSerializer<JSONEngine>& e, const char* id, const bool& obj, std::ostream& s, uint32_t& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    s << (obj ? "true" : "false");
  }

  template<class T>
  void static WriteJSON(const T& obj, std::ostream& s, uint32_t pretty = 0) { cSerializer<JSONEngine> e; e.engine.pretty = pretty; e.Serialize<T>(obj, s, 0); }

  template<class T>
  void static WriteJSON(const T& obj, const char* file, uint32_t pretty = 0) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteJSON<T>(obj, fs, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSONBase<JSONValue>(cSerializer<JSONEngine>& e, const char* id, const JSONValue& obj, std::ostream& s, uint32_t& pretty)
  {
    switch(obj.tag())
    {
    case JSONValue::Type<cStr>::value: WriteJSONBase<cStr>(e, id, obj.get<cStr>(), s, pretty); break;
    case JSONValue::Type<double>::value: WriteJSONBase<double>(e, id, obj.get<double>(), s, pretty); break;
    case JSONValue::Type<int64_t>::value: WriteJSONBase<int64_t>(e, id, obj.get<int64_t>(), s, pretty); break;
    case JSONValue::Type<bool>::value: WriteJSONBase<bool>(e, id, obj.get<bool>(), s, pretty); break;
    case JSONValue::Type<JSONValue::JSONArray>::value:
      WriteJSONBase<JSONValue::JSONArray>(e, id, obj.get<JSONValue::JSONArray>(), s, pretty); break;
      break;
    case JSONValue::Type<JSONValue::JSONObject>::value:
      WriteJSONObject<JSONValue>(e, id, obj, s, pretty);
      break;
    }
  }

  template<typename T>
  void JSONEngine::Serialize(cSerializer<JSONEngine>& e, const T& t, const char* id)
  {
    WriteJSONBase<T>(e, id, t, *e.out, e.engine.pretty);
  }
}

#endif