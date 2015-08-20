// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_UBJSON_H__BSS__
#define __C_UBJSON_H__BSS__

#include "cDynArray.h"
#include "cStr.h"
#include "bss_util.h"
#include <sstream>
#include <istream>
#include <ostream>
#include <limits>

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
    s.read((char*)&v, sizeof(T));
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
    s.write((char*)&v, sizeof(T));
  }

  static inline __int64 ParseUBJSONLength(std::istream& s)
  {
    __int64 ret = -1;
    while(s)
    {
      switch(s.get())
      {
      case UBJSONTuple::TYPE_CHAR: // you aren't supposed to do this but we'll deal with it anyway
      case UBJSONTuple::TYPE_INT8: ret = ParseUBJSONInteger<char>(s); break;
      case UBJSONTuple::TYPE_UINT8: ret = ParseUBJSONInteger<unsigned char>(s); break;
      case UBJSONTuple::TYPE_INT16: ret = ParseUBJSONInteger<short>(s); break;
      case UBJSONTuple::TYPE_INT32: ret = ParseUBJSONInteger<__int32>(s); break;
      case UBJSONTuple::TYPE_INT64: ret = ParseUBJSONInteger<__int64>(s); break;
      case UBJSONTuple::TYPE_NO_OP: continue; // try again
      default:
        throw std::runtime_error("Invalid length type.");
      }
      break;
    }
    if(ret < 0)
      throw std::runtime_error("Negative length is not allowed.");
    return ret;
  }

  static inline void ParseUBJSONValue(UBJSONTuple& tuple, std::istream& s, char type)
  {
    do
    {
      tuple.type = (UBJSONTuple::TYPE)((type != 0) ? type : s.get());
    } while(tuple.type == UBJSONTuple::TYPE_NO_OP);
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
    case UBJSONTuple::TYPE_UINT8: tuple.UInt8 = ParseUBJSONInteger<unsigned char>(s); break;
    case UBJSONTuple::TYPE_INT16: tuple.Int16 = ParseUBJSONInteger<short>(s); break;
    case UBJSONTuple::TYPE_INT32: tuple.Int32 = ParseUBJSONInteger<__int32>(s); break;
    case UBJSONTuple::TYPE_INT64: tuple.Int64 = ParseUBJSONInteger<__int64>(s); break;
    case UBJSONTuple::TYPE_FLOAT: tuple.Float = ParseUBJSONInteger<float>(s); break;
    case UBJSONTuple::TYPE_DOUBLE: tuple.Double = ParseUBJSONInteger<double>(s); break;
    case UBJSONTuple::TYPE_BIGNUM:
    case UBJSONTuple::TYPE_STRING:
      tuple.length = ParseUBJSONLength(s);
      tuple.String = new char[(size_t)tuple.length+1];
      s.read(tuple.String, tuple.length);
      tuple.String[tuple.length] = 0;
      break;
    }
  }
  
  static inline __int64 ParseUBJSONTypeCount(std::istream& s, char& type)
  {
    type = 0;
    if(!!s && s.peek() == UBJSONTuple::TYPE_TYPE)
    { 
      s.get();
      type = s.get();
    }
    if(!!s && s.peek() == UBJSONTuple::TYPE_COUNT)
    { 
      s.get();
      return ParseUBJSONLength(s);
    } else if(type != 0)
      throw std::runtime_error("A type was specified, but no count was given. A count MUST follow a type!");
    return -1;
  }

  DEFINE_MEMBER_CHECKER(EvalUBJSON);

  template<class T, bool B>
  struct ParseUBJSONInternal
  {
    static_assert(HAS_MEMBER(T, EvalUBJSON), "T must implement void EvalUBJSON(const char*, std::istream&, char)");
    static void F(T& obj, std::istream& s, char ty)
    {
      if(ty != 0 && ty != UBJSONTuple::TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object parsing function!");
      if(!ty && (s.get() != UBJSONTuple::TYPE_OBJECT))
        throw std::runtime_error("Expected object, found invalid character");
      cStr buf;
      char type = 0;
      __int64 count = ParseUBJSONTypeCount(s, type);
      while(!!s && (count<0 || count>0) && (count>0 || s.peek() != UBJSONTuple::TYPE_OBJECT_END) && s.peek() != -1)
      {
        __int64 length = ParseUBJSONLength(s);
        --count;
        buf.reserve(length+1);
        s.read(buf, length);
        buf[length] = 0;
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
  static inline __int64 ParseUBJSONArray(T& obj, std::istream& s, char ty)
  {
    if(ty != 0 && ty != UBJSONTuple::TYPE_ARRAY) // Sanity check
      throw std::runtime_error("Expecting a type other than array in the array parsing function!");
    if(!ty && (s.get() != UBJSONTuple::TYPE_ARRAY))
      throw std::runtime_error("Expected array, found invalid character");
    __int64 num = 0;
    char type = 0;
    __int64 count = ParseUBJSONTypeCount(s, type);
    if(count <= 0 || !type || !ParseUBJSONInternal<T, false>::DoBulkRead(obj, std::istream& s, count, type)) // attempt mass read in if we have both a type and a count
    {
      while(!!s && && (count > 0 || count < 0) && (count>0 || s.peek() != UBJSONTuple::TYPE_ARRAY_END) && s.peek() != -1)
      {
        ParseUBJSONInternal<T, false>::DoAddCall(obj, s, num, type);
        --count;
      }
    }
  }

  template<class T, int I, bool B>
  struct ParseUBJSONInternal<T[I], B>
  {
    static inline bool DoBulkRead(T(&obj)[I], std::istream& s, __int64 count, char ty) { return false; }
    static inline void DoAddCall(T(&obj)[I], std::istream& s, int& n, char ty) { if(n<I) ParseUBJSON<T>(obj[n++], s, ty); }
    static void F(T(&obj)[I], std::istream& s, char ty) { ParseUBJSONArray<T[I]>(obj, s, ty); }
  };

  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseUBJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static inline bool DoBulkRead(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s, __int64 count, char ty)
    { 
      if((type != UBJSONTuple::TYPE_CHAR && type != UBJSONTuple::TYPE_UINT8 && type != UBJSONTuple::TYPE_INT8) || count%sizeof(T) != 0)
        return false;

      obj.SetLength(count/sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
      s.read((char*)(T*)obj, count);
      return true;
    }
    static inline void DoAddCall(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s, int& n, char ty) { obj.Add(T()); ParseUBJSON<T>(obj.Back(), s, ty); }
    static void F(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s, char ty)
    {
      obj.Clear();
      ParseUBJSONArray<cDynArray<T, SizeType, ArrayType, Alloc>>(obj, s, ty);
    }
  };

  template<class T, typename Alloc>
  struct ParseUBJSONInternal<std::vector<T, Alloc>, false>
  {
    static inline bool DoBulkRead(std::vector<T, Alloc>& obj, std::istream& s, __int64 count, char ty)
    {
      if((type != UBJSONTuple::TYPE_CHAR && type != UBJSONTuple::TYPE_UINT8 && type != UBJSONTuple::TYPE_INT8) || count%sizeof(T) != 0)
        return false;

      obj.resize(count / sizeof(T)); // If the type is 1 byte and the count is divisible by the array element size, we can do an optimized read
      s.read((char*)(T*)obj, count);
      return true;
    }
    static inline void DoAddCall(std::vector<T, Alloc>& obj, std::istream& s, int& n, char ty) { obj.push_back(T()); ParseUBJSON<T>(obj.back(), s, ty); }
    static void F(std::vector<T, Alloc>& obj, std::istream& s, char ty)
    {
      obj.clear();
      ParseUBJSONArray<std::vector<T, Alloc>>(obj, s, ty);
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
  inline void ParseUBJSON<cStr>(cStr& obj, std::istream& s, char type){ ParseUBJSON<std::string>(obj, s, type); }

  DEFINE_MEMBER_CHECKER(SerializeUBJSON);

  template<class T, bool B>
  struct WriteUBJSONInternal
  {
    static_assert(HAS_MEMBER(T, SerializeUBJSON), "T must implement void SerializeUBJSON(std::ostream&) const");
    static void F(const T& obj, std::ostream& s, char ty)
    {
      if(ty != 0 && ty != UBJSONTuple::TYPE_OBJECT) // Sanity check
        throw std::runtime_error("Expecting a type other than object in the object serializing function!");
      if(!ty)
        s.put(UBJSONTuple::TYPE_OBJECT);
      obj.SerializeUBJSON(s);
      s.put(UBJSONTuple::TYPE_OBJECT_END);
    }
  };

  template<class T, typename FROM>
  bool WriteUBJSONSpecificInt(const FROM& obj, std::ostream& s, UBJSONTuple::TYPE type)
  {
    if(obj >= (FROM)std::numeric_limits<T>::min() && obj <= (FROM)std::numeric_limits<T>::max())
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
    static void F(const T& obj, std::ostream& s, char type)
    {
      switch(type)
      {
      case 0:
        if(!WriteUBJSONSpecificInt<unsigned char, T>(obj, s, UBJSONTuple::TYPE_UINT8) &&
          !WriteUBJSONSpecificInt<char, T>(obj, s, UBJSONTuple::TYPE_INT8) &&
          !WriteUBJSONSpecificInt<short, T>(obj, s, UBJSONTuple::TYPE_INT16) &&
          !WriteUBJSONSpecificInt<__int32, T>(obj, s, UBJSONTuple::TYPE_INT32) &&
          !WriteUBJSONSpecificInt<__int64, T>(obj, s, UBJSONTuple::TYPE_INT64))
        {
          s.put(UBJSONTuple::TYPE_BIGNUM);
          throw std::runtime_error("Unknown large integer type!");
        }
        break;
      case UBJSONTuple::TYPE_CHAR:
      case UBJSONTuple::TYPE_INT8: WriteUBJSONInteger<char>((char)obj, s); break;
      case UBJSONTuple::TYPE_UINT8: WriteUBJSONInteger<unsigned char>((unsigned char)obj, s); break;
      case UBJSONTuple::TYPE_INT16: WriteUBJSONInteger<short>((short)obj, s); break;
      case UBJSONTuple::TYPE_INT32: WriteUBJSONInteger<__int32>((__int32)obj, s); break;
      case UBJSONTuple::TYPE_INT64: WriteUBJSONInteger<__int64>((__int64)obj, s); break;
      case UBJSONTuple::TYPE_BIGNUM: break; // we can't deal with bignum
      }
    }
  };

  template<class T> struct WriteUBJSONType { static const char t = UBJSONTuple::TYPE_OBJECT; };
  template<> struct WriteUBJSONType<unsigned char> { static const char t = UBJSONTuple::TYPE_UINT8; };
  template<> struct WriteUBJSONType<char> { static const char t = UBJSONTuple::TYPE_INT8; };
  template<> struct WriteUBJSONType<bool> { static const char t = 0; };
  template<> struct WriteUBJSONType<short> { static const char t = UBJSONTuple::TYPE_INT16; };
  template<> struct WriteUBJSONType<__int32> { static const char t = UBJSONTuple::TYPE_INT32; };
  template<> struct WriteUBJSONType<__int64> { static const char t = UBJSONTuple::TYPE_INT64; };
  template<> struct WriteUBJSONType<float> { static const char t = UBJSONTuple::TYPE_FLOAT; };
  template<> struct WriteUBJSONType<double> { static const char t = UBJSONTuple::TYPE_DOUBLE; };
  template<> struct WriteUBJSONType<const char*> { static const char t = UBJSONTuple::TYPE_STRING; };
  template<> struct WriteUBJSONType<std::string> { static const char t = UBJSONTuple::TYPE_STRING; };
  template<> struct WriteUBJSONType<cStr> { static const char t = UBJSONTuple::TYPE_STRING; };
  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteUBJSONType<cDynArray<T, SizeType, ArrayType, Alloc>> { static const char t = UBJSONTuple::TYPE_ARRAY; };
  template<class T, typename Alloc>
  struct WriteUBJSONType<std::vector<T, Alloc>> { static const char t = UBJSONTuple::TYPE_ARRAY; };
  template<class T, int I> struct WriteUBJSONType<T[I]> { static const char t = UBJSONTuple::TYPE_ARRAY; };

  template<class T>
  static inline __int64 WriteUBJSONArray(const T* obj, size_t size, std::ostream& s, char type)
  {
    if(type)
    {
      s.put(UBJSONTuple::TYPE_TYPE);
      s.put(type);
    }
    s.put(UBJSONTuple::TYPE_COUNT);
    WriteUBJSON<size_t>(size, s);
    if(type == UBJSONTuple::TYPE_CHAR || type == UBJSONTuple::TYPE_UINT8 || type == UBJSONTuple::TYPE_INT8)
      s.write((char*)obj, size*sizeof(T)); //sizeof(T) should be 1 here but we multiply it anyway
    else
    {
      for(unsigned int i = 0; i < size; ++i)
        WriteUBJSON<T>(obj[i], s, type);
    }
  }

  template<class T, int I>
  struct WriteUBJSONInternal<T[I], false>
  {
    static void F(const T(&obj)[I], std::ostream& s, char ty)
    {
      if(!ty) s.put(UBJSONTuple::TYPE_ARRAY);
      WriteUBJSONArray<T>(obj, I, s, WriteUBJSONType<T>::t);
    }
  };

  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteUBJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static void F(const cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::ostream& s, char ty)
    {
      if(!ty) s.put(UBJSONTuple::TYPE_ARRAY);
      WriteUBJSONArray<T>(obj, obj.Length(), s, WriteUBJSONType<T>::t);
    }
  };

  template<class T, typename Alloc>
  struct WriteUBJSONInternal<std::vector<T, Alloc>, false>
  {
    static void F(const std::vector<T, Alloc>& obj, std::ostream& s, char ty)
    {
      if(!ty) s.put(UBJSONTuple::TYPE_ARRAY);
      WriteUBJSONArray<T>(obj, obj.size(), s, WriteUBJSONType<T>::t);
    }
  };

  template<class T>
  inline void WriteUBJSON(const char* id, const T& obj, std::ostream& s, char type){ WriteUBJSONId(id, s); WriteUBJSONInternal<T, std::is_integral<T>::value>::F(obj, s, type); }

  template<class T>
  inline void WriteUBJSON(const T& obj, std::ostream& s, char type = 0) { WriteUBJSON<T>(0, obj, s, type); }

  inline static void WriteUBJSONString(const char* str, size_t len, std::ostream& s, char type)
  {
    if(!type)
      s.put(UBJSONTuple::TYPE_STRING);
    WriteUBJSON<size_t>(len, s);
    s.write(str, len);
  }
  
  static inline void WriteUBJSONId(const char* id, std::ostream& s)
  {
    if(id) WriteUBJSONString(id, strlen(id), s, UBJSONTuple::TYPE_STRING);
  }
  
  template<>
  inline void WriteUBJSON<bool>(const char* id, const bool& obj, std::ostream& s, char type) { WriteUBJSONId(id, s); if(!type) s.put(obj ? UBJSONTuple::TYPE_TRUE : UBJSONTuple::TYPE_FALSE); }

  template<>
  inline void WriteUBJSON<float>(const char* id, const float& obj, std::ostream& s, char type) { WriteUBJSONId(id, s); if(!type) s.put(UBJSONTuple::TYPE_FLOAT); WriteUBJSONInteger<float>(obj, s); }

  template<>
  inline void WriteUBJSON<double>(const char* id, const double& obj, std::ostream& s, char type) { WriteUBJSONId(id, s); if(!type) s.put(UBJSONTuple::TYPE_DOUBLE); WriteUBJSONInteger<double>(obj, s); }

  template<>
  inline void WriteUBJSON<std::string>(const char* id, const std::string& obj, std::ostream& s, char type) { WriteUBJSONId(id, s); WriteUBJSONString(obj.c_str(), obj.length(), s, type); }

  template<>
  inline void WriteUBJSON<cStr>(const char* id, const cStr& obj, std::ostream& s, char type) { WriteUBJSON<std::string>(id, obj, s, type); }
}

#endif