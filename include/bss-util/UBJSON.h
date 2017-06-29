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
    template<typename T>
    static void Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id);
    template<typename T>
    static void Parse(Serializer<UBJSONEngine>& e, T& t, const char* id);
    template<typename... Args>
    static void ParseMany(Serializer<UBJSONEngine>& e, const Trie<uint16_t>& t, std::tuple<Args...>& args);

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
      TYPE backup = e.engine.type;
      e.engine.type = TYPE_NONE;
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

      e.engine.type = backup;
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

    TYPE type;
    bool endobject;
  };

  template<class T>
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, UBJSONEngine::TYPE type);

  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s, UBJSONEngine::TYPE type);

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, UBJSONEngine::TYPE type = UBJSONEngine::TYPE_NONE);

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE type);

  template<typename... Args>
  inline const UBJSONEngine::TYPE GetUBJSONVariantType(const Variant<Args...>& v);

  namespace internal {
    template<class T, class BASE>
    struct __UBJSONValue_conv
    {
      inline static constexpr T&& f(typename std::remove_reference<T>::type& r) { return (static_cast<T&&>(r)); }
      inline static constexpr T&& f(typename std::remove_reference<T>::type&& r) { return (static_cast<T&&>(r)); }
    };
    template<class BASE>
    struct __UBJSONValue_conv<UBJSONValue, BASE>
    {
      inline static constexpr BASE&& f(std::remove_reference<UBJSONValue>::type& r) { return (static_cast<BASE&&>(r)); }
      inline static constexpr BASE&& f(std::remove_reference<UBJSONValue>::type&& r) { return (static_cast<BASE&&>(r)); }
    };
    template<class BASE>
    struct __UBJSONValue_conv<const UBJSONValue, BASE>
    {
      inline static constexpr const BASE&& f(std::remove_reference<const UBJSONValue>::type& r) { return (static_cast<const BASE&&>(r)); }
      inline static constexpr const BASE&& f(std::remove_reference<const UBJSONValue>::type&& r) { return (static_cast<const BASE&&>(r)); }
    };
  }

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
    explicit UBJSONValue(T&& t) : BASE(internal::__UBJSONValue_conv<T, BASE>::f(t)) {}
    ~UBJSONValue() {}
    BASE& operator=(const BASE& right) { BASE::operator=(right); return *this; }
    BASE& operator=(BASE&& right) { BASE::operator=(std::move(right)); return *this; }
    template<typename T>
    BASE& operator=(const T& right) { BASE::operator=(right); return *this; }
    template<typename T>
    BASE& operator=(T&& right) { BASE::operator=(internal::__UBJSONValue_conv<T, BASE>::f(right)); return *this; }

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
        WriteUBJSONBase<size_t>(e, 0, v.Length(), s, UBJSONEngine::TYPE_NONE);

        for(auto& i : v)
          WriteUBJSONBase<UBJSONValue>(e, i.first, i.second, s, type);

        e.engine.endobject = false;
        //return false;
      }
    }
  };

  namespace internal {
    template<class T, bool B>
    struct ParseUBJSONInternal
    {
      static void F(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, UBJSONEngine::TYPE ty) { UBJSONEngine::TYPE backup = e.engine.type; e.engine.type = ty; obj.template Serialize<UBJSONEngine>(e); e.engine.type = backup; }
    };

    template<class T>
    struct ParseUBJSONInternal<T, true>
    {
      static void F(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, UBJSONEngine::TYPE type)
      {
        UBJSONEngine::UBJSONTuple tuple;
        UBJSONEngine::ParseValue(tuple, s, type);

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
    };

    template<class T>
    inline void ParseUBJSONArray(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, UBJSONEngine::TYPE ty)
    {
      if(ty != UBJSONEngine::TYPE_NONE && ty != UBJSONEngine::TYPE_ARRAY) // Sanity check
        throw std::runtime_error("Expecting a type other than array in the array parsing function!");
      if(!ty && (s.get() != UBJSONEngine::TYPE_ARRAY))
        throw std::runtime_error("Expected array, found invalid character");
      int64_t num = 0;
      UBJSONEngine::TYPE type = UBJSONEngine::TYPE_NONE;
      int64_t count = UBJSONEngine::ParseTypeCount(s, type);
      if(count <= 0 || type == UBJSONEngine::TYPE_NONE || !ParseUBJSONInternal<T, false>::DoBulkRead(e, obj, s, count, type)) // attempt mass read in if we have both a type and a count
      {
        while(!!s && (count > 0 || count < 0) && (count > 0 || s.peek() != UBJSONEngine::TYPE_ARRAY_END) && s.peek() != -1)
        {
          ParseUBJSONInternal<T, false>::DoAddCall(e, obj, s, num, type);
          --count;
        }
        if(!!s && count < 0 && s.peek() == UBJSONEngine::TYPE_ARRAY_END) s.get(); // If we were looking for the ARRAY_END symbol, eat it.
      }
    }

    template<class T, int I, bool B>
    struct ParseUBJSONInternal<T[I], B>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, int64_t count, UBJSONEngine::TYPE ty)
      {
        if((ty != UBJSONEngine::TYPE_CHAR && ty != UBJSONEngine::TYPE_UINT8 && ty != UBJSONEngine::TYPE_INT8) || count != (I * sizeof(T)))
          return false;

        s.read((char*)obj, count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, int64_t& n, UBJSONEngine::TYPE ty) { if(n < I) ParseUBJSONBase<T>(e, obj[n++], s, ty); }
      static void F(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, UBJSONEngine::TYPE ty) { ParseUBJSONArray<T[I]>(e, obj, s, ty); }
    };

    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct ParseUBJSONInternal<DynArray<T, CType, ArrayType, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t count, UBJSONEngine::TYPE ty)
      {
        if((ty != UBJSONEngine::TYPE_CHAR && ty != UBJSONEngine::TYPE_UINT8 && ty != UBJSONEngine::TYPE_INT8) || count % sizeof(T) != 0)
          return false;

        obj.SetLength(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        s.read((char*)(T*)obj, count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t& n, UBJSONEngine::TYPE ty) { obj.AddConstruct(); ParseUBJSONBase<T>(e, obj.Back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        obj.Clear();
        ParseUBJSONArray<DynArray<T, CType, ArrayType, Alloc>>(e, obj, s, ty);
      }
    };

    template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct ParseUBJSONInternal<DynArray<bool, CType, ArrayType, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t count, UBJSONEngine::TYPE ty) { return false; }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t& n, UBJSONEngine::TYPE ty) { bool b; ParseUBJSONBase<bool>(e, b, s, ty); obj.Add(b); }
      static void F(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        obj.Clear();
        ParseUBJSONArray<DynArray<bool, CType, ArrayType, Alloc>>(e, obj, s, ty);
      }
    };

    template<class T, typename Alloc>
    struct ParseUBJSONInternal<std::vector<T, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int64_t count, UBJSONEngine::TYPE ty)
      {
        if((ty != UBJSONEngine::TYPE_CHAR && ty != UBJSONEngine::TYPE_UINT8 && ty != UBJSONEngine::TYPE_INT8) || count % sizeof(T) != 0)
          return false;

        obj.resize(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        s.read((char*)obj.data(), count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int64_t& n, UBJSONEngine::TYPE ty) { obj.push_back(T()); ParseUBJSONBase<T>(e, obj.back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        obj.clear();
        ParseUBJSONArray<std::vector<T, Alloc>>(e, obj, s, ty);
      }
    };

    template<>
    struct ParseUBJSONInternal<UBJSONValue::UBJSONArray, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, int64_t count, UBJSONEngine::TYPE ty) { return false; }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, int64_t& n, UBJSONEngine::TYPE ty) { obj.SetLength(obj.Length() + 1); ParseUBJSONBase<UBJSONValue>(e, obj.Back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, UBJSONEngine::TYPE ty) { ParseUBJSONArray<UBJSONValue::UBJSONArray>(e, obj, s, ty); }
    };

    template<>
    struct ParseUBJSONInternal<UBJSONValue::UBJSONObject, false>
    {
      static void F(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONObject& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        UBJSONEngine::ParseObj(e, ty, [&obj, &s](Serializer<UBJSONEngine>& e, const char* id) {
          std::pair<Str, UBJSONValue> pair;
          pair.first = id;
          ParseUBJSONBase<UBJSONValue>(e, pair.second, s, e.engine.type);
          obj.Add(pair);
        });
      }
    };

    template<typename T, typename Arg, typename... Args>
    struct ParseUBJSONVariantInternal
    {
      static void F(Serializer<UBJSONEngine>& e, int tag, T& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        if(T::template Type<Arg>::value == tag)
        {
          obj.template typeset<Arg>();
          ParseUBJSONBase<Arg>(e, obj.template get<Arg>(), s, ty);
        }
        else
          ParseUBJSONVariantInternal<T, Args...>::F(e, tag, obj, s, ty);
      }
    };

    template<typename T, typename Arg>
    struct ParseUBJSONVariantInternal<T, Arg>
    {
      static void F(Serializer<UBJSONEngine>& e, int tag, T& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        if(T::template Type<Arg>::value == tag)
        {
          obj.template typeset<Arg>();
          ParseUBJSONBase<Arg>(e, obj.template get<Arg>(), s, ty);
        }
        else
          assert(obj.tag() != -1);
      }
    };

    template<typename... Args>
    struct ParseUBJSONInternal<Variant<Args...>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, Variant<Args...>& obj, std::istream& s, UBJSONEngine::TYPE ty)
      {
        int tag = -1;
        UBJSONEngine::ParseObj(e, ty, [&obj, &s, &tag](Serializer<UBJSONEngine>& e, const char* id) {
          assert(id[0] != 0 && id[1] == 0);
          switch(id[0])
          {
          case 't':
            ParseUBJSONBase<int>(e, tag, s, UBJSONEngine::TYPE_NONE);
            break;
          case 'o':
            ParseUBJSONVariantInternal<Variant<Args...>, Args...>::F(e, tag, obj, s, UBJSONEngine::TYPE_NONE);
            break;
          }
        });
      }
    };
  }

  template<class T>
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, UBJSONEngine::TYPE type) { internal::ParseUBJSONInternal<T, std::is_arithmetic<T>::value>::F(e, obj, s, type); }

  template<>
  inline void ParseUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, std::string& obj, std::istream& s, UBJSONEngine::TYPE type)
  {
    UBJSONEngine::UBJSONTuple tuple;
    UBJSONEngine::ParseValue(tuple, s, type);

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
  inline void ParseUBJSONBase<Str>(Serializer<UBJSONEngine>& e, Str& obj, std::istream& s, UBJSONEngine::TYPE type) { ParseUBJSONBase<std::string>(e, obj, s, type); }

  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s, UBJSONEngine::TYPE type)
  {
    UBJSONEngine::UBJSONTuple tuple;
    UBJSONEngine::ParseValue(tuple, s, type);

    switch(tuple.type)
    {
    case UBJSONEngine::TYPE_NO_OP: break;
    case UBJSONEngine::TYPE_NULL: break;
    case UBJSONEngine::TYPE_TRUE: obj = true; break;
    case UBJSONEngine::TYPE_FALSE: obj = false; break;
    case UBJSONEngine::TYPE_ARRAY:
      obj = UBJSONValue::UBJSONArray();
      internal::ParseUBJSONInternal<UBJSONValue::UBJSONArray, false>::F(e, obj.get<UBJSONValue::UBJSONArray>(), s, UBJSONEngine::TYPE_ARRAY);
      break;
    case UBJSONEngine::TYPE_OBJECT:
      obj = UBJSONValue::UBJSONObject();
      internal::ParseUBJSONInternal<UBJSONValue::UBJSONObject, false>::F(e, obj.get<UBJSONValue::UBJSONObject>(), s, UBJSONEngine::TYPE_OBJECT);
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
      if(ty != UBJSONEngine::TYPE_NONE && ty != UBJSONEngine::TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object serializing function!");
      if(!ty)
        s.put(UBJSONEngine::TYPE_OBJECT);

      const_cast<T&>(obj).template Serialize<UBJSONEngine>(e);
      if(e.engine.endobject)
      //if(obj.SerializeUBJSON(s))
        s.put(UBJSONEngine::TYPE_OBJECT_END);
    }

    template<class T, bool B>
    struct WriteUBJSONInternal
    {
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, UBJSONEngine::TYPE ty) { WriteUBJSONObject<T>(e, obj, s, ty); }
    };

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

    template<class T>
    struct WriteUBJSONInternal<T, true>
    {
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, UBJSONEngine::TYPE type)
      {
        switch(type)
        {
        case 0:
          if(!WriteUBJSONSpecificInt<uint8_t, T>(obj, s, UBJSONEngine::TYPE_UINT8) &&
            !WriteUBJSONSpecificInt<char, T>(obj, s, UBJSONEngine::TYPE_INT8) &&
            !WriteUBJSONSpecificInt<short, T>(obj, s, UBJSONEngine::TYPE_INT16) &&
            !WriteUBJSONSpecificInt<int32_t, T>(obj, s, UBJSONEngine::TYPE_INT32) &&
            !WriteUBJSONSpecificInt<int64_t, T>(obj, s, UBJSONEngine::TYPE_INT64))
          {
            s.put(UBJSONEngine::TYPE_BIGNUM);
            throw std::runtime_error("Unknown large integer type!");
          }
          break;
        case UBJSONEngine::TYPE_CHAR:
        case UBJSONEngine::TYPE_INT8: WriteUBJSONInteger<char>((char)obj, s); break;
        case UBJSONEngine::TYPE_UINT8: WriteUBJSONInteger<uint8_t>((uint8_t)obj, s); break;
        case UBJSONEngine::TYPE_INT16: WriteUBJSONInteger<short>((short)obj, s); break;
        case UBJSONEngine::TYPE_INT32: WriteUBJSONInteger<int32_t>((int32_t)obj, s); break;
        case UBJSONEngine::TYPE_INT64: WriteUBJSONInteger<int64_t>((int64_t)obj, s); break;
        case UBJSONEngine::TYPE_BIGNUM: break; // we can't deal with bignum
        }
      }
    };

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

    template<class E, class T>
    inline void WriteUBJSONArray(Serializer<UBJSONEngine>& e, T obj, const char* data, size_t size, std::ostream& s, UBJSONEngine::TYPE type)
    {
      if(!size) // It's more efficient to skip type/length information for zero length arrays.
      {
        s.put(UBJSONEngine::TYPE_ARRAY_END);
        return;
      }
      if(type)
      {
        s.put(UBJSONEngine::TYPE_TYPE);
        s.put(type);
      }
      s.put(UBJSONEngine::TYPE_COUNT);
      WriteUBJSONBase<size_t>(e, size, s);
      if(data != 0 && (type == UBJSONEngine::TYPE_CHAR || type == UBJSONEngine::TYPE_UINT8 || type == UBJSONEngine::TYPE_INT8))
        s.write(data, size * sizeof(E)); //sizeof(E) should be 1 here but we multiply it anyway
      else
      {
        for(size_t i = 0; i < size; ++i)
          WriteUBJSONBase<E>(e, obj[i], s, type);
      }
    }

    template<class T, int I>
    struct WriteUBJSONInternal<T[I], false>
    {
      static void F(Serializer<UBJSONEngine>& e, const T(&obj)[I], std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(!ty) 
          s.put(UBJSONEngine::TYPE_ARRAY);
        WriteUBJSONArray<T, const T*>(e, (const T*)obj, (const char*)obj, I, s, WriteUBJSONType<T>::t);
      }
    };

    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONInternal<DynArray<T, CType, ArrayType, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const DynArray<T, CType, ArrayType, Alloc>& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(!ty) 
          s.put(UBJSONEngine::TYPE_ARRAY);
        WriteUBJSONArray<T>(e, obj, (const char*)obj.begin(), obj.Length(), s, WriteUBJSONType<T>::t);
      }
    };

    template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONInternal<DynArray<bool, CType, ArrayType, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const DynArray<bool, CType, ArrayType, Alloc>& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(!ty) 
          s.put(UBJSONEngine::TYPE_ARRAY);
        WriteUBJSONArray<bool>(e, obj, 0, obj.Length(), s, UBJSONEngine::TYPE_NONE);
      }
    };

    template<class T, typename Alloc>
    struct WriteUBJSONInternal<std::vector<T, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const std::vector<T, Alloc>& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(!ty)
          s.put(UBJSONEngine::TYPE_ARRAY);
        WriteUBJSONArray<T>(e, obj, (const char*)obj.data(), obj.size(), s, WriteUBJSONType<T>::t);
      }
    };

    template<typename T, typename Arg, typename... Args>
    struct WriteUBJSONVariantInternal
    {
      static void F(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(obj.template is<Arg>())
          WriteUBJSONBase<Arg>(e, id, obj.template get<Arg>(), s, ty);
        else
          WriteUBJSONVariantInternal<T, Args...>::F(e, id, obj, s, ty);
      }
    };

    template<typename T, typename Arg>
    struct WriteUBJSONVariantInternal<T, Arg>
    {
      static void F(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(obj.template is<Arg>())
          WriteUBJSONBase<Arg>(e, id, obj.template get<Arg>(), s, ty);
        else
          assert(obj.tag() != -1);
      }
    };

    template<typename... Args>
    struct WriteUBJSONInternal<Variant<Args...>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const Variant<Args...>& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        if(ty != UBJSONEngine::TYPE_NONE && ty != UBJSONEngine::TYPE_OBJECT) // Sanity check
          throw std::runtime_error("Expecting a type other than object in the object serializing function!");
        if(!ty)
          s.put(UBJSONEngine::TYPE_OBJECT); // Start a new object

        bool endobject = e.engine.endobject;
        e.engine.endobject = true;
        WriteUBJSONBase<int>(e, "t", obj.tag(), s, UBJSONEngine::TYPE_NONE); // Write the type tag
        WriteUBJSONVariantInternal<Variant<Args...>, Args...>::F(e, "o", obj, s, UBJSONEngine::TYPE_NONE); // Write the actual variant object
        if(endobject)
          s.put(UBJSONEngine::TYPE_OBJECT_END);
      }
    };

    template<>
    struct WriteUBJSONInternal<UBJSONValue, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const UBJSONValue& obj, std::ostream& s, UBJSONEngine::TYPE ty)
      {
        switch(obj.tag())
        {
        case UBJSONValue::Type<Str>::value: WriteUBJSONBase<Str>(e, obj.get<Str>(), s, ty); break;
        case UBJSONValue::Type<bool>::value: WriteUBJSONBase<bool>(e, obj.get<bool>(), s, ty); break;
        case UBJSONValue::Type<uint8_t>::value: WriteUBJSONBase<uint8_t>(e, obj.get<uint8_t>(), s, ty); break;
        case UBJSONValue::Type<char>::value: WriteUBJSONBase<char>(e, obj.get<char>(), s, ty); break;
        case UBJSONValue::Type<int16_t>::value: WriteUBJSONBase<int16_t>(e, obj.get<int16_t>(), s, ty); break;
        case UBJSONValue::Type<int32_t>::value: WriteUBJSONBase<int32_t>(e, obj.get<int32_t>(), s, ty); break;
        case UBJSONValue::Type<int64_t>::value: WriteUBJSONBase<int64_t>(e, obj.get<int64_t>(), s, ty); break;
        case UBJSONValue::Type<float>::value: WriteUBJSONBase<float>(e, obj.get<float>(), s, ty); break;
        case UBJSONValue::Type<double>::value: WriteUBJSONBase<double>(e, obj.get<double>(), s, ty); break;
        case UBJSONValue::Type<UBJSONValue::UBJSONBinary>::value: WriteUBJSONBase<UBJSONValue::UBJSONBinary>(e, obj.get<UBJSONValue::UBJSONBinary>(), s, ty); break;
        case UBJSONValue::Type<UBJSONValue::UBJSONArray>::value:
        {
          auto& v = obj.get<UBJSONValue::UBJSONArray>();

          UBJSONEngine::TYPE type = (v.Length() > 0) ? GetUBJSONVariantType(v[0]) : UBJSONEngine::TYPE_NONE;
          for(size_t i = 1; i < v.Length(); ++i)
            if(type != GetUBJSONVariantType(v[i]))
              type = UBJSONEngine::TYPE_NONE;

          assert(!ty || ty == UBJSONEngine::TYPE_ARRAY);
          if(!ty) 
            s.put(UBJSONEngine::TYPE_ARRAY);

          WriteUBJSONArray<UBJSONValue, const UBJSONValue*>(e, (const UBJSONValue*)v, 0, v.Length(), s, type);
          break;
        }
        case UBJSONValue::Type<UBJSONValue::UBJSONObject>::value:
          WriteUBJSONObject<UBJSONValue>(e, obj, s, ty);
          break;
        }
      }
    };

    inline void WriteUBJSONString(Serializer<UBJSONEngine>& e, const char* str, size_t len, std::ostream& s, UBJSONEngine::TYPE type)
    {
      if(!type)
        s.put(UBJSONEngine::TYPE_STRING);

      WriteUBJSONBase<size_t>(e, len, s);
      s.write(str, len);
    }

    inline void WriteUBJSONId(Serializer<UBJSONEngine>& e, const char* id, std::ostream& s)
    {
      if(id) 
        WriteUBJSONString(e, id, strlen(id), s, UBJSONEngine::TYPE_STRING);
    }
  }

  template<typename... Args>
  inline const UBJSONEngine::TYPE GetUBJSONVariantType(const Variant<Args...>& v) { return internal::__UBJSONVariantType<Variant<Args...>, Args...>::F(v); }

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, UBJSONEngine::TYPE type) { internal::WriteUBJSONId(e, id, s); internal::WriteUBJSONInternal<T, std::is_integral<T>::value>::F(e, obj, s, type); }

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, UBJSONEngine::TYPE type) { WriteUBJSONBase<T>(e, 0, obj, s, type); }

  template<>
  inline void WriteUBJSONBase<bool>(Serializer<UBJSONEngine>& e, const char* id, const bool& obj, std::ostream& s, UBJSONEngine::TYPE type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(obj ? UBJSONEngine::TYPE_TRUE : UBJSONEngine::TYPE_FALSE); }

  template<>
  inline void WriteUBJSONBase<float>(Serializer<UBJSONEngine>& e, const char* id, const float& obj, std::ostream& s, UBJSONEngine::TYPE type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(UBJSONEngine::TYPE_FLOAT); internal::WriteUBJSONInteger<float>(obj, s); }

  template<>
  inline void WriteUBJSONBase<double>(Serializer<UBJSONEngine>& e, const char* id, const double& obj, std::ostream& s, UBJSONEngine::TYPE type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(UBJSONEngine::TYPE_DOUBLE); internal::WriteUBJSONInteger<double>(obj, s); }

  template<>
  inline void WriteUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, const char* id, const std::string& obj, std::ostream& s, UBJSONEngine::TYPE type) { internal::WriteUBJSONId(e, id, s); internal::WriteUBJSONString(e, obj.c_str(), obj.length(), s, type); }

  template<>
  inline void WriteUBJSONBase<Str>(Serializer<UBJSONEngine>& e, const char* id, const Str& obj, std::ostream& s, UBJSONEngine::TYPE type) { WriteUBJSONBase<std::string>(e, id, obj, s, type); }

  template<class T>
  inline void WriteUBJSON(const T& obj, std::ostream& s) { Serializer<UBJSONEngine> e; e.Serialize(obj, s); }

  template<typename T>
  void UBJSONEngine::Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id) { e.engine.endobject = true; WriteUBJSONBase<T>(e, id, t, *e.out, UBJSONEngine::TYPE_NONE); }
  template<typename T>
  void UBJSONEngine::Parse(Serializer<UBJSONEngine>& e, T& t, const char* id) { ParseUBJSONBase<T>(e, t, *e.in, e.engine.type); }
  template<typename... Args>
  void UBJSONEngine::ParseMany(Serializer<UBJSONEngine>& e, const Trie<uint16_t>& t, std::tuple<Args...>& args)
  {
    ParseObj(e, e.engine.type, [&t, &args](Serializer<UBJSONEngine>& e, const char* id) {
      Serializer<UBJSONEngine>::_findparse(e, id, t, args);
    });
  }
}

#endif