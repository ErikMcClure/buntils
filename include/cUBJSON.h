// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_JSON_H__BSS__
#define __C_JSON_H__BSS__

#include "cDynArray.h"
#include "cStr.h"
#include "bss_util.h"
#include <sstream>
#include <istream>
#include <ostream>


namespace bss_util {
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

    __int64 length;
    union
    {
      char Int8;
      unsigned char UInt8;
      short Int16;
      __int32 Int32;
      __int64 Int64;
      float Float;
      double Double;
      char* String;
    };
  };

  template<class T>
  inline T ParseUBJSONInteger(std::istream& s)
  {
    T v;
    s.read(&v, sizeof(T));
#ifdef BSS_ENDIAN_LITTLE
    flipendian<T>(&v);
#endif
    return v;
  }

  template<class T>
  inline void WriteUBJSONInteger(T v, std::ostream& s)
  {
#ifdef BSS_ENDIAN_LITTLE
    flipendian<T>(&v);
#endif
    s.write(&v, sizeof(T));
  }

  static inline __int64 ParseUBJSONLength(std::istream& s)
  {
    switch(s.get())
    {
    case UBJSONTuple::TYPE_CHAR: // you aren't supposed to do this but we'll deal with it anyway
    case UBJSONTuple::TYPE_INT8: return ParseUBJSONInteger<char>(s);
    case UBJSONTuple::TYPE_UINT8: return ParseUBJSONInteger<unsigned char>(s);
    case UBJSONTuple::TYPE_INT16: return ParseUBJSONInteger<short>(s);
    case UBJSONTuple::TYPE_INT32: return ParseUBJSONInteger<__int32>(s);
    case UBJSONTuple::TYPE_INT64: return ParseUBJSONInteger<__int64>(s);
    default: return -1;
    }
  }

  static inline void ParseUBJSONValue(UBJSONTuple& tuple, std::istream& s, char type)
  {
    tuple.type = (UBJSONTuple::TYPE)((type != 0)?type:s.get());
    switch(tuple.type)
    {
    default:
    case UBJSONTuple::TYPE_NO_OP:
    case UBJSONTuple::TYPE_NULL:
    case UBJSONTuple::TYPE_TRUE:
    case UBJSONTuple::TYPE_FALSE:
    case UBJSONTuple::TYPE_ARRAY:
    case UBJSONTuple::TYPE_OBJECT:
      return;
    case UBJSONTuple::TYPE_CHAR:
    case UBJSONTuple::TYPE_INT8: tuple.Int8 = ParseUBJSONInteger<char>(s); break;
    case UBJSONTuple::TYPE_UINT8: tuple.UInt8 = ParseUBJSONInteger<unsigned char>(s); break;
    case UBJSONTuple::TYPE_INT16: tuple.Int16 = ParseUBJSONInteger<short>(s); break;
    case UBJSONTuple::TYPE_INT32: tuple.Int32 = ParseUBJSONInteger<__int32>(s); break;
    case UBJSONTuple::TYPE_INT64: tuple.Int64 = ParseUBJSONInteger<__int64>(s); break;
    case UBJSONTuple::TYPE_FLOAT: tuple.Float = ParseUBJSONInteger<float>(s); break;
    case UBJSONTuple::TYPE_DOUBLE: tuple.Double = ParseUBJSONInteger<double>(s); break;
    case UBJSONTuple::TYPE_BIGNUM:
    case UBJSONTuple::TYPE_STRING:
      tuple.length = ParseUBJSONLength(s);
      if(tuple.length < 0) break;
      tuple.String = new char[tuple.length]; // need null terminator?
      s.read(tuple.String, tuple.length);
      break;
    }
  }
  
  template<class T, bool B>
  struct ParseUBJSONInternal
  {
    static void F(T& obj, std::istream& s, char ty)
    {
      if(s.peek() != UBJSONTuple::TYPE_OBJECT && ty != UBJSONTuple::TYPE_OBJECT) return;
      s.get();
      cStr buf;
      __int64 count = -1;
      char type = 0;
      if(s.peek() == UBJSONTuple::TYPE_TYPE) { s.get(); type = s.get(); }
      if(s.peek() == UBJSONTuple::TYPE_COUNT) { s.get(); count = ParseUBJSONLength(s); }
      while(!!s && s.peek() != UBJSONTuple::TYPE_OBJECT_END && s.peek() != -1 && (count<0 || count>0))
      {
        __int64 length = ParseUBJSONLength(s);
        if(length < 0) continue; // we hit a no-op or an illegal value
        --count;
        buf.reserve(length);
        s.read(buf, length);
        // TODO: null terminator?
        obj.EvalUBJSON(buf.c_str(), s, type);
      }
    }
  };

  template<class T>
  struct ParseUBJSONInternal<T, true>
  {
    static void F(T& obj, std::istream& s, char type)
    {
      UBJSONTuple tuple;
      ParseUBJSONValue(tuple, s, type);

      switch(tuple.type) // as long as it's any arithmetic type, attempt to shove it into our target, casting if necessary.
      {
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
      case UBJSONTuple::TYPE_BIGNUM: break; // we can't deal with bignum
      }
    }
  };

  template<class T, int I, bool B>
  struct ParseUBJSONInternal<T[I], B>
  {
    static void F(T(&obj)[I] obj, std::istream& s, char ty)
    {
      if((UBJSONTuple::TYPE)s.peek() != UBJSONTuple::TYPE_ARRAY && ty != UBJSONTuple::TYPE_ARRAY) return;
      s.get();
      __int64 count = -1;
      __int64 num = 0;
      char type = 0;
      if(s.peek() == UBJSONTuple::TYPE_TYPE) { s.get(); type = s.get(); }
      if(s.peek() == UBJSONTuple::TYPE_COUNT) { s.get(); count = ParseUBJSONLength(s); }
      if(count<0) count = I;
      else count = bssmin(count, I);
      while(!!s && s.peek() != UBJSONTuple::TYPE_ARRAY_END && s.peek() != -1 && (count<0 || count<num))
      {
        ParseUBJSON(obj[num++], s, type);
      }
    }
  };

  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseUBJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static void F(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s, char type)
    {
      if((UBJSONTuple::TYPE)s.peek() != UBJSONTuple::TYPE_ARRAY && ty != UBJSONTuple::TYPE_ARRAY) return;
      s.get();
      __int64 count = -1;
      __int64 num = 0;
      char type = 0;
      if(s.peek() == UBJSONTuple::TYPE_TYPE) { s.get(); type = s.get(); }
      if(s.peek() == UBJSONTuple::TYPE_COUNT) { s.get(); count = ParseUBJSONLength(s); }
      if(count>0)
      {
        obj.SetLength(count);
        if(sizeof(T) == 1 && (type == UBJSONTuple::TYPE_CHAR || type == UBJSONTuple::TYPE_UINT8 || type == UBJSONTuple::TYPE_INT8))
        { // If this array is made out of a 1 byte type, and the indicated type is also 1 byte, AND we have a provided count, we can do an optimized read
          s.read((char*)(T*)obj, count);
          return;
        }
      }
      while(!!s && s.peek() != UBJSONTuple::TYPE_ARRAY_END && s.peek() != -1 && (count<0 || num<count))
      {
        obj.SetLength(++num); // This is done instead of Add() so that if we got a count previously, this turns into a no-op
        ParseUBJSON(obj[num-1], s, type);
      }
    }
  };

  template<class T>
  inline void ParseUBJSON(T& obj, std::istream& s, char type){ ParseUBJSONInternal<T, std::is_arithmetic<T>::value>::F(obj, s); }

  template<>
  inline void ParseUBJSON<std::string>(std::string& obj, std::istream& s, char type)
  {
    UBJSONTuple tuple;
    ParseUBJSONValue(tuple, s, type);

    switch(tuple.type) // we will shove just about anything we possibly can into a string
    {
    case UBJSONTuple::TYPE_CHAR: obj = tuple.Int8;
    case UBJSONTuple::TYPE_INT8: obj = std::to_string(tuple.Int8); break;
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
  inline void ParseUBJSON<cStr>(cStr& obj, std::istream& s, char type){ ParseUBJSON<std::string>(obj, s, type); }

  template<class T>
  inline void WriteUBJSON(T& obj, std::istream& s, char type){ ParseUBJSONInternal<T, std::is_arithmetic<T>::value>::F(obj, s); }
}

#endif