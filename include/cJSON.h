// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_JSON_H__BSS__
#define __C_JSON_H__BSS__

#include "cDynArray.h"
#include "cStr.h"
#include "bss_util.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <utility>
#include "variant.h"

namespace bss_util {
  template<class T>
  inline void ParseJSON(T& obj, std::istream& s);

  struct JSONValue;
  template<>
  inline void ParseJSON<JSONValue>(JSONValue& target, std::istream& s);

  template<class T>
  void static WriteJSON(const char* id, const T& obj, std::ostream& s, unsigned int& pretty);

  template<>
  void BSS_EXPLICITSTATIC WriteJSON<JSONValue>(const char* id, const JSONValue& obj, std::ostream& s, unsigned int& pretty);

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

    void EvalJSON(const char* id, std::istream& s)
    {
      assert(BASE::template is<JSONObject>());
      std::pair<cStr, JSONValue> pair;
      pair.first = id;
      ParseJSON<JSONValue>(pair.second, s);
      BASE::template get<JSONObject>().Add(pair);
    }

    void SerializeJSON(std::ostream& s, unsigned int& pretty) const
    {
      assert(BASE::template is<JSONObject>());
      auto& v = BASE::template get<JSONObject>();
      for(auto& e : v)
        WriteJSON<JSONValue>(e.first.c_str(), e.second, s, pretty);
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

  DEFINE_MEMBER_CHECKER(EvalJSON);

  template<class T, bool B>
  struct ParseJSONInternal
  {
    static_assert(HAS_MEMBER(T, EvalJSON), "T must implement void EvalJSON(const char*, std::istream&)");
    static void F(T& obj, std::istream& s)
    {
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
        obj.EvalJSON(buf.c_str(), s);
        while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) s.get(); // eat everything up to a , or } character
        if(!!s && s.peek() == ',') s.get(); // Only eat comma if it's there.
        ParseJSONEatWhitespace(s);
      }
      if(s.peek() == '}') s.get(); // eat the closing brace
    }
  };
  template<class T> // For arithmetic base types
  struct ParseJSONInternal<T, true>
  {
    static void F(T& obj, std::istream& s)
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
  void ParseJSONArray(T& obj, std::istream& s)
  {
    ParseJSONEatWhitespace(s);
    if(!s || s.get() != '[') return;
    ParseJSONEatWhitespace(s);
    int n = 0;
    while(!!s && s.peek() != ']' && s.peek() != -1)
    {
      ParseJSONInternal<T, false>::DoAddCall(obj, s, n);
      while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
      if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
      ParseJSONEatWhitespace(s);
    }
  }
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline void DoAddCall(cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.Add(T());
      ParseJSON<T>(obj.Back(), s);
    }
    static void F(cDynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.Clear();
      ParseJSONArray<cDynArray<T, CType, ArrayType, Alloc>>(obj, s);
    }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cArray<T, CType, ArrayType, Alloc>, false>
  {
    static inline void DoAddCall(cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.SetCapacity(obj.Capacity() + 1);
      ParseJSON<T>(obj.Back(), s);
    }
    static void F(cArray<T, CType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.SetCapacity(0);
      ParseJSONArray<cArray<T, CType, ArrayType, Alloc>>(obj, s);
    }
  };
  template<class T, int I, bool B> // For fixed-length arrays
  struct ParseJSONInternal<T[I], B>
  {
    static inline void DoAddCall(T(&obj)[I], std::istream& s, int& n) { if(n<I) ParseJSON<T>(obj[n++], s); }
    static void F(T(&obj)[I], std::istream& s) { ParseJSONArray<T[I]>(obj, s); }
  };
  template<class T, typename Alloc>
  struct ParseJSONInternal<std::vector<T, Alloc>, false>
  {
    static inline void DoAddCall(std::vector<T, Alloc>& obj, std::istream& s, int& n)
    {
      obj.push_back(T());
      ParseJSON<T>(obj.back(), s);
    }
    static void F(std::vector<T, Alloc>& obj, std::istream& s)
    {
      obj.clear();
      ParseJSONArray<std::vector<T, Alloc>>(obj, s);
    }
  };
  template<>
  struct ParseJSONInternal<JSONValue::JSONArray, false>
  {
    static inline void DoAddCall(JSONValue::JSONArray& obj, std::istream& s, int& n) { obj.SetLength(obj.Length() + 1); ParseJSON<JSONValue>(obj.Back(), s); }
    static void F(JSONValue::JSONArray& obj, std::istream& s) { obj.Clear(); ParseJSONArray<JSONValue::JSONArray>(obj, s); }
  };

  template<class T>
  inline void ParseJSON(T& obj, std::istream& s) { ParseJSONInternal<T, std::is_arithmetic<T>::value>::F(obj, s); }
  template<class T>
  inline void ParseJSON(T& obj, const char* s) { std::istringstream ss(s); ParseJSON<T>(obj, ss); }

  template<>
  inline void ParseJSON<std::string>(std::string& target, std::istream& s)
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
  inline void ParseJSON<cStr>(cStr& target, std::istream& s) { ParseJSON<std::string>(target, s); }
  template<>
  inline void ParseJSON<bool>(bool& target, std::istream& s)
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
  inline void ParseJSON<JSONValue>(JSONValue& target, std::istream& s)
  {
    ParseJSONEatWhitespace(s);
    switch(s.peek())
    {
    case '{':
      target = JSONValue::JSONObject();
      ParseJSONInternal<JSONValue, false>::F(target, s);
      break;
    case '[':
      target = JSONValue::JSONArray();
      ParseJSONInternal<JSONValue::JSONArray, false>::F(target.get<JSONValue::JSONArray>(), s);
      break;
    case '"':
      target = cStr();
      ParseJSON<cStr>(target.get<cStr>(), s);
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
      ParseJSON<cStr>(buf, s);
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
      ParseJSON<cStr>(buf, s);
      if(!STRICMP(buf.c_str(), "true")) target = true;
      else if(!STRICMP(buf.c_str(), "false")) target = false;
      else if(!STRICMP(buf.c_str(), "null")) break;
      else target = buf;
      break;
    }
    }
  }

  DEFINE_MEMBER_CHECKER(SerializeJSON);

  static bool WriteJSONIsPretty(unsigned int pretty) { return (pretty&(~0x80000000))>0; }
  static void WriteJSONTabs(std::ostream& s, unsigned int pretty)
  {
    unsigned int count = (pretty&(~0x80000000)) - 1;
    for(unsigned int i = 0; i < count; ++i)
      s << '\t';
  }

  static void WriteJSONId(const char* id, std::ostream& s, unsigned int pretty)
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

  static void WriteJSONComma(std::ostream& s, unsigned int& pretty)
  {
    if(pretty & 0x80000000) s << ',';
    pretty |= 0x80000000;
  }

  static unsigned int WriteJSONPretty(unsigned int pretty) { return (pretty&(~0x80000000)) + ((pretty&(~0x80000000))>0); }

  template<class T>
  static void WriteJSONObject(const char* id, const T& obj, std::ostream& s, unsigned int& pretty)
  {
    static_assert(HAS_MEMBER(T, SerializeJSON), "T must implement void SerializeJSON(std::ostream&, unsigned int&) const");
    WriteJSONComma(s, pretty);
    if(!id && WriteJSONIsPretty(pretty)) s << std::endl;
    WriteJSONId(id, s, pretty);
    if(!id && WriteJSONIsPretty(pretty)) WriteJSONTabs(s, pretty);
    s << '{';
    unsigned int npretty = WriteJSONPretty(pretty);
    obj.SerializeJSON(s, npretty);
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
    static void F(const char* id, const T& obj, std::ostream& s, unsigned int& pretty) { WriteJSONObject<T>(id, obj, s, pretty); }
  };

  template<class T>
  struct WriteJSONInternal<T, true>
  {
    static void F(const char* id, const T& obj, std::ostream& s, unsigned int& pretty)
    {
      WriteJSONComma(s, pretty);
      WriteJSONId(id, s, pretty);
      s << obj;
    }
  };

  template<class T>
  void static WriteJSON(const char* id, const T& obj, std::ostream& s, unsigned int& pretty);

  template<class T>
  void WriteJSONArray(const char* id, const T* obj, size_t size, std::ostream& s, unsigned int& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    unsigned int npretty = WriteJSONPretty(pretty);
    s << '[';
    for(unsigned int i = 0; i < size; ++i)
      WriteJSON(0, obj[i], s, npretty);
    s << ']';
  }

  template<class T, int I, bool B>
  struct WriteJSONInternal<T[I], B>
  {
    static void F(const char* id, const T(&obj)[I], std::ostream& s, unsigned int& pretty) { WriteJSONArray<T>(id, obj, I, s, pretty); }
  };
  template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteJSONInternal<cDynArray<T, CType, ArrayType, Alloc>, false>
  {
    static void F(const char* id, const cDynArray<T, CType, ArrayType, Alloc>& obj, std::ostream& s, unsigned int& pretty) { WriteJSONArray<T>(id, obj, obj.Length(), s, pretty); }
  };
  template<class T, typename Alloc>
  struct WriteJSONInternal<std::vector<T, Alloc>, false>
  {
    static void F(const char* id, const std::vector<T, Alloc>& obj, std::ostream& s, unsigned int& pretty) { WriteJSONArray<T>(id, obj.data(), obj.size(), s, pretty); }
  };

  // To enable pretty output, set pretty to 1. The upper two bits are used as flags, so if you need more than 1073741823 levels of indentation... what the hell are you doing?!
  template<class T>
  void static WriteJSON(const char* id, const T& obj, std::ostream& s, unsigned int& pretty) { WriteJSONInternal<T, std::is_arithmetic<T>::value>::F(id, obj, s, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSON<std::string>(const char* id, const std::string& obj, std::ostream& s, unsigned int& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    s << '"';
    for(unsigned int i = 0; i < obj.size(); ++i)
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
  void BSS_EXPLICITSTATIC WriteJSON<cStr>(const char* id, const cStr& obj, std::ostream& s, unsigned int& pretty) { WriteJSON<std::string>(id, obj, s, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSON<bool>(const char* id, const bool& obj, std::ostream& s, unsigned int& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    s << (obj ? "true" : "false");
  }

  template<class T>
  void static WriteJSON(const T& obj, std::ostream& s, unsigned int pretty = 0) { WriteJSON<T>(0, obj, s, pretty); }

  template<class T>
  void static WriteJSON(const T& obj, const char* file, unsigned int pretty = 0) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteJSON<T>(0, obj, fs, pretty); }

  template<>
  void BSS_EXPLICITSTATIC WriteJSON<JSONValue>(const char* id, const JSONValue& obj, std::ostream& s, unsigned int& pretty)
  {
    switch(obj.tag())
    {
    case JSONValue::Type<cStr>::value: WriteJSON<cStr>(id, obj.get<cStr>(), s, pretty); break;
    case JSONValue::Type<double>::value: WriteJSON<double>(id, obj.get<double>(), s, pretty); break;
    case JSONValue::Type<int64_t>::value: WriteJSON<int64_t>(id, obj.get<int64_t>(), s, pretty); break;
    case JSONValue::Type<bool>::value: WriteJSON<bool>(id, obj.get<bool>(), s, pretty); break;
    case JSONValue::Type<JSONValue::JSONArray>::value:
      WriteJSON<JSONValue::JSONArray>(id, obj.get<JSONValue::JSONArray>(), s, pretty); break;
      break;
    case JSONValue::Type<JSONValue::JSONObject>::value:
      WriteJSONObject<JSONValue>(id, obj, s, pretty);
      break;
    }
  }
}

#endif