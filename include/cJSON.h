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
      } else
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
  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static inline void DoAddCall(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s, int& n)
    {
      obj.Add(T());
      ParseJSON<T>(obj.Back(), s);
    }
    static void F(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.Clear();
      ParseJSONArray<cDynArray<T, SizeType, ArrayType, Alloc>>(obj, s);
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

  template<class T>
  inline void ParseJSON(T& obj, std::istream& s){ ParseJSONInternal<T, std::is_arithmetic<T>::value>::F(obj, s); }
  template<class T>
  inline void ParseJSON(T& obj, const char* s){ std::istringstream ss(s); ParseJSON<T>(obj, ss); }

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
      unsigned __int64 num;
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

  DEFINE_MEMBER_CHECKER(SerializeJSON);

  static bool WriteJSONIsPretty(unsigned int pretty) { return (pretty&(~0x80000000))>0; }
  static void WriteJSONTabs(std::ostream& s, unsigned int pretty)
  { 
    unsigned int count = (pretty&(~0x80000000))-1;
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
    if(pretty&0x80000000) s << ',';
    pretty|=0x80000000;
  }

  static unsigned int WriteJSONPretty(unsigned int pretty) { return (pretty&(~0x80000000)) + ((pretty&(~0x80000000))>0); }

  template<class T, bool B>
  struct WriteJSONInternal
  {
    static_assert(HAS_MEMBER(T, SerializeJSON), "T must implement void SerializeJSON(std::ostream&, unsigned int&) const");
    static void F(const char* id, const T& obj, std::ostream& s, unsigned int& pretty)
    {
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
  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct WriteJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static void F(const char* id, const cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::ostream& s, unsigned int& pretty) { WriteJSONArray<T>(id, obj, obj.Length(), s, pretty); }
  };
  template<class T, typename Alloc>
  struct WriteJSONInternal<std::vector<T, Alloc>, false>
  {
    static void F(const char* id, const std::vector<T, Alloc>& obj, std::ostream& s, unsigned int& pretty) { WriteJSONArray<T>(id, obj, obj.size(), s, pretty); }
  };

  // To enable pretty output, set pretty to 1. The upper two bits are used as flags, so if you need more than 1073741823 levels of indentation... what the hell are you doing?!
  template<class T> 
  void WriteJSON(const char* id, const T& obj, std::ostream& s, unsigned int& pretty) { WriteJSONInternal<T, std::is_arithmetic<T>::value>::F(id, obj, s, pretty); }

  template<>
  void WriteJSON<std::string>(const char* id, const std::string& obj, std::ostream& s, unsigned int& pretty)
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
  void WriteJSON<cStr>(const char* id, const cStr& obj, std::ostream& s, unsigned int& pretty) { WriteJSON<std::string>(id, obj, s, pretty); }

  template<>
  void WriteJSON<bool>(const char* id, const bool& obj, std::ostream& s, unsigned int& pretty)
  {
    WriteJSONComma(s, pretty);
    WriteJSONId(id, s, pretty);
    s << (obj?"true":"false");
  }

  template<class T>
  void WriteJSON(const T& obj, std::ostream& s, unsigned int pretty = 0) { WriteJSON<T>(0, obj, s, pretty); }

  template<class T>
  void WriteJSON(const T& obj, const char* file, unsigned int pretty = 0) { std::ofstream fs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); WriteJSON<T>(0, obj, fs, pretty); }
}

#endif