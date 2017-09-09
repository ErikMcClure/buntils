// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __UBJSON_H__BSS__
#define __UBJSON_H__BSS__

#include "DynArray.h"
#include "Str.h"
#include "bss_util.h"
#include "Variant.h"
#include "Serializer.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <limits>

namespace bss {
  struct UBJSONValue;

  class UBJSONEngine
  {
  public:
    UBJSONEngine() : type(TYPE_NONE), endobject(false) {}
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<UBJSONEngine>& e) {}
    static void End(Serializer<UBJSONEngine>& e) {}
    template<typename T>
    static void Parse(Serializer<UBJSONEngine>& e, T& t, const char* id);
    template<typename F>
    static void ParseMany(Serializer<UBJSONEngine>& e, F && f);
    template<typename T, typename E>
    static void ParseArray(Serializer<UBJSONEngine>& e, T& obj, const char* id);
    template<typename T>
    static void ParseNumber(Serializer<UBJSONEngine>& e, T& t, const char* id);
    static void ParseBool(Serializer<UBJSONEngine>& e, bool& obj, const char* id)
    {
      UBJSONTuple tuple;
      ParseValue(tuple, *e.in, e.engine.type);

      switch(tuple.type)
      {
      case TYPE_NULL: break;
      case TYPE_TRUE: obj = true; break;
      case TYPE_FALSE: obj = false; break;
      case TYPE_CHAR:
      case TYPE_INT8: obj = tuple.Int8 != 0; break;
      case TYPE_UINT8: obj = tuple.UInt8 != 0; break;
      case TYPE_INT16: obj = tuple.Int16 != 0; break;
      case TYPE_INT32: obj = tuple.Int32 != 0; break;
      case TYPE_INT64: obj = tuple.Int64 != 0; break;
      case TYPE_BIGNUM: break;
      }
    }

    template<typename T>
    static void Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id);
    template<typename T>
    static void SerializeArray(Serializer<UBJSONEngine>& e, const T& obj, size_t size, const char* id);
    template<typename T>
    static void SerializeNumber(Serializer<UBJSONEngine>& e, T t, const char* id);
    static void SerializeBool(Serializer<UBJSONEngine>& e, bool t, const char* id) 
    { 
      WriteUBJSONId(e, id, *e.out); 
      if(!e.engine.type)
        e.out->put(t ? UBJSONEngine::TYPE_TRUE : UBJSONEngine::TYPE_FALSE); 
    }

    enum TYPE : char
    {
      TYPE_NONE = 0, // Distinct from "NULL", this means there is no type specified
      TYPE_NULL = 'Z',
      TYPE_NO_OP = 'N',
      TYPE_TRUE = 'T',
      TYPE_FALSE = 'F',
      TYPE_INT8 = 'i',
      TYPE_UINT8 = 'U',
      TYPE_INT16 = 'I',
      TYPE_INT32 = 'l',
      TYPE_INT64 = 'L',
      TYPE_FLOAT = 'd',
      TYPE_DOUBLE = 'D',
      TYPE_BIGNUM = 'H',
      TYPE_CHAR = 'C',
      TYPE_STRING = 'S',
      TYPE_OBJECT = '{',
      TYPE_ARRAY = '[',
      TYPE_OBJECT_END = '}',
      TYPE_ARRAY_END = ']',
      TYPE_TYPE = '$',
      TYPE_COUNT = '#',
    };

    struct UBJSONTuple
    {
      UBJSONTuple() : type(TYPE_NO_OP), length(-1), Int64(0) {}
      ~UBJSONTuple()
      {
        if((type == TYPE_STRING || type == TYPE_BIGNUM) && String != 0)
          delete[] String;
      }

      TYPE type;
      int64_t length;
      union
      {
        char Int8;
        uint8_t UInt8;
        short Int16;
        int32_t Int32;
        int64_t Int64;
        float Float;
        double Double;
        char* String;
      };
    };

    template<class T>
    inline static T ParseInteger(std::istream& s)
    {
      T v;
      s.read((char*)&v, sizeof(T));
#ifdef BSS_ENDIAN_LITTLE
      FlipEndian<T>(&v);
#endif
      return v;
    }

    inline static int64_t ParseLength(std::istream& s)
    {
      int64_t ret = -1;
      while(s)
      {
        switch(s.get())
        {
        case TYPE_CHAR: // you aren't supposed to do this but we'll deal with it anyway
        case TYPE_INT8: ret = ParseInteger<char>(s); break;
        case TYPE_UINT8: ret = ParseInteger<uint8_t>(s); break;
        case TYPE_INT16: ret = ParseInteger<short>(s); break;
        case TYPE_INT32: ret = ParseInteger<int32_t>(s); break;
        case TYPE_INT64: ret = ParseInteger<int64_t>(s); break;
        case TYPE_NO_OP: continue; // try again
        default:
          throw std::runtime_error("Invalid length type");
        }
        break;
      }

      if(ret < 0)
        throw std::runtime_error("Negative length is not allowed.");
      return ret;
    }

    inline static int64_t ParseTypeCount(std::istream& s, TYPE& type)
    {
      type = TYPE_NONE;
      if(!!s && s.peek() == TYPE_TYPE)
      {
        s.get(); // eat '$'
        type = TYPE(s.get());
      }

      if(!!s && s.peek() == TYPE_COUNT)
      {
        s.get(); // eat '#'
        return ParseLength(s);
      }
      else if(type != TYPE_NONE)
        throw std::runtime_error("A type was specified, but no count was given. A count MUST follow a type!");
      return -1;
    }

    template<class F>
    inline static void ParseObj(Serializer<UBJSONEngine>& e, TYPE ty, F f)
    {
      std::istream& s = *e.in;

      if(ty != TYPE_NONE && ty != TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object parsing function!");
      if(!ty && (s.get() != TYPE_OBJECT))
        throw std::runtime_error("Expected object, found invalid character");

      Str buf;
      internal::serializer::PushValue<TYPE> push(e.engine.type, TYPE_NONE);
      int64_t count = UBJSONEngine::ParseTypeCount(s, e.engine.type);

      while(!!s && (count < 0 || count>0) && (count > 0 || s.peek() != TYPE_OBJECT_END) && s.peek() != -1)
      {
        int64_t length = UBJSONEngine::ParseLength(s);
        --count;
        buf.reserve(length + 1);
        s.read(buf.UnsafeString(), length);
        buf.UnsafeString()[length] = 0;
        f(e, buf.c_str());
      }

      if(!!s && count < 0 && s.peek() == TYPE_OBJECT_END)
        s.get(); // If we were looking for the OBJECT_END symbol, eat it.
    }

    inline static void ParseValue(UBJSONTuple& tuple, std::istream& s, TYPE type)
    {
      if(type != TYPE_NONE)
        tuple.type = (TYPE)type;
      else
      {
        do
          tuple.type = (TYPE)s.get();
        while(tuple.type == TYPE_NO_OP);
      }

      switch(tuple.type)
      {
      default:
        throw std::runtime_error("Unexpected character while parsing value.");
      case TYPE_NO_OP:
      case TYPE_NULL:
      case TYPE_TRUE:
      case TYPE_FALSE:
      case TYPE_ARRAY:
      case TYPE_OBJECT:
        return;
      case TYPE_CHAR:
      case TYPE_INT8: tuple.Int8 = ParseInteger<char>(s); break;
      case TYPE_UINT8: tuple.UInt8 = ParseInteger<uint8_t>(s); break;
      case TYPE_INT16: tuple.Int16 = ParseInteger<short>(s); break;
      case TYPE_INT32: tuple.Int32 = ParseInteger<int32_t>(s); break;
      case TYPE_INT64: tuple.Int64 = ParseInteger<int64_t>(s); break;
      case TYPE_FLOAT: tuple.Float = ParseInteger<float>(s); break;
      case TYPE_DOUBLE: tuple.Double = ParseInteger<double>(s); break;
      case TYPE_BIGNUM:
      case TYPE_STRING:
        tuple.length = ParseLength(s);
        tuple.String = new char[(size_t)tuple.length + 1];

        if(tuple.length > 0)
          s.read(tuple.String, tuple.length);

        tuple.String[tuple.length] = 0;
        break;
      }
    }

    inline static void WriteUBJSONString(Serializer<UBJSONEngine>& e, const char* str, size_t len, std::ostream& s, UBJSONEngine::TYPE type)
    {
      if(!type)
        s.put(TYPE_STRING);

      internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, TYPE_NONE);
      Serializer<UBJSONEngine>::ActionBind<size_t>::Serialize(e, len, 0);
      s.write(str, len);
    }

    inline static void WriteUBJSONId(Serializer<UBJSONEngine>& e, const char* id, std::ostream& s)
    {
      if(id)
        WriteUBJSONString(e, id, strlen(id), s, TYPE_STRING);
    }

    TYPE type;
    bool endobject;
  };

  template<class T>
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s);

  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s);

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE type);

  template<typename... Args>
  inline const UBJSONEngine::TYPE GetUBJSONVariantType(const Variant<Args...>& v);

  struct UBJSONValue : Variant<Str, bool, uint8_t, char, int16_t, int32_t, int64_t, float, double, DynArray<UBJSONValue, size_t, ARRAY_SAFE>, DynArray<std::pair<Str, UBJSONValue>, size_t, ARRAY_SAFE>, DynArray<uint8_t, size_t>>
  {
    typedef DynArray<UBJSONValue, size_t, ARRAY_SAFE> UBJSONArray;
    typedef DynArray<std::pair<Str, UBJSONValue>, size_t, ARRAY_SAFE> UBJSONObject;
    typedef DynArray<uint8_t, size_t> UBJSONBinary;
    typedef Variant<Str, bool, uint8_t, char, int16_t, int32_t, int64_t, float, double, UBJSONArray, UBJSONObject, UBJSONBinary> BASE;

  public:
    UBJSONValue() : BASE() {}
    UBJSONValue(const BASE& v) : BASE(v) {}
    UBJSONValue(BASE&& v) : BASE(std::move(v)) {}
    template<typename T>
    explicit UBJSONValue(const T& t) : BASE(t) {}
    template<typename T>
    explicit UBJSONValue(T&& t) : BASE(internal::serializer::ConvRef<UBJSONValue>::Value<T, BASE>::f(t)) {}
    ~UBJSONValue() {}
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(internal::serializer::ConvRef<UBJSONValue>::Value<T, BASE>::f(right)); return *this; }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e)
    {
      assert(is<UBJSONObject>());
      if(e.out)
      {
        auto& v = get<UBJSONObject>();
        std::ostream& s = *e.out;

        UBJSONEngine::TYPE type = (v.Length() > 0) ? GetUBJSONVariantType(v[0].second) : UBJSONEngine::TYPE_NONE;
        for(size_t i = 1; i < v.Length(); ++i)
          if(type != GetUBJSONVariantType(v[i].second))
            type = UBJSONEngine::TYPE_NONE;

        if(type)
        {
          s.put(UBJSONEngine::TYPE_TYPE);
          s.put(type);
        }

        s.put(UBJSONEngine::TYPE_COUNT);

        {
          internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, UBJSONEngine::TYPE_NONE);
          Serializer<UBJSONEngine>::ActionBind<size_t>::Serialize(e, v.Length(), 0);
        }

        for(auto& i : v)
          WriteUBJSONBase<UBJSONValue>(e, i.first, i.second, s, type);

        e.engine.endobject = false;
      }
    }
  };

  template<typename T>
  void UBJSONEngine::ParseNumber(Serializer<UBJSONEngine>& e, T& obj, const char* id)
  {
    UBJSONEngine::UBJSONTuple tuple;
    UBJSONEngine::ParseValue(tuple, *e.in, e.engine.type);

    switch(tuple.type) // as long as it's any arithmetic type, attempt to shove it into our target, casting if necessary.
    {
    case UBJSONEngine::TYPE_NULL: break;
    case UBJSONEngine::TYPE_TRUE: obj = (T)1; break;
    case UBJSONEngine::TYPE_FALSE: obj = (T)0; break;
    case UBJSONEngine::TYPE_CHAR:
    case UBJSONEngine::TYPE_INT8: obj = (T)tuple.Int8; break;
    case UBJSONEngine::TYPE_UINT8: obj = (T)tuple.UInt8; break;
    case UBJSONEngine::TYPE_INT16: obj = (T)tuple.Int16; break;
    case UBJSONEngine::TYPE_INT32: obj = (T)tuple.Int32; break;
    case UBJSONEngine::TYPE_INT64: obj = (T)tuple.Int64; break;
    case UBJSONEngine::TYPE_FLOAT: obj = (T)tuple.Float; break;
    case UBJSONEngine::TYPE_DOUBLE: obj = (T)tuple.Double; break;
    case UBJSONEngine::TYPE_BIGNUM: break;// we can't deal with bignum
    }
  }

  template<typename T, typename E>
  void UBJSONEngine::ParseArray(Serializer<UBJSONEngine>& e, T& obj, const char* id)
  {
    UBJSONEngine::TYPE ty = e.engine.type;
    std::istream& s = *e.in;
    if(ty != UBJSONEngine::TYPE_NONE && ty != UBJSONEngine::TYPE_ARRAY) // Sanity check
      throw std::runtime_error("Expecting a type other than array in the array parsing function!");
    if(!ty && (s.get() != UBJSONEngine::TYPE_ARRAY))
      throw std::runtime_error("Expected array, found invalid character");
    int num = 0;
    UBJSONEngine::TYPE type = UBJSONEngine::TYPE_NONE;
    int64_t count = UBJSONEngine::ParseTypeCount(s, type);
    internal::serializer::PushValue<TYPE> push(e.engine.type, type);
    if(count <= 0 ||
      type == UBJSONEngine::TYPE_NONE || 
      (type != UBJSONEngine::TYPE_CHAR && type != UBJSONEngine::TYPE_UINT8 && type != UBJSONEngine::TYPE_INT8) ||
      !Serializer<UBJSONEngine>::template Bulk<T>::Read(e, obj, count)) // attempt mass read in if we have a 1 byte type and a count
    {
      while(!!s && (count > 0 || count < 0) && (count > 0 || s.peek() != UBJSONEngine::TYPE_ARRAY_END) && s.peek() != -1)
      {
        Serializer<UBJSONEngine>::ActionBind<T>::Add(e, obj, num);
        --count;
      }
      if(!!s && count < 0 && s.peek() == UBJSONEngine::TYPE_ARRAY_END) s.get(); // If we were looking for the ARRAY_END symbol, eat it.
    }
  }

  template<class T>
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s)
  {
    obj.template Serialize<UBJSONEngine>(e);
  }

  template<>
  inline void ParseUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, std::string& obj, std::istream& s)
  {
    UBJSONEngine::UBJSONTuple tuple;
    UBJSONEngine::ParseValue(tuple, s, e.engine.type);

    switch(tuple.type) // we will shove just about anything we possibly can into a string
    {
    case UBJSONEngine::TYPE_NULL: break;
    case UBJSONEngine::TYPE_CHAR: obj = tuple.Int8;
    case UBJSONEngine::TYPE_INT8: obj = std::to_string((int)tuple.Int8); break;
    case UBJSONEngine::TYPE_UINT8: obj = std::to_string(tuple.UInt8); break;
    case UBJSONEngine::TYPE_INT16: obj = std::to_string(tuple.Int16); break;
    case UBJSONEngine::TYPE_INT32: obj = std::to_string(tuple.Int32); break;
    case UBJSONEngine::TYPE_INT64: obj = std::to_string(tuple.Int64); break;
    case UBJSONEngine::TYPE_FLOAT: obj = std::to_string(tuple.Float); break;
    case UBJSONEngine::TYPE_DOUBLE: obj = std::to_string(tuple.Double); break;
    case UBJSONEngine::TYPE_BIGNUM:
    case UBJSONEngine::TYPE_STRING:
      obj = tuple.String;
      break;
    }
  }

  template<>
  inline void ParseUBJSONBase<Str>(Serializer<UBJSONEngine>& e, Str& obj, std::istream& s) { ParseUBJSONBase<std::string>(e, obj, s); }

  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s)
  {
    UBJSONEngine::UBJSONTuple tuple;
    UBJSONEngine::ParseValue(tuple, s, e.engine.type);

    switch(tuple.type)
    {
    case UBJSONEngine::TYPE_NO_OP: break;
    case UBJSONEngine::TYPE_NULL: break;
    case UBJSONEngine::TYPE_TRUE: obj = true; break;
    case UBJSONEngine::TYPE_FALSE: obj = false; break;
    case UBJSONEngine::TYPE_ARRAY:
    {
      obj = UBJSONValue::UBJSONArray();
      internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, UBJSONEngine::TYPE_ARRAY);
      Serializer<UBJSONEngine>::ActionBind<UBJSONValue::UBJSONArray>::Parse(e, obj.get<UBJSONValue::UBJSONArray>(), 0);
    }
      break;
    case UBJSONEngine::TYPE_OBJECT:
      obj = UBJSONValue::UBJSONObject();
      UBJSONEngine::ParseObj(e, UBJSONEngine::TYPE_OBJECT, internal::serializer::KeyValueArray<UBJSONEngine, Str, UBJSONValue>(obj.get<UBJSONValue::UBJSONObject>()));
      break;
    case UBJSONEngine::TYPE_CHAR:
    case UBJSONEngine::TYPE_INT8: obj = tuple.Int8; break;
    case UBJSONEngine::TYPE_UINT8: obj = tuple.UInt8; break;
    case UBJSONEngine::TYPE_INT16: obj = tuple.Int16; break;
    case UBJSONEngine::TYPE_INT32: obj = tuple.Int32; break;
    case UBJSONEngine::TYPE_INT64: obj = tuple.Int64; break;
    case UBJSONEngine::TYPE_FLOAT: obj = tuple.Float; break;
    case UBJSONEngine::TYPE_DOUBLE: obj = tuple.Double; break;
    case UBJSONEngine::TYPE_BIGNUM:
    case UBJSONEngine::TYPE_STRING:
      obj = Str(tuple.String);
      break;
    }
  }

  template<class T>
  inline void ParseUBJSON(T& obj, std::istream& s)
  {
    Serializer<UBJSONEngine> e;
    e.Parse<T>(obj, s);
  }

  namespace internal {
    template<class T>
    void WriteUBJSONObject(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, UBJSONEngine::TYPE ty)
    {
      if(!ty)
        s.put(UBJSONEngine::TYPE_OBJECT);
      else if(ty != UBJSONEngine::TYPE_OBJECT)
        throw std::runtime_error("Expecting a type other than object in the object serializing function!");

      internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, UBJSONEngine::TYPE_NONE);
      const_cast<T&>(obj).template Serialize<UBJSONEngine>(e);

      if(e.engine.endobject)
      //if(obj.SerializeUBJSON(s))
        s.put(UBJSONEngine::TYPE_OBJECT_END);
    }

    template<class T>
    inline void WriteUBJSONInteger(T v, std::ostream& s)
    {
#ifdef BSS_ENDIAN_LITTLE
      FlipEndian<T>(&v);
#endif
      s.write((char*)&v, sizeof(T));
    }

    template<class T, typename FROM>
    bool WriteUBJSONSpecificInt(const FROM& obj, std::ostream& s, UBJSONEngine::TYPE type)
    {
      if(obj >= (FROM)std::numeric_limits<typename std::conditional<std::is_unsigned<FROM>::value && sizeof(FROM) == sizeof(T), FROM, T>::type>::min() &&
        obj <= (FROM)std::numeric_limits<typename std::conditional<std::is_signed<FROM>::value && sizeof(FROM) == sizeof(T), FROM, T>::type>::max())
      {
        s.put(type);
        WriteUBJSONInteger<T>((T)obj, s);
        return true;
      }
      return false;
    }

    template<class T> struct WriteUBJSONType { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_OBJECT; };
    template<> struct WriteUBJSONType<uint8_t> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_UINT8; };
    template<> struct WriteUBJSONType<char> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_INT8; };
    template<> struct WriteUBJSONType<bool> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_NONE; };
    template<> struct WriteUBJSONType<short> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_INT16; };
    template<> struct WriteUBJSONType<int32_t> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_INT32; };
    template<> struct WriteUBJSONType<int64_t> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_INT64; };
    template<> struct WriteUBJSONType<float> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_FLOAT; };
    template<> struct WriteUBJSONType<double> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_DOUBLE; };
    template<> struct WriteUBJSONType<const char*> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_STRING; };
    template<> struct WriteUBJSONType<std::string> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_STRING; };
    template<> struct WriteUBJSONType<Str> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_STRING; };
    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONType<DynArray<T, CType, ArrayType, Alloc>> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_ARRAY; };
    template<class T, typename Alloc>
    struct WriteUBJSONType<std::vector<T, Alloc>> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_ARRAY; };
    template<class T, int I> struct WriteUBJSONType<T[I]> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_ARRAY; };
    template<> struct WriteUBJSONType<UBJSONValue::UBJSONArray> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_ARRAY; };
    template<> struct WriteUBJSONType<UBJSONValue::UBJSONObject> { static const UBJSONEngine::TYPE t = UBJSONEngine::TYPE_OBJECT; };

    template<class T, typename Arg, typename... Args>
    struct __UBJSONVariantType
    {
      static inline const UBJSONEngine::TYPE F(const T& v)
      {
        if(v.template is<Arg>())
          return WriteUBJSONType<Arg>::t;
        return __UBJSONVariantType<T, Args...>::F(v);
      }
    };
    template<class T, typename Arg>
    struct __UBJSONVariantType<T, Arg>
    {
      static inline const UBJSONEngine::TYPE F(const T& v)
      {
        if(v.template is<Arg>())
          return WriteUBJSONType<Arg>::t;
        return UBJSONEngine::TYPE_NONE;
      }
    };

    template<typename T>
    inline void WriteUBJSONArray(Serializer<UBJSONEngine>& e, const T& obj, size_t size, const char* id, UBJSONEngine::TYPE ty)
    {
      std::ostream& s = *e.out;
      UBJSONEngine::WriteUBJSONId(e, id, s);

      if(!e.engine.type)
        s.put(UBJSONEngine::TYPE_ARRAY);
      else
        assert(e.engine.type == UBJSONEngine::TYPE_ARRAY);
      if(!size) // It's more efficient to skip type/length information for zero length arrays.
      {
        s.put(UBJSONEngine::TYPE_ARRAY_END);
        return;
      }
      if(ty)
      {
        s.put(UBJSONEngine::TYPE_TYPE);
        s.put(ty);
      }
      s.put(UBJSONEngine::TYPE_COUNT);

      {
        internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, UBJSONEngine::TYPE_NONE);
        Serializer<UBJSONEngine>::ActionBind<size_t>::Serialize(e, size, 0);
      }

      if(ty != UBJSONEngine::TYPE_CHAR || ty != UBJSONEngine::TYPE_UINT8 || ty != UBJSONEngine::TYPE_INT8 || !Serializer<UBJSONEngine>::Bulk<T>::Write(e, obj, size))
      {
        auto begin = std::begin(obj);
        auto end = std::end(obj);
        internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, ty);
        for(; begin != end; ++begin)
          Serializer<UBJSONEngine>::ActionBind<typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type>::Serialize(e, *begin, 0);
      }
    }
  }

  template<typename T>
  void UBJSONEngine::SerializeArray(Serializer<UBJSONEngine>& e, const T& obj, size_t size, const char* id)
  {
    TYPE ty = internal::WriteUBJSONType<typename std::remove_const<typename std::remove_reference<decltype(*std::begin(obj))>::type>::type>::t; // This is assigned to whatever type our actual array resolves to
    internal::WriteUBJSONArray<T>(e, obj, size, id, ty);
  }

  template<typename T>
  void UBJSONEngine::SerializeNumber(Serializer<UBJSONEngine>& e, T obj, const char* id)
  {
    std::ostream& s = *e.out;
    WriteUBJSONId(e, id, s);
    switch(e.engine.type)
    {
    case 0:
      if(std::is_same<float, T>::value)
      {
        s.put(UBJSONEngine::TYPE_FLOAT);
        internal::WriteUBJSONInteger<float>((float)obj, s);
      }
      else if(std::is_same<double, T>::value || std::is_same<long double, T>::value)
      {
        s.put(UBJSONEngine::TYPE_DOUBLE);
        internal::WriteUBJSONInteger<double>((double)obj, s);
      }
      else if(!internal::WriteUBJSONSpecificInt<uint8_t, T>(obj, s, UBJSONEngine::TYPE_UINT8) &&
        !internal::WriteUBJSONSpecificInt<char, T>(obj, s, UBJSONEngine::TYPE_INT8) &&
        !internal::WriteUBJSONSpecificInt<short, T>(obj, s, UBJSONEngine::TYPE_INT16) &&
        !internal::WriteUBJSONSpecificInt<int32_t, T>(obj, s, UBJSONEngine::TYPE_INT32) &&
        !internal::WriteUBJSONSpecificInt<int64_t, T>(obj, s, UBJSONEngine::TYPE_INT64))
      {
        s.put(UBJSONEngine::TYPE_BIGNUM);
        throw std::runtime_error("Unknown large integer type!");
      }
      break;
    case UBJSONEngine::TYPE_CHAR:
    case UBJSONEngine::TYPE_INT8: internal::WriteUBJSONInteger<char>((char)obj, s); break;
    case UBJSONEngine::TYPE_UINT8: internal::WriteUBJSONInteger<uint8_t>((uint8_t)obj, s); break;
    case UBJSONEngine::TYPE_INT16: internal::WriteUBJSONInteger<short>((short)obj, s); break;
    case UBJSONEngine::TYPE_INT32: internal::WriteUBJSONInteger<int32_t>((int32_t)obj, s); break;
    case UBJSONEngine::TYPE_INT64: internal::WriteUBJSONInteger<int64_t>((int64_t)obj, s); break;
    case UBJSONEngine::TYPE_FLOAT: internal::WriteUBJSONInteger<float>((float)obj, s); break;
    case UBJSONEngine::TYPE_DOUBLE: internal::WriteUBJSONInteger<double>((double)obj, s); break;
    case UBJSONEngine::TYPE_BIGNUM: break; // we can't deal with bignum
    }
  }

  template<typename... Args>
  inline const UBJSONEngine::TYPE GetUBJSONVariantType(const Variant<Args...>& v) { return internal::__UBJSONVariantType<Variant<Args...>, Args...>::F(v); }

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE type) { UBJSONEngine::WriteUBJSONId(e, id, s); internal::WriteUBJSONObject<T>(e, obj, s, type); }

  template<>
  inline void WriteUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, const char* id, const UBJSONValue& obj, std::ostream& s, UBJSONEngine::TYPE ty)
  {
    UBJSONEngine::WriteUBJSONId(e, id, s);
    internal::serializer::PushValue<UBJSONEngine::TYPE> push(e.engine.type, ty);

    switch(obj.tag())
    {
    case UBJSONValue::Type<Str>::value: Serializer<UBJSONEngine>::ActionBind<Str>::Serialize(e, obj.get<Str>(), 0); break;
    case UBJSONValue::Type<bool>::value: Serializer<UBJSONEngine>::ActionBind<bool>::Serialize(e, obj.get<bool>(), 0); break;
    case UBJSONValue::Type<uint8_t>::value: Serializer<UBJSONEngine>::ActionBind<uint8_t>::Serialize(e, obj.get<uint8_t>(), 0); break;
    case UBJSONValue::Type<char>::value: Serializer<UBJSONEngine>::ActionBind<char>::Serialize(e, obj.get<char>(), 0); break;
    case UBJSONValue::Type<int16_t>::value: Serializer<UBJSONEngine>::ActionBind<int16_t>::Serialize(e, obj.get<int16_t>(), 0); break;
    case UBJSONValue::Type<int32_t>::value: Serializer<UBJSONEngine>::ActionBind<int32_t>::Serialize(e, obj.get<int32_t>(), 0); break;
    case UBJSONValue::Type<int64_t>::value: Serializer<UBJSONEngine>::ActionBind<int64_t>::Serialize(e, obj.get<int64_t>(), 0); break;
    case UBJSONValue::Type<float>::value: Serializer<UBJSONEngine>::ActionBind<float>::Serialize(e, obj.get<float>(), 0); break;
    case UBJSONValue::Type<double>::value: Serializer<UBJSONEngine>::ActionBind<double>::Serialize(e, obj.get<double>(), 0); break;
    case UBJSONValue::Type<UBJSONValue::UBJSONBinary>::value: Serializer<UBJSONEngine>::ActionBind<UBJSONValue::UBJSONBinary>::Serialize(e, obj.get<UBJSONValue::UBJSONBinary>(), 0); break;
    case UBJSONValue::Type<UBJSONValue::UBJSONArray>::value:
    {
      auto& v = obj.get<UBJSONValue::UBJSONArray>();

      UBJSONEngine::TYPE type = (v.Length() > 0) ? GetUBJSONVariantType(v[0]) : UBJSONEngine::TYPE_NONE;
      for(size_t i = 1; i < v.Length(); ++i)
        if(type != GetUBJSONVariantType(v[i]))
          type = UBJSONEngine::TYPE_NONE;

      internal::WriteUBJSONArray<UBJSONValue::UBJSONArray>(e, v, v.Length(), 0, type);
      break;
    }
    case UBJSONValue::Type<UBJSONValue::UBJSONObject>::value:
      internal::WriteUBJSONObject<UBJSONValue>(e, obj, s, ty);
      break;
    }
  }

  template<>
  inline void WriteUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, const char* id, const std::string& obj, std::ostream& s, UBJSONEngine::TYPE type) { UBJSONEngine::WriteUBJSONId(e, id, s); UBJSONEngine::WriteUBJSONString(e, obj.c_str(), obj.length(), s, type); }

  template<>
  inline void WriteUBJSONBase<Str>(Serializer<UBJSONEngine>& e, const char* id, const Str& obj, std::ostream& s, UBJSONEngine::TYPE type) { WriteUBJSONBase<std::string>(e, id, obj, s, type); }

  template<class T>
  inline void WriteUBJSON(const T& obj, std::ostream& s) { Serializer<UBJSONEngine> e; e.Serialize(obj, s); }

  template<typename T>
  void UBJSONEngine::Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id) { e.engine.endobject = true; WriteUBJSONBase<T>(e, id, t, *e.out, e.engine.type); }
  template<typename T>
  void UBJSONEngine::Parse(Serializer<UBJSONEngine>& e, T& t, const char* id) { ParseUBJSONBase<T>(e, t, *e.in); }
  template<typename F>
  void UBJSONEngine::ParseMany(Serializer<UBJSONEngine>& e, F && f)
  {
    ParseObj(e, e.engine.type, f);
  }
}

#endif