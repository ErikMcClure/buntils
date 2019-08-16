// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __JSON_H__BSS__
#define __JSON_H__BSS__

#include "DynArray.h"
#include "Str.h"
#include "bss_util.h"
#include "Variant.h"
#include "Serializer.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <utility>

namespace bss {
  class JSONEngine
  {
  public:
    JSONEngine() : pretty(0) {}
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<JSONEngine>& e) {}
    static void End(Serializer<JSONEngine>& e) {}
    template<typename T>
    static void Serialize(Serializer<JSONEngine>& e, const T& obj, const char* id);
    template<typename T>
    static void SerializeArray(Serializer<JSONEngine>& e, const T& obj, size_t size, const char* id);
    template<typename T, size_t... S>
    static void SerializeTuple(Serializer<JSONEngine>& e, const T& obj, const char* id, std::index_sequence<S...>);
    template<typename T>
    static void SerializeNumber(Serializer<JSONEngine>& e, T obj, const char* id);
    static void SerializeBool(Serializer<JSONEngine>& e, bool obj, const char* id)
    {
      WriteJSONComma(*e.out, e.engine.pretty);
      WriteJSONId(id, *e.out, e.engine.pretty);
      (*e.out) << (obj ? "true" : "false");
    }

    template<typename T>
    static void Parse(Serializer<JSONEngine>& e, T& obj, const char* id);
    template<typename T, typename E, void (*Add)(Serializer<JSONEngine>& e, T& obj, int& n), bool (*Read)(Serializer<JSONEngine>& e, T& obj, int64_t count)>
    static void ParseArray(Serializer<JSONEngine>& e, T& obj, const char* id);
    template<typename T>
    static void ParseNumber(Serializer<JSONEngine>& e, T& obj, const char* id);
    static void ParseBool(Serializer<JSONEngine>& e, bool& target, const char* id)
    {
      std::istream& s = *e.in;
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
    template<typename F>
    static void ParseMany(Serializer<JSONEngine>& e, F && f);

    static inline void ParseJSONEatWhitespace(std::istream& s) { while(!!s && isspace(s.peek())) s.get(); }
    static inline void ParseJSONEatCharacter(std::string& str, std::istream& src)
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

        for(int i = 0; i < 4; ++i)
          buf[i] = src.get();

        buf[4] = 0;
        OutputUnicode(str, strtoul(buf, 0, 16));
        break;
      }
    }
    template<typename F>
    static inline void ParseJSONObject(Serializer<JSONEngine>& e, F f);

    static inline bool WriteJSONIsPretty(size_t pretty) { return (pretty&(~JSONEngine::PRETTYFLAG)) > 0; }
    static inline void WriteJSONTabs(std::ostream& s, size_t pretty)
    {
      size_t count = (pretty&(~JSONEngine::PRETTYFLAG)) - 1;
      for(size_t i = 0; i < count; ++i)
        s << '\t';
    }

    static inline void WriteJSONId(const char* id, std::ostream& s, size_t pretty)
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

    static inline void WriteJSONComma(std::ostream& s, size_t& pretty)
    {
      if(pretty & JSONEngine::PRETTYFLAG)
        s << ',';
      pretty |= JSONEngine::PRETTYFLAG;
    }

    static inline size_t WriteJSONPretty(size_t pretty) { return (pretty&(~JSONEngine::PRETTYFLAG)) + ((pretty&(~JSONEngine::PRETTYFLAG)) > 0); }

    template<class T>
    static void WriteJSONObject(Serializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, size_t& pretty);

    size_t pretty;

    static const size_t PRETTYFLAG = size_t(1) << ((sizeof(size_t) << 3) - 1);
  };

  struct JSONValue : public Variant<Str, double, int64_t, bool, DynArray<JSONValue, size_t, ARRAY_SAFE>, DynArray<std::pair<Str, JSONValue>, size_t, ARRAY_SAFE>>
  {
    typedef DynArray<JSONValue, size_t, ARRAY_SAFE> JSONArray;
    typedef DynArray<std::pair<Str, JSONValue>, size_t, ARRAY_SAFE> JSONObject;
    typedef Variant<Str, double, int64_t, bool, JSONArray, JSONObject> BASE;

  public:
    JSONValue() : BASE() {}
    JSONValue(const BASE& v) : BASE(v) {}
    JSONValue(BASE&& v) : BASE(std::move(v)) {}
    template<typename T>
    explicit JSONValue(const T& obj) : BASE(obj) {}
    template<typename T>
    explicit JSONValue(T&& obj) : BASE(internal::serializer::ConvRef<JSONValue>::Value<T, BASE>::f(obj)) {}
    ~JSONValue() {}
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(internal::serializer::ConvRef<JSONValue>::Value<T, BASE>::f(right)); return *this; }
  };

  template<typename F>
  void JSONEngine::ParseJSONObject(Serializer<JSONEngine>& e, F f)
  {
    std::istream& s = *e.in;
    Str buf;
    ParseJSONEatWhitespace(s);
    if(!s || s.get() != '{') return;
    ParseJSONEatWhitespace(s);
    while(!!s && s.peek() != '}' && s.peek() != -1)
    {
      if(!s || s.get() != '"' || s.peek() == -1)
        continue;

      buf.clear(); // clear buffer to hold name
      while(!!s && s.peek() != '"' && s.peek() != -1)
        ParseJSONEatCharacter(buf, s);
      if(s)
        s.get(); // eat " character

      ParseJSONEatWhitespace(s);
      if(s.get() != ':')
        continue;

      ParseJSONEatWhitespace(s);
      f(e, buf.c_str());

      while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != -1) // eat everything up to a , or } character
        s.get();
      if(!!s && s.peek() == ',') // Only eat comma if it's there.
        s.get();

      ParseJSONEatWhitespace(s);
    }
    if(s.peek() == '}') s.get(); // eat the closing brace
  }

  template<class T>
  inline void ParseJSONBase(Serializer<JSONEngine>& e, T& obj, std::istream& s)
  { 
    static_assert(internal::serializer::is_serializable<JSONEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
    obj.template Serialize<JSONEngine>(e, 0);
  }
  template<class T>
  inline void ParseJSON(T& obj, std::istream& s) { Serializer<JSONEngine> e; e.Parse<T>(obj, s, 0); }
  template<class T>
  inline void ParseJSON(T& obj, const char* s) { std::istringstream ss(s); ParseJSON<T>(obj, ss); }

  template<>
  inline void ParseJSONBase<std::string>(Serializer<JSONEngine>& e, std::string& target, std::istream& s)
  {
    target.clear();

    if(s.peek() == ',' || s.peek() == ']' || s.peek() == '}')
      return;

    char c = s.get();
    if(c != '"')
    {
      target += c;
      while(!!s && s.peek() != ',' && s.peek() != '}' && s.peek() != ']' && s.peek() != -1)
        target += s.get();
      return;
    }

    while(!!s && s.peek() != '"' && s.peek() != -1)
      JSONEngine::ParseJSONEatCharacter(target, s);

    s.get(); // eat last " character
  }
  template<>
  inline void ParseJSONBase<Str>(Serializer<JSONEngine>& e, Str& target, std::istream& s) { ParseJSONBase<std::string>(e, target, s); }

  template<>
  inline void ParseJSONBase<JSONValue>(Serializer<JSONEngine>& e, JSONValue& target, std::istream& s)
  {
    JSONEngine::ParseJSONEatWhitespace(s);
    switch(s.peek())
    {
    case '{':
      target = JSONValue::JSONObject();
      Serializer<JSONEngine>::template ActionBind<JSONValue::JSONObject>::Parse(e, target.get<JSONValue::JSONObject>(), 0);
      break;
    case '[':
      target = JSONValue::JSONArray();
      Serializer<JSONEngine>::template ActionBind<JSONValue::JSONArray>::Parse(e, target.get<JSONValue::JSONArray>(), 0);
      break;
    case '"':
      target = Str();
      ParseJSONBase<Str>(e, target.get<Str>(), s);
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
    {
      Str buf;
      ParseJSONBase<Str>(e, buf, s);
      const char* dot = strchr(buf.c_str(), '.');

      if(dot)
        target = (double)strtod(buf.c_str(), 0);
      else
        target = (int64_t)strtoll(buf.c_str(), 0, 10);
    }
    break;
    default:
    {
      Str buf;
      ParseJSONBase<Str>(e, buf, s);
      if(!STRICMP(buf.c_str(), "true")) target = true;
      else if(!STRICMP(buf.c_str(), "false")) target = false;
      else if(!STRICMP(buf.c_str(), "null")) break;
      else target = buf;
      break;
    }
    }
  }
  template<typename T>
  void JSONEngine::Parse(Serializer<JSONEngine>& e, T& obj, const char* id) { ParseJSONBase<T>(e, obj, *e.in); }
  template<typename F>
  void JSONEngine::ParseMany(Serializer<JSONEngine>& e, F && f)
  {
    ParseJSONObject<F>(e, f);
  }
  template<typename T, typename E, void(*Add)(Serializer<JSONEngine>& e, T& obj, int& n), bool(*Read)(Serializer<JSONEngine>& e, T& obj, int64_t count)>
  void JSONEngine::ParseArray(Serializer<JSONEngine>& e, T& obj, const char* id)
  {
    std::istream& s = *e.in;
    ParseJSONEatWhitespace(s);

    if(!s || s.get() != '[')
      return;

    ParseJSONEatWhitespace(s);
    int n = 0;

    while(!!s && s.peek() != ']' && s.peek() != -1)
    {
      Add(e, obj, n);
      while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) // eat everything up to a , or ] character
        s.get();
      if(!!s && s.peek() == ',' && s.peek() != -1) // Only eat comma if it's there.
        s.get();
      ParseJSONEatWhitespace(s);
    }
    if(s.peek() == ']')
      s.get();
  }
  template<typename T>
  void JSONEngine::ParseNumber(Serializer<JSONEngine>& e, T& obj, const char* id)
  {
    std::istream& s = *e.in;
    if(s.peek() == ',' || s.peek() == ']' || s.peek() == '}')
      return;

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

  template<class T>
  void JSONEngine::WriteJSONObject(Serializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s, size_t& pretty)
  {
    WriteJSONComma(s, pretty);

    if(!id && WriteJSONIsPretty(pretty))
      s << std::endl;

    WriteJSONId(id, s, pretty);

    if(!id && WriteJSONIsPretty(pretty))
      WriteJSONTabs(s, pretty);

    s << '{';
    {
      internal::serializer::PushValue<size_t> push(e.engine.pretty, WriteJSONPretty(pretty));
      static_assert(internal::serializer::is_serializable<JSONEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
      const_cast<T&>(obj).template Serialize<JSONEngine>(e, id);
    }

    if(WriteJSONIsPretty(pretty))
    {
      s << std::endl;
      WriteJSONTabs(s, pretty);
    }

    s << '}';
    s.flush();
  }

  // To enable pretty output, set pretty to 1. The upper two bits are used as flags, so if you need more than 1073741823 levels of indentation... what the hell are you doing?!
  template<class T>
  inline void WriteJSONBase(Serializer<JSONEngine>& e, const char* id, const T& obj, std::ostream& s) { JSONEngine::WriteJSONObject<T>(e, id, obj, s, e.engine.pretty); }

  template<>
  inline void WriteJSONBase<std::string>(Serializer<JSONEngine>& e, const char* id, const std::string& obj, std::ostream& s)
  {
    JSONEngine::WriteJSONComma(s, e.engine.pretty);
    JSONEngine::WriteJSONId(id, s, e.engine.pretty);
    s << '"';

    for(size_t i = 0; i < obj.size(); ++i)
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
  inline void WriteJSONBase<Str>(Serializer<JSONEngine>& e, const char* id, const Str& obj, std::ostream& s) { WriteJSONBase<std::string>(e, id, obj, s); }

  template<class T>
  inline void WriteJSON(const T& obj, std::ostream& s, size_t pretty = 0) { Serializer<JSONEngine> e; e.engine.pretty = pretty; e.Serialize<T>(obj, s, 0); }

  template<class T>
  inline void WriteJSON(const T& obj, const char* file, size_t pretty = 0) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteJSON<T>(obj, fs, pretty); }

  template<>
  inline void WriteJSONBase<JSONValue>(Serializer<JSONEngine>& e, const char* id, const JSONValue& obj, std::ostream& s)
  {
    switch(obj.tag())
    {
    case JSONValue::Type<Str>::value: Serializer<JSONEngine>::template ActionBind<Str>::Serialize(e, obj.get<Str>(), id); break;
    case JSONValue::Type<double>::value: Serializer<JSONEngine>::template ActionBind<double>::Serialize(e, obj.get<double>(), id); break;
    case JSONValue::Type<int64_t>::value: Serializer<JSONEngine>::template ActionBind<int64_t>::Serialize(e, obj.get<int64_t>(), id); break;
    case JSONValue::Type<bool>::value: Serializer<JSONEngine>::template ActionBind<bool>::Serialize(e, obj.get<bool>(), id); break;
    case JSONValue::Type<JSONValue::JSONArray>::value:
      Serializer<JSONEngine>::template ActionBind<JSONValue::JSONArray>::Serialize(e, obj.get<JSONValue::JSONArray>(), id);
      break;
    case JSONValue::Type<JSONValue::JSONObject>::value:
      Serializer<JSONEngine>::template ActionBind<JSONValue::JSONObject>::Serialize(e, obj.get<JSONValue::JSONObject>(), id);
      break;
    }
  }
  template<typename T>
  void JSONEngine::SerializeArray(Serializer<JSONEngine>& e, const T& obj, size_t size, const char* id)
  {
    std::ostream& s = *e.out;
    WriteJSONComma(s, e.engine.pretty);
    WriteJSONId(id, s, e.engine.pretty);
    internal::serializer::PushValue<size_t> push(e.engine.pretty, WriteJSONPretty(e.engine.pretty));
    s << '[';

    auto begin = std::begin(obj);
    auto end = std::end(obj);
    for(; begin != end; ++begin)
      Serializer<JSONEngine>::template ActionBind<remove_cvref_t<decltype(*begin)>>::Serialize(e, *begin, 0);
    
    s << ']';
  }
  template<typename T, size_t... S>
  void JSONEngine::SerializeTuple(Serializer<JSONEngine>& e, const T& obj, const char* id, std::index_sequence<S...>)
  {
    std::ostream& s = *e.out;
    WriteJSONComma(s, e.engine.pretty);
    WriteJSONId(id, s, e.engine.pretty);
    internal::serializer::PushValue<size_t> push(e.engine.pretty, WriteJSONPretty(e.engine.pretty));
    s << '[';

    (Serializer<JSONEngine>::template ActionBind<std::tuple_element_t<S, T>>::Serialize(e, std::get<S>(obj), 0), ...);

    s << ']';
  }

  template<typename T>
  void JSONEngine::SerializeNumber(Serializer<JSONEngine>& e, T obj, const char* id)
  {
    WriteJSONComma(*e.out, e.engine.pretty);
    WriteJSONId(id, *e.out, e.engine.pretty);
    (*e.out) << obj;
  }

  template<typename T>
  void JSONEngine::Serialize(Serializer<JSONEngine>& e, const T& obj, const char* id)
  {
    WriteJSONBase<T>(e, id, obj, *e.out);
  }
}

#endif