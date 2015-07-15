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
      OutputUnicode(str, strhex(buf));
      break;
    }
  }

  template<class T, bool B>
  struct ParseJSONInternal
  {
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
        obj.EvalJSON(buf, s);
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
  template<class T, typename SizeType, ARRAY_TYPE ArrayType, typename Alloc>
  struct ParseJSONInternal<cDynArray<T, SizeType, ArrayType, Alloc>, false>
  {
    static void F(cDynArray<T, SizeType, ArrayType, Alloc>& obj, std::istream& s)
    {
      obj.Clear();
      ParseJSONEatWhitespace(s);
      if(!s || s.get() != '[') return;
      ParseJSONEatWhitespace(s);
      while(!!s && s.peek() != ']' && s.peek() != -1)
      {
        obj.Add(T());
        ParseJSON(obj.Back(), s);
        while(!!s && s.peek() != ',' && s.peek() != ']' && s.peek() != -1) s.get(); // eat everything up to a , or ] character
        if(!!s && s.peek() == ',' && s.peek() != -1) s.get(); // Only eat comma if it's there.
        ParseJSONEatWhitespace(s);
      }
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
}

#endif