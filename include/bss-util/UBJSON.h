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
  class UBJSONEngine
  {
  public:
    UBJSONEngine() : type(0), endobject(false) {}
    static constexpr bool Ordered() { return false; }
    template<typename T>
    static void Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id);
    template<typename T>
    static void Parse(Serializer<UBJSONEngine>& e, T& t, const char* id);
    template<typename... Args>
    static void ParseMany(Serializer<UBJSONEngine>& e, const Trie<uint16_t>& t, std::tuple<Args...>& args);

    char type;
    bool endobject;
  };

  struct UBJSONTuple
  {
    UBJSONTuple() : type(TYPE_NO_OP), length(-1), Int64(0) {}
    ~UBJSONTuple()
    {
      if((type == TYPE_STRING || type == TYPE_BIGNUM) && String != 0)
        delete[] String;
    }

    enum TYPE : char {
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
    } type;

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
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, char type);

  struct UBJSONValue;
  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s, char type);

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char type = 0);

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, char type);

  template<typename... Args>
  inline const char GetUBJSONVariantType(const Variant<Args...>& v);

  namespace internal {
    template<class T>
    inline T ParseUBJSONInteger(std::istream& s)
    {
      T v;
      s.read((char*)&v, sizeof(T));
#ifdef BSS_ENDIAN_LITTLE
      FlipEndian<T>(&v);
#endif
      return v;
    }

    inline int64_t ParseUBJSONLength(std::istream& s)
    {
      int64_t ret = -1;
      while(s)
      {
        switch(s.get())
        {
        case UBJSONTuple::TYPE_CHAR: // you aren't supposed to do this but we'll deal with it anyway
        case UBJSONTuple::TYPE_INT8: ret = ParseUBJSONInteger<char>(s); break;
        case UBJSONTuple::TYPE_UINT8: ret = ParseUBJSONInteger<uint8_t>(s); break;
        case UBJSONTuple::TYPE_INT16: ret = ParseUBJSONInteger<short>(s); break;
        case UBJSONTuple::TYPE_INT32: ret = ParseUBJSONInteger<int32_t>(s); break;
        case UBJSONTuple::TYPE_INT64: ret = ParseUBJSONInteger<int64_t>(s); break;
        case UBJSONTuple::TYPE_NO_OP: continue; // try again
        default:
          throw std::runtime_error("Invalid length type");
        }
        break;
      }

      if(ret < 0)
        throw std::runtime_error("Negative length is not allowed.");
      return ret;
    }

    inline int64_t ParseUBJSONTypeCount(std::istream& s, char& type)
    {
      type = 0;
      if(!!s && s.peek() == UBJSONTuple::TYPE_TYPE)
      {
        s.get(); // eat '$'
        type = s.get();
      }

      if(!!s && s.peek() == UBJSONTuple::TYPE_COUNT)
      {
        s.get(); // eat '#'
        return ParseUBJSONLength(s);
      }
      else if(type != 0)
        throw std::runtime_error("A type was specified, but no count was given. A count MUST follow a type!");
      return -1;
    }

    template<class F>
    inline void ParseUBJSONObj(Serializer<UBJSONEngine>& e, char ty, F f)
    {
      std::istream& s = *e.in;

      if(ty != 0 && ty != UBJSONTuple::TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object parsing function!");
      if(!ty && (s.get() != UBJSONTuple::TYPE_OBJECT))
        throw std::runtime_error("Expected object, found invalid character");

      Str buf;
      char backup = e.engine.type;
      e.engine.type = 0;
      int64_t count = ParseUBJSONTypeCount(s, e.engine.type);

      while(!!s && (count < 0 || count>0) && (count > 0 || s.peek() != UBJSONTuple::TYPE_OBJECT_END) && s.peek() != -1)
      {
        int64_t length = ParseUBJSONLength(s);
        --count;
        buf.reserve(length + 1);
        s.read(buf.UnsafeString(), length);
        buf.UnsafeString()[length] = 0;
        f(e, buf.c_str());
      }

      e.engine.type = backup;
      if(!!s && count < 0 && s.peek() == UBJSONTuple::TYPE_OBJECT_END) 
        s.get(); // If we were looking for the OBJECT_END symbol, eat it.
    }

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

        char type = (v.Length() > 0) ? GetUBJSONVariantType(v[0].second) : 0;
        for(size_t i = 1; i < v.Length(); ++i)
          if(type != GetUBJSONVariantType(v[i].second))
            type = 0;

        if(type)
        {
          s.put(UBJSONTuple::TYPE_TYPE);
          s.put(type);
        }

        s.put(UBJSONTuple::TYPE_COUNT);
        WriteUBJSONBase<size_t>(e, 0, v.Length(), s, 0);

        for(auto& i : v)
          WriteUBJSONBase<UBJSONValue>(e, i.first, i.second, s, type);

        e.engine.endobject = false;
        //return false;
      }
    }
  };

  namespace internal {
    inline void ParseUBJSONValue(UBJSONTuple& tuple, std::istream& s, char type)
    {
      if(type != 0)
        tuple.type = (UBJSONTuple::TYPE)type;
      else
      {
        do
          tuple.type = (UBJSONTuple::TYPE)s.get();
        while(tuple.type == UBJSONTuple::TYPE_NO_OP);
      }

      switch(tuple.type)
      {
      default:
        throw std::runtime_error("Unexpected character while parsing value.");
      case UBJSONTuple::TYPE_NO_OP:
      case UBJSONTuple::TYPE_NULL:
      case UBJSONTuple::TYPE_TRUE:
      case UBJSONTuple::TYPE_FALSE:
      case UBJSONTuple::TYPE_ARRAY:
      case UBJSONTuple::TYPE_OBJECT:
        return;
      case UBJSONTuple::TYPE_CHAR:
      case UBJSONTuple::TYPE_INT8: tuple.Int8 = ParseUBJSONInteger<char>(s); break;
      case UBJSONTuple::TYPE_UINT8: tuple.UInt8 = ParseUBJSONInteger<uint8_t>(s); break;
      case UBJSONTuple::TYPE_INT16: tuple.Int16 = ParseUBJSONInteger<short>(s); break;
      case UBJSONTuple::TYPE_INT32: tuple.Int32 = ParseUBJSONInteger<int32_t>(s); break;
      case UBJSONTuple::TYPE_INT64: tuple.Int64 = ParseUBJSONInteger<int64_t>(s); break;
      case UBJSONTuple::TYPE_FLOAT: tuple.Float = ParseUBJSONInteger<float>(s); break;
      case UBJSONTuple::TYPE_DOUBLE: tuple.Double = ParseUBJSONInteger<double>(s); break;
      case UBJSONTuple::TYPE_BIGNUM:
      case UBJSONTuple::TYPE_STRING:
        tuple.length = ParseUBJSONLength(s);
        tuple.String = new char[(size_t)tuple.length + 1];

        if(tuple.length > 0) 
          s.read(tuple.String, tuple.length);

        tuple.String[tuple.length] = 0;
        break;
      }
    }

    template<class T, bool B>
    struct ParseUBJSONInternal
    {
      static void F(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, char ty) { char backup = e.engine.type; e.engine.type = ty; obj.template Serialize<UBJSONEngine>(e); e.engine.type = backup; }
    };

    template<class T>
    struct ParseUBJSONInternal<T, true>
    {
      static void F(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, char type)
      {
        UBJSONTuple tuple;
        ParseUBJSONValue(tuple, s, type);

        switch(tuple.type) // as long as it's any arithmetic type, attempt to shove it into our target, casting if necessary.
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
        }
      }
    };

    template<class T>
    inline void ParseUBJSONArray(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, char ty)
    {
      if(ty != 0 && ty != UBJSONTuple::TYPE_ARRAY) // Sanity check
        throw std::runtime_error("Expecting a type other than array in the array parsing function!");
      if(!ty && (s.get() != UBJSONTuple::TYPE_ARRAY))
        throw std::runtime_error("Expected array, found invalid character");
      int64_t num = 0;
      char type = 0;
      int64_t count = ParseUBJSONTypeCount(s, type);
      if(count <= 0 || !type || !ParseUBJSONInternal<T, false>::DoBulkRead(e, obj, s, count, type)) // attempt mass read in if we have both a type and a count
      {
        while(!!s && (count > 0 || count < 0) && (count > 0 || s.peek() != UBJSONTuple::TYPE_ARRAY_END) && s.peek() != -1)
        {
          ParseUBJSONInternal<T, false>::DoAddCall(e, obj, s, num, type);
          --count;
        }
        if(!!s && count < 0 && s.peek() == UBJSONTuple::TYPE_ARRAY_END) s.get(); // If we were looking for the ARRAY_END symbol, eat it.
      }
    }

    template<class T, int I, bool B>
    struct ParseUBJSONInternal<T[I], B>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, int64_t count, char ty)
      {
        if((ty != UBJSONTuple::TYPE_CHAR && ty != UBJSONTuple::TYPE_UINT8 && ty != UBJSONTuple::TYPE_INT8) || count != (I * sizeof(T)))
          return false;

        s.read((char*)obj, count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, int64_t& n, char ty) { if(n < I) ParseUBJSONBase<T>(e, obj[n++], s, ty); }
      static void F(Serializer<UBJSONEngine>& e, T(&obj)[I], std::istream& s, char ty) { ParseUBJSONArray<T[I]>(e, obj, s, ty); }
    };

    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct ParseUBJSONInternal<DynArray<T, CType, ArrayType, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t count, char ty)
      {
        if((ty != UBJSONTuple::TYPE_CHAR && ty != UBJSONTuple::TYPE_UINT8 && ty != UBJSONTuple::TYPE_INT8) || count % sizeof(T) != 0)
          return false;

        obj.SetLength(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        s.read((char*)(T*)obj, count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t& n, char ty) { obj.AddConstruct(); ParseUBJSONBase<T>(e, obj.Back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, DynArray<T, CType, ArrayType, Alloc>& obj, std::istream& s, char ty)
      {
        obj.Clear();
        ParseUBJSONArray<DynArray<T, CType, ArrayType, Alloc>>(e, obj, s, ty);
      }
    };

    template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct ParseUBJSONInternal<DynArray<bool, CType, ArrayType, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t count, char ty) { return false; }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, int64_t& n, char ty) { bool b; ParseUBJSONBase<bool>(e, b, s, ty); obj.Add(b); }
      static void F(Serializer<UBJSONEngine>& e, DynArray<bool, CType, ArrayType, Alloc>& obj, std::istream& s, char ty)
      {
        obj.Clear();
        ParseUBJSONArray<DynArray<bool, CType, ArrayType, Alloc>>(e, obj, s, ty);
      }
    };

    template<class T, typename Alloc>
    struct ParseUBJSONInternal<std::vector<T, Alloc>, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int64_t count, char ty)
      {
        if((ty != UBJSONTuple::TYPE_CHAR && ty != UBJSONTuple::TYPE_UINT8 && ty != UBJSONTuple::TYPE_INT8) || count % sizeof(T) != 0)
          return false;

        obj.resize(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
        s.read((char*)obj.data(), count);
        return true;
      }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, int64_t& n, char ty) { obj.push_back(T()); ParseUBJSONBase<T>(e, obj.back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, std::vector<T, Alloc>& obj, std::istream& s, char ty)
      {
        obj.clear();
        ParseUBJSONArray<std::vector<T, Alloc>>(e, obj, s, ty);
      }
    };

    template<>
    struct ParseUBJSONInternal<UBJSONValue::UBJSONArray, false>
    {
      static inline bool DoBulkRead(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, int64_t count, char ty) { return false; }
      static inline void DoAddCall(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, int64_t& n, char ty) { obj.SetLength(obj.Length() + 1); ParseUBJSONBase<UBJSONValue>(e, obj.Back(), s, ty); }
      static void F(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONArray& obj, std::istream& s, char ty) { ParseUBJSONArray<UBJSONValue::UBJSONArray>(e, obj, s, ty); }
    };

    template<>
    struct ParseUBJSONInternal<UBJSONValue::UBJSONObject, false>
    {
      static void F(Serializer<UBJSONEngine>& e, UBJSONValue::UBJSONObject& obj, std::istream& s, char ty)
      {
        ty = ty;
        ParseUBJSONObj(e, ty, [&obj, &s](Serializer<UBJSONEngine>& e, const char* id) {
          std::pair<Str, UBJSONValue> pair;
          pair.first = id;
          ParseUBJSONBase<UBJSONValue>(e, pair.second, s, e.engine.type);
          obj.Add(pair);
        });
      }
    };
  }

  template<class T>
  inline void ParseUBJSONBase(Serializer<UBJSONEngine>& e, T& obj, std::istream& s, char type) { internal::ParseUBJSONInternal<T, std::is_arithmetic<T>::value>::F(e, obj, s, type); }

  template<>
  inline void ParseUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, std::string& obj, std::istream& s, char type)
  {
    UBJSONTuple tuple;
    internal::ParseUBJSONValue(tuple, s, type);

    switch(tuple.type) // we will shove just about anything we possibly can into a string
    {
    case UBJSONTuple::TYPE_NULL: break;
    case UBJSONTuple::TYPE_CHAR: obj = tuple.Int8;
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

  template<>
  inline void ParseUBJSONBase<Str>(Serializer<UBJSONEngine>& e, Str& obj, std::istream& s, char type) { ParseUBJSONBase<std::string>(e, obj, s, type); }

  template<>
  inline void ParseUBJSONBase<UBJSONValue>(Serializer<UBJSONEngine>& e, UBJSONValue& obj, std::istream& s, char type)
  {
    UBJSONTuple tuple;
    internal::ParseUBJSONValue(tuple, s, type);

    switch(tuple.type)
    {
    case UBJSONTuple::TYPE_NO_OP: break;
    case UBJSONTuple::TYPE_NULL: break;
    case UBJSONTuple::TYPE_TRUE: obj = true; break;
    case UBJSONTuple::TYPE_FALSE: obj = false; break;
    case UBJSONTuple::TYPE_ARRAY:
      obj = UBJSONValue::UBJSONArray();
      internal::ParseUBJSONInternal<UBJSONValue::UBJSONArray, false>::F(e, obj.get<UBJSONValue::UBJSONArray>(), s, UBJSONTuple::TYPE_ARRAY);
      break;
    case UBJSONTuple::TYPE_OBJECT:
      obj = UBJSONValue::UBJSONObject();
      internal::ParseUBJSONInternal<UBJSONValue::UBJSONObject, false>::F(e, obj.get<UBJSONValue::UBJSONObject>(), s, UBJSONTuple::TYPE_OBJECT);
      break;
    case UBJSONTuple::TYPE_CHAR:
    case UBJSONTuple::TYPE_INT8: obj = tuple.Int8; break;
    case UBJSONTuple::TYPE_UINT8: obj = tuple.UInt8; break;
    case UBJSONTuple::TYPE_INT16: obj = tuple.Int16; break;
    case UBJSONTuple::TYPE_INT32: obj = tuple.Int32; break;
    case UBJSONTuple::TYPE_INT64: obj = tuple.Int64; break;
    case UBJSONTuple::TYPE_FLOAT: obj = tuple.Float; break;
    case UBJSONTuple::TYPE_DOUBLE: obj = tuple.Double; break;
    case UBJSONTuple::TYPE_BIGNUM:
    case UBJSONTuple::TYPE_STRING:
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
    void WriteUBJSONObject(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char ty)
    {
      if(ty != 0 && ty != UBJSONTuple::TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object serializing function!");
      if(!ty)
        s.put(UBJSONTuple::TYPE_OBJECT);

      const_cast<T&>(obj).template Serialize<UBJSONEngine>(e);
      if(e.engine.endobject)
      //if(obj.SerializeUBJSON(s))
        s.put(UBJSONTuple::TYPE_OBJECT_END);
    }

    template<class T, bool B>
    struct WriteUBJSONInternal
    {
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char ty) { WriteUBJSONObject<T>(e, obj, s, ty); }
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
    bool WriteUBJSONSpecificInt(const FROM& obj, std::ostream& s, UBJSONTuple::TYPE type)
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
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char type)
      {
        switch(type)
        {
        case 0:
          if(!WriteUBJSONSpecificInt<uint8_t, T>(obj, s, UBJSONTuple::TYPE_UINT8) &&
            !WriteUBJSONSpecificInt<char, T>(obj, s, UBJSONTuple::TYPE_INT8) &&
            !WriteUBJSONSpecificInt<short, T>(obj, s, UBJSONTuple::TYPE_INT16) &&
            !WriteUBJSONSpecificInt<int32_t, T>(obj, s, UBJSONTuple::TYPE_INT32) &&
            !WriteUBJSONSpecificInt<int64_t, T>(obj, s, UBJSONTuple::TYPE_INT64))
          {
            s.put(UBJSONTuple::TYPE_BIGNUM);
            throw std::runtime_error("Unknown large integer type!");
          }
          break;
        case UBJSONTuple::TYPE_CHAR:
        case UBJSONTuple::TYPE_INT8: WriteUBJSONInteger<char>((char)obj, s); break;
        case UBJSONTuple::TYPE_UINT8: WriteUBJSONInteger<uint8_t>((uint8_t)obj, s); break;
        case UBJSONTuple::TYPE_INT16: WriteUBJSONInteger<short>((short)obj, s); break;
        case UBJSONTuple::TYPE_INT32: WriteUBJSONInteger<int32_t>((int32_t)obj, s); break;
        case UBJSONTuple::TYPE_INT64: WriteUBJSONInteger<int64_t>((int64_t)obj, s); break;
        case UBJSONTuple::TYPE_BIGNUM: break; // we can't deal with bignum
        }
      }
    };

    template<class T> struct WriteUBJSONType { static const char t = UBJSONTuple::TYPE_OBJECT; };
    template<> struct WriteUBJSONType<uint8_t> { static const char t = UBJSONTuple::TYPE_UINT8; };
    template<> struct WriteUBJSONType<char> { static const char t = UBJSONTuple::TYPE_INT8; };
    template<> struct WriteUBJSONType<bool> { static const char t = 0; };
    template<> struct WriteUBJSONType<short> { static const char t = UBJSONTuple::TYPE_INT16; };
    template<> struct WriteUBJSONType<int32_t> { static const char t = UBJSONTuple::TYPE_INT32; };
    template<> struct WriteUBJSONType<int64_t> { static const char t = UBJSONTuple::TYPE_INT64; };
    template<> struct WriteUBJSONType<float> { static const char t = UBJSONTuple::TYPE_FLOAT; };
    template<> struct WriteUBJSONType<double> { static const char t = UBJSONTuple::TYPE_DOUBLE; };
    template<> struct WriteUBJSONType<const char*> { static const char t = UBJSONTuple::TYPE_STRING; };
    template<> struct WriteUBJSONType<std::string> { static const char t = UBJSONTuple::TYPE_STRING; };
    template<> struct WriteUBJSONType<Str> { static const char t = UBJSONTuple::TYPE_STRING; };
    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONType<DynArray<T, CType, ArrayType, Alloc>> { static const char t = UBJSONTuple::TYPE_ARRAY; };
    template<class T, typename Alloc>
    struct WriteUBJSONType<std::vector<T, Alloc>> { static const char t = UBJSONTuple::TYPE_ARRAY; };
    template<class T, int I> struct WriteUBJSONType<T[I]> { static const char t = UBJSONTuple::TYPE_ARRAY; };
    template<> struct WriteUBJSONType<UBJSONValue::UBJSONArray> { static const char t = UBJSONTuple::TYPE_ARRAY; };
    template<> struct WriteUBJSONType<UBJSONValue::UBJSONObject> { static const char t = UBJSONTuple::TYPE_OBJECT; };

    template<class T, typename Arg, typename... Args>
    struct __UBJSONVariantType
    {
      static inline const char F(const T& v)
      {
        if(v.template is<Arg>())
          return WriteUBJSONType<Arg>::t;
        return __UBJSONVariantType<T, Args...>::F(v);
      }
    };
    template<class T, typename Arg>
    struct __UBJSONVariantType<T, Arg>
    {
      static inline const char F(const T& v)
      {
        if(v.template is<Arg>())
          return WriteUBJSONType<Arg>::t;
        return 0;
      }
    };

    template<class E, class T>
    inline void WriteUBJSONArray(Serializer<UBJSONEngine>& e, T obj, const char* data, size_t size, std::ostream& s, char type)
    {
      if(!size) // It's more efficient to skip type/length information for zero length arrays.
      {
        s.put(UBJSONTuple::TYPE_ARRAY_END);
        return;
      }
      if(type)
      {
        s.put(UBJSONTuple::TYPE_TYPE);
        s.put(type);
      }
      s.put(UBJSONTuple::TYPE_COUNT);
      WriteUBJSONBase<size_t>(e, size, s);
      if(data != 0 && (type == UBJSONTuple::TYPE_CHAR || type == UBJSONTuple::TYPE_UINT8 || type == UBJSONTuple::TYPE_INT8))
        s.write(data, size * sizeof(E)); //sizeof(E) should be 1 here but we multiply it anyway
      else
      {
        for(uint32_t i = 0; i < size; ++i)
          WriteUBJSONBase<E>(e, obj[i], s, type);
      }
    }

    template<class T, int I>
    struct WriteUBJSONInternal<T[I], false>
    {
      static void F(Serializer<UBJSONEngine>& e, const T(&obj)[I], std::ostream& s, char ty)
      {
        if(!ty) 
          s.put(UBJSONTuple::TYPE_ARRAY);
        WriteUBJSONArray<T, const T*>(e, (const T*)obj, (const char*)obj, I, s, WriteUBJSONType<T>::t);
      }
    };

    template<class T, typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONInternal<DynArray<T, CType, ArrayType, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const DynArray<T, CType, ArrayType, Alloc>& obj, std::ostream& s, char ty)
      {
        if(!ty) 
          s.put(UBJSONTuple::TYPE_ARRAY);
        WriteUBJSONArray<T>(e, obj, (const char*)obj.begin(), obj.Length(), s, WriteUBJSONType<T>::t);
      }
    };

    template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
    struct WriteUBJSONInternal<DynArray<bool, CType, ArrayType, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const DynArray<bool, CType, ArrayType, Alloc>& obj, std::ostream& s, char ty)
      {
        if(!ty) 
          s.put(UBJSONTuple::TYPE_ARRAY);
        WriteUBJSONArray<bool>(e, obj, 0, obj.Length(), s, 0);
      }
    };

    template<class T, typename Alloc>
    struct WriteUBJSONInternal<std::vector<T, Alloc>, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const std::vector<T, Alloc>& obj, std::ostream& s, char ty)
      {
        if(!ty)
          s.put(UBJSONTuple::TYPE_ARRAY);
        WriteUBJSONArray<T>(e, obj, (const char*)obj.data(), obj.size(), s, WriteUBJSONType<T>::t);
      }
    };

    template<typename T, typename Arg, typename... Args>
    struct WriteUBJSONVariantInternal
    {
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char ty)
      {
        if(obj.template is<Arg>())
          WriteUBJSONBase<Arg>(e, obj.template get<Arg>(), s, ty);
        else
          WriteUBJSONVariantInternal<T, Args...>::F(obj, s, ty);
      }
    };

    template<typename T, typename Arg>
    struct WriteUBJSONVariantInternal<T, Arg>
    {
      static void F(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char ty)
      {
        if(obj.template is<Arg>())
          WriteUBJSONBase<Arg>(e, obj.template get<Arg>(), s, ty);
        else
          assert(obj.tag() != -1);
      }
    };

    template<typename... Args>
    void WriteUBJSONVariant(Serializer<UBJSONEngine>& e, const Variant<Args...>& obj, std::ostream& s, char ty) { WriteUBJSONVariantInternal<Variant<Args...>, Args...>::F(e, obj, s, ty); }

    template<>
    struct WriteUBJSONInternal<UBJSONValue, false>
    {
      static void F(Serializer<UBJSONEngine>& e, const UBJSONValue& obj, std::ostream& s, char ty)
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

          char type = (v.Length() > 0) ? GetUBJSONVariantType(v[0]) : 0;
          for(size_t i = 1; i < v.Length(); ++i)
            if(type != GetUBJSONVariantType(v[i]))
              type = 0;

          assert(!ty || ty == UBJSONTuple::TYPE_ARRAY);
          if(!ty) 
            s.put(UBJSONTuple::TYPE_ARRAY);

          WriteUBJSONArray<UBJSONValue, const UBJSONValue*>(e, (const UBJSONValue*)v, 0, v.Length(), s, type);
          break;
        }
        case UBJSONValue::Type<UBJSONValue::UBJSONObject>::value:
          WriteUBJSONObject<UBJSONValue>(e, obj, s, ty);
          break;
        }
      }
    };

    inline void WriteUBJSONString(Serializer<UBJSONEngine>& e, const char* str, size_t len, std::ostream& s, char type)
    {
      if(!type)
        s.put(UBJSONTuple::TYPE_STRING);

      WriteUBJSONBase<size_t>(e, len, s);
      s.write(str, len);
    }

    inline void WriteUBJSONId(Serializer<UBJSONEngine>& e, const char* id, std::ostream& s)
    {
      if(id) 
        WriteUBJSONString(e, id, strlen(id), s, UBJSONTuple::TYPE_STRING);
    }
  }

  template<typename... Args>
  inline const char GetUBJSONVariantType(const Variant<Args...>& v) { return internal::__UBJSONVariantType<Variant<Args...>, Args...>::F(v); }

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const char* id, const T& obj, std::ostream& s, char type) { internal::WriteUBJSONId(e, id, s); internal::WriteUBJSONInternal<T, std::is_integral<T>::value>::F(e, obj, s, type); }

  template<class T>
  inline void WriteUBJSONBase(Serializer<UBJSONEngine>& e, const T& obj, std::ostream& s, char type) { WriteUBJSONBase<T>(e, 0, obj, s, type); }

  template<>
  inline void WriteUBJSONBase<bool>(Serializer<UBJSONEngine>& e, const char* id, const bool& obj, std::ostream& s, char type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(obj ? UBJSONTuple::TYPE_TRUE : UBJSONTuple::TYPE_FALSE); }

  template<>
  inline void WriteUBJSONBase<float>(Serializer<UBJSONEngine>& e, const char* id, const float& obj, std::ostream& s, char type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(UBJSONTuple::TYPE_FLOAT); internal::WriteUBJSONInteger<float>(obj, s); }

  template<>
  inline void WriteUBJSONBase<double>(Serializer<UBJSONEngine>& e, const char* id, const double& obj, std::ostream& s, char type) { internal::WriteUBJSONId(e, id, s); if(!type) s.put(UBJSONTuple::TYPE_DOUBLE); internal::WriteUBJSONInteger<double>(obj, s); }

  template<>
  inline void WriteUBJSONBase<std::string>(Serializer<UBJSONEngine>& e, const char* id, const std::string& obj, std::ostream& s, char type) { internal::WriteUBJSONId(e, id, s); internal::WriteUBJSONString(e, obj.c_str(), obj.length(), s, type); }

  template<>
  inline void WriteUBJSONBase<Str>(Serializer<UBJSONEngine>& e, const char* id, const Str& obj, std::ostream& s, char type) { WriteUBJSONBase<std::string>(e, id, obj, s, type); }

  template<class T>
  inline void WriteUBJSON(const T& obj, std::ostream& s) { Serializer<UBJSONEngine> e; e.Serialize(obj, s); }

  template<typename T>
  void UBJSONEngine::Serialize(Serializer<UBJSONEngine>& e, const T& t, const char* id) { e.engine.endobject = true; WriteUBJSONBase<T>(e, id, t, *e.out, 0); }
  template<typename T>
  void UBJSONEngine::Parse(Serializer<UBJSONEngine>& e, T& t, const char* id) { ParseUBJSONBase<T>(e, t, *e.in, e.engine.type); }
  template<typename... Args>
  void UBJSONEngine::ParseMany(Serializer<UBJSONEngine>& e, const Trie<uint16_t>& t, std::tuple<Args...>& args)
  {
    internal::ParseUBJSONObj(e, e.engine.type, [&t, &args](Serializer<UBJSONEngine>& e, const char* id) {
      Serializer<UBJSONEngine>::_findparse(e, id, t, args);
    });
  }
}

#endif