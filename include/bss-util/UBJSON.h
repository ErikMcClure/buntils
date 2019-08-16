// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __UBJSON_H__BSS__
#define __UBJSON_H__BSS__

#include "DynArray.h"
#include "Str.h"
#include "bss_util.h"
#include "Serializer.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <limits>
#include <functional>

namespace bss {
  struct BSS_DLLEXPORT UBJSONTuple
  {
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
    typedef DynArray<UBJSONTuple, size_t, ARRAY_CONSTRUCT> UBJSONArray;
    typedef DynArray<std::pair<Str, UBJSONTuple>, size_t, ARRAY_CONSTRUCT> UBJSONObject;

    UBJSONTuple(const UBJSONTuple& copy);
    UBJSONTuple(UBJSONTuple&& mov);
    UBJSONTuple(TYPE type, int64_t length, const char* s);
    template<class T>
    UBJSONTuple(TYPE type, T n) : Type(type), Length(-1), Int64(0)
    {
      if(Type == TYPE_NONE)
      {
        if constexpr(std::is_same_v<T, float>)
          Type = TYPE_FLOAT;
        else if constexpr(std::is_same_v<T, double>)
          Type = TYPE_DOUBLE;
        else
          Type = GetIntType(n);
      }

      switch(Type)
      {
      case TYPE_CHAR:
      case TYPE_INT8: Int8 = (char)n; break;
      case TYPE_UINT8: UInt8 = (uint8_t)n; break;
      case TYPE_INT16: Int16 = (short)n; break;
      case TYPE_INT32: Int32 = (int32_t)n; break;
      case TYPE_INT64: Int64 = (int64_t)n; break;
      case TYPE_FLOAT: Float = (float)n; break;
      case TYPE_BIGNUM:
      case TYPE_DOUBLE: Double = (double)n; break;
      default: assert(false); break;
      }
    }
    UBJSONTuple();
    ~UBJSONTuple();
    void Parse(std::istream& s, TYPE ty);
    static int64_t ParseLength(std::istream& s);
    static int64_t ParseTypeCount(std::istream& s, TYPE& type);
    void Write(std::ostream& s, TYPE type) const;
    static void WriteTypeCount(std::ostream& s, TYPE type, int64_t count);

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

    template<typename FnAdd, typename FnInsert, typename FnBulk>
    inline void Parse(std::istream& s, TYPE ty, FnAdd add, FnInsert insert, FnBulk bulkadd)
    {
      Type = (TYPE)ty;
      if(Type == TYPE_NONE)
      {
        do
          Type = (TYPE)s.get();
        while(Type == TYPE_NO_OP && !!s && s.peek() != -1);
      }

      switch(Type)
      {
      default:
        throw std::runtime_error("Unexpected character while parsing value.");
      case TYPE_NO_OP:
      case TYPE_NULL:
      case TYPE_TRUE:
      case TYPE_FALSE:
        break;
      case TYPE_ARRAY:
      {
        int64_t count = ParseTypeCount(s, ty);
        new (&Array) UBJSONArray(std::is_null_pointer_v<FnAdd> ? bssmax(count, 1) : 0);

        if constexpr(!std::is_null_pointer_v<FnBulk>)
          if(count >= 0 && (ty == TYPE_CHAR || ty == TYPE_UINT8 || ty == TYPE_INT8) && bulkadd(ty, count))
            break;

        while(!!s && s.peek() != -1 && (count > 0 || count < 0) && (count > 0 || s.peek() != TYPE_ARRAY_END))
        {
          if constexpr(std::is_null_pointer_v<FnAdd>)
          {
            Array.AddConstruct();
            Array.Back().Parse(s, ty);
          }
          else
            add(ty);

          --count;
        }

        if(count < 0 && !!s && s.peek() == TYPE_ARRAY_END)
          s.get(); // If we were looking for the ARRAY_END symbol, eat it.
        break;
      }
      case TYPE_OBJECT:
      {
        int64_t count = ParseTypeCount(s, ty);
        new (&Object) UBJSONObject(std::is_null_pointer_v<FnInsert> ? bssmax(count, 1) : 0);
        Str buf;
        while(!!s && s.peek() != -1 && (count > 0 || count < 0) && (count > 0 || s.peek() != TYPE_OBJECT_END))
        {
          int64_t length = ParseLength(s);
          buf.reserve(length + 1);
          s.read(buf.UnsafeString(), length);
          buf.UnsafeString()[length] = 0;

          if constexpr(std::is_null_pointer_v<FnInsert>)
          {
            Object.AddConstruct();
            Object.Back().first.assign(buf.data(), length);
            Object.Back().second.Parse(s, ty);
          }
          else
            insert(ty, buf.data(), length);

          --count;
        }
        if(count < 0 && !!s && s.peek() == TYPE_OBJECT_END)
          s.get(); // If we were looking for the OBJECT_END symbol, eat it.
        break;
      }
      case TYPE_CHAR:
      case TYPE_INT8: Int8 = ParseInteger<char>(s); break;
      case TYPE_UINT8: UInt8 = ParseInteger<uint8_t>(s); break;
      case TYPE_INT16: Int16 = ParseInteger<short>(s); break;
      case TYPE_INT32: Int32 = ParseInteger<int32_t>(s); break;
      case TYPE_INT64: Int64 = ParseInteger<int64_t>(s); break;
      case TYPE_FLOAT: Float = ParseInteger<float>(s); break;
      case TYPE_DOUBLE: Double = ParseInteger<double>(s); break;
      case TYPE_BIGNUM:
      case TYPE_STRING:
        Length = ParseLength(s);
        String = new char[(size_t)Length + 1];

        if(Length > 0)
          s.read(String, Length);

        String[Length] = 0;
        break;
      }
    }

    template<class T>
    inline static void WriteInteger(T v, std::ostream& s)
    {
#ifdef BSS_ENDIAN_LITTLE
      FlipEndian<T>(&v);
#endif
      s.write((char*)&v, sizeof(T));
    }

    template<class T, typename FROM>
    inline static constexpr bool CheckIntType(const FROM& obj)
    {
      return (obj >= (FROM)std::numeric_limits<typename std::conditional<std::is_unsigned<FROM>::value && sizeof(FROM) == sizeof(T), FROM, T>::type>::min() &&
        obj <= (FROM)std::numeric_limits<typename std::conditional<std::is_signed<FROM>::value && sizeof(FROM) == sizeof(T), FROM, T>::type>::max());
    }

    template<typename T>
    inline static TYPE GetIntType(T obj)
    {
      if(CheckIntType<uint8_t, T>(obj)) return TYPE_UINT8;
      if(CheckIntType<char, T>(obj)) return TYPE_INT8;
      if(CheckIntType<short, T>(obj)) return TYPE_INT16;
      if(CheckIntType<int32_t, T>(obj)) return TYPE_INT32;
      if(CheckIntType<int64_t, T>(obj)) return TYPE_INT64;
      return TYPE_BIGNUM;
    }

    inline static void WriteLength(std::ostream& s, int64_t length)
    {
      UBJSONTuple tuple(TYPE_NONE, length);
      tuple.Write(s, TYPE_NONE);
    }

    inline static void WriteString(std::ostream& s, const char* str, size_t len, TYPE type)
    {
      UBJSONTuple tuple(TYPE_STRING, len, str);
      tuple.Write(s, type);
      tuple.String = 0;
    }

    inline static void WriteId(std::ostream& s, const char* id)
    {
      if(id)
        WriteString(s, id, strlen(id), TYPE_STRING);
    }

    TYPE Type;
    int64_t Length;
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
      UBJSONArray Array;
      UBJSONObject Object;
    };
  };

  namespace internal {
    template<class T> struct WriteUBJSONType { static const UBJSONTuple::TYPE t = internal::serializer::is_serializer_array<T>::value ? UBJSONTuple::TYPE_ARRAY : UBJSONTuple::TYPE_OBJECT; };
    template<> struct WriteUBJSONType<uint8_t> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_UINT8; };
    template<> struct WriteUBJSONType<char> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_INT8; };
    template<> struct WriteUBJSONType<bool> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_NONE; };
    template<> struct WriteUBJSONType<short> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_INT16; };
    template<> struct WriteUBJSONType<int32_t> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_INT32; };
    template<> struct WriteUBJSONType<int64_t> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_INT64; };
    template<> struct WriteUBJSONType<float> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_FLOAT; };
    template<> struct WriteUBJSONType<double> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_DOUBLE; };
    template<> struct WriteUBJSONType<const char*> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_STRING; };
    template<> struct WriteUBJSONType<std::string> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_STRING; };
    template<> struct WriteUBJSONType<Str> { static const UBJSONTuple::TYPE t = UBJSONTuple::TYPE_STRING; };
  }

  class UBJSONEngine
  {
  public:
    UBJSONEngine() : type(UBJSONTuple::TYPE_NONE) {}
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<UBJSONEngine>& e) {}
    static void End(Serializer<UBJSONEngine>& e) {}
    template<typename T>
    static void Parse(Serializer<UBJSONEngine>& e, T& obj, const char* id)
    {
      if constexpr(std::is_base_of<std::string, T>::value)
      {
        UBJSONTuple tuple;
        tuple.Parse(*e.in, e.engine.type);
        switch(tuple.Type) // we will shove just about anything we possibly can into a string
        {
        case UBJSONTuple::TYPE_NULL: break;
        case UBJSONTuple::TYPE_CHAR: obj.assign(&tuple.Int8, 1);
        case UBJSONTuple::TYPE_INT8: obj = std::to_string((int)tuple.Int8); break;
        case UBJSONTuple::TYPE_UINT8: obj = std::to_string(tuple.UInt8); break;
        case UBJSONTuple::TYPE_INT16: obj = std::to_string(tuple.Int16); break;
        case UBJSONTuple::TYPE_INT32: obj = std::to_string(tuple.Int32); break;
        case UBJSONTuple::TYPE_INT64: obj = std::to_string(tuple.Int64); break;
        case UBJSONTuple::TYPE_FLOAT: obj = std::to_string(tuple.Float); break;
        case UBJSONTuple::TYPE_DOUBLE: obj = std::to_string(tuple.Double); break;
        case UBJSONTuple::TYPE_BIGNUM:
        case UBJSONTuple::TYPE_STRING:
          obj = tuple.String;
          break;
        }
      }
      else if constexpr(std::is_same<T, UBJSONTuple>::value)
        obj.Parse(*e.in, e.engine.type);
      else
      {
        static_assert(internal::serializer::is_serializable<UBJSONEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
        obj.template Serialize<UBJSONEngine>(e, 0);
      }
    }
    template<typename F>
    static void ParseMany(Serializer<UBJSONEngine>& e, F && f)
    {
      UBJSONTuple tuple;
      tuple.Parse(*e.in, e.engine.type, nullptr,
        [&](UBJSONTuple::TYPE ty, const char* id, size_t len)
      {
        internal::serializer::PushValue<UBJSONTuple::TYPE> push(e.engine.type, ty);
        f(e, id);
      }, nullptr);
      assert(tuple.Type == UBJSONTuple::TYPE_OBJECT);
    }
    template<typename T, typename E, void(*Add)(Serializer<UBJSONEngine>& e, T& obj, int& n), bool(*Read)(Serializer<UBJSONEngine>& e, T& obj, int64_t count)>
    static void ParseArray(Serializer<UBJSONEngine>& e, T& obj, const char* id)
    {
      int num = 0;
      UBJSONTuple tuple;
      tuple.Parse(*e.in, e.engine.type,
        [&](UBJSONTuple::TYPE ty)
      {
        internal::serializer::PushValue<UBJSONTuple::TYPE> push(e.engine.type, ty);
        Add(e, obj, num);
      },
        nullptr,
        [&](UBJSONTuple::TYPE, int64_t count)->bool { return Read(e, obj, count); });
      assert(tuple.Type == UBJSONTuple::TYPE_ARRAY);
    }
    template<typename T>
    static void ParseNumber(Serializer<UBJSONEngine>& e, T& obj, const char* id)
    {
      UBJSONTuple tuple;
      tuple.Parse(*e.in, e.engine.type);

      switch(tuple.Type) // as long as it's any arithmetic type, attempt to shove it into our target, casting if necessary.
      {
      case UBJSONTuple::TYPE_NULL: break;
      case UBJSONTuple::TYPE_TRUE: obj = (T)1; break;
      case UBJSONTuple::TYPE_FALSE: obj = (T)0; break;
      case UBJSONTuple::TYPE_CHAR:
      case UBJSONTuple::TYPE_INT8: obj = (T)tuple.Int8; break;
      case UBJSONTuple::TYPE_UINT8: obj = (T)tuple.UInt8; break;
      case UBJSONTuple::TYPE_INT16: obj = (T)tuple.Int16; break;
      case UBJSONTuple::TYPE_INT32: obj = (T)tuple.Int32; break;
      case UBJSONTuple::TYPE_INT64: obj = (T)tuple.Int64; break;
      case UBJSONTuple::TYPE_FLOAT: obj = (T)tuple.Float; break;
      case UBJSONTuple::TYPE_DOUBLE: obj = (T)tuple.Double; break;
      case UBJSONTuple::TYPE_BIGNUM: break;// we can't deal with bignum
      default: assert(false); break;
      }
    }
    static void ParseBool(Serializer<UBJSONEngine>& e, bool& obj, const char* id)
    {
      UBJSONTuple tuple;
      tuple.Parse(*e.in, e.engine.type);

      switch(tuple.Type)
      {
      case UBJSONTuple::TYPE_NULL: break;
      case UBJSONTuple::TYPE_TRUE: obj = true; break;
      case UBJSONTuple::TYPE_FALSE: obj = false; break;
      case UBJSONTuple::TYPE_CHAR:
      case UBJSONTuple::TYPE_INT8: obj = tuple.Int8 != 0; break;
      case UBJSONTuple::TYPE_UINT8: obj = tuple.UInt8 != 0; break;
      case UBJSONTuple::TYPE_INT16: obj = tuple.Int16 != 0; break;
      case UBJSONTuple::TYPE_INT32: obj = tuple.Int32 != 0; break;
      case UBJSONTuple::TYPE_INT64: obj = tuple.Int64 != 0; break;
      case UBJSONTuple::TYPE_BIGNUM: break;
      default: assert(false); break;
      }
    }

    template<typename T>
    static void Serialize(Serializer<UBJSONEngine>& e, const T& obj, const char* id)
    {
      UBJSONTuple::WriteId(*e.out, id);
      if constexpr(std::is_base_of<std::string, T>::value)
      {
        UBJSONTuple tuple(UBJSONTuple::TYPE_STRING, obj.size(), obj.data());
        tuple.Write(*e.out, e.engine.type);
        tuple.String = 0;
      }
      else if constexpr(std::is_same<T, UBJSONTuple>::value)
        obj.Write(*e.out, e.engine.type);
      else
      {
        if(!e.engine.type)
          e.out->put(UBJSONTuple::TYPE_OBJECT);
        else if(e.engine.type != UBJSONTuple::TYPE_OBJECT)
          throw std::runtime_error("Expecting a type other than object in the object serializing function!");

        internal::serializer::PushValue<UBJSONTuple::TYPE> push(e.engine.type, UBJSONTuple::TYPE_NONE);
        static_assert(internal::serializer::is_serializable<UBJSONEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
        const_cast<T&>(obj).template Serialize<UBJSONEngine>(e, 0);

        e.out->put(UBJSONTuple::TYPE_OBJECT_END);
      }
    }
    template<typename T>
    static void SerializeArray(Serializer<UBJSONEngine>& e, const T& obj, size_t size, const char* id)
    {
      std::ostream& s = *e.out;
      UBJSONTuple::WriteId(s, id);

      if(!e.engine.type)
        s.put(UBJSONTuple::TYPE_ARRAY);
      else
        assert(e.engine.type == UBJSONTuple::TYPE_ARRAY);
      if(!size) // It's more efficient to skip type/length information for zero length arrays.
      {
        s.put(UBJSONTuple::TYPE_ARRAY_END);
        return;
      }
      UBJSONTuple::TYPE ty = internal::WriteUBJSONType<remove_cvref_t<decltype(*std::begin(obj))>>::t;
      UBJSONTuple::WriteTypeCount(s, ty, size);

      if(ty != UBJSONTuple::TYPE_CHAR || ty != UBJSONTuple::TYPE_UINT8 || ty != UBJSONTuple::TYPE_INT8 || !e.BulkWrite(std::begin(obj), std::end(obj), size))
      {
        auto begin = std::begin(obj);
        auto end = std::end(obj);
        internal::serializer::PushValue<UBJSONTuple::TYPE> push(e.engine.type, ty);
        for(; begin != end; ++begin)
          Serializer<UBJSONEngine>::ActionBind<remove_cvref_t<decltype(*begin)>>::Serialize(e, *begin, 0);
      }

      if(size < 0)
        s.put(UBJSONTuple::TYPE_ARRAY_END);
    }

    template<typename T, size_t... S>
    static void SerializeTuple(Serializer<UBJSONEngine>& e, const T& t, const char* id, std::index_sequence<S...>)
    {
      std::ostream& s = *e.out;
      UBJSONTuple::WriteId(s, id);

      if(!e.engine.type)
        s.put(UBJSONTuple::TYPE_ARRAY);
      else
        assert(e.engine.type == UBJSONTuple::TYPE_ARRAY);
      static_assert(sizeof...(S) > 0);
      UBJSONTuple::WriteTypeCount(s, UBJSONTuple::TYPE_NONE, sizeof...(S));

      internal::serializer::PushValue<UBJSONTuple::TYPE> push(e.engine.type, UBJSONTuple::TYPE_NONE);
      (Serializer<UBJSONEngine>::ActionBind<std::tuple_element_t<S, T>>::Serialize(e, std::get<S>(t), 0), ...);
    }

    template<typename T>
    static void SerializeNumber(Serializer<UBJSONEngine>& e, T t, const char* id)
    {
      UBJSONTuple::WriteId(*e.out, id);
      UBJSONTuple tuple(e.engine.type, t);
      tuple.Write(*e.out, e.engine.type);
    }
    static void SerializeBool(Serializer<UBJSONEngine>& e, bool t, const char* id)
    {
      UBJSONTuple::WriteId(*e.out, id);
      if(!e.engine.type)
        e.out->put(t ? UBJSONTuple::TYPE_TRUE : UBJSONTuple::TYPE_FALSE);
    }

    UBJSONTuple::TYPE type;
  };
}

#endif