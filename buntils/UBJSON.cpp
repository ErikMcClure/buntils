// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "buntils/buntils.h"
#include "buntils/UBJSON.h"
#include <sstream>
#include <fstream>
#include <string>

using namespace bun;

UBJSONTuple::UBJSONTuple(const UBJSONTuple& copy) : Type(copy.Type), Length(copy.Length)
{
  switch(Type)
  {
  case TYPE_STRING:
  case TYPE_BIGNUM:
    assert(copy.Length >= 0);
    String = new char[Length + 1];
    MEMCPY(String, Length + 1, copy.String, copy.Length + 1);
    break;
  case TYPE_ARRAY:
    new (&Array) UBJSONArray(copy.Array);
    break;
  case TYPE_OBJECT:
    new (&Object) UBJSONObject(copy.Object);
    break;
  default:
    Int64 = copy.Int64;
    break;
  }
}
UBJSONTuple::UBJSONTuple(UBJSONTuple&& mov) : Type(mov.Type), Length(mov.Length)
{
  switch(Type)
  {
  case TYPE_STRING:
  case TYPE_BIGNUM:
    String = mov.String;
    break;
  case TYPE_ARRAY:
    new (&Array) UBJSONArray(std::move(mov.Array));
    break;
  case TYPE_OBJECT:
    new (&Object) UBJSONObject(std::move(mov.Object));
    break;
  default:
    Int64 = mov.Int64;
    break;
  }
  mov.Type = TYPE_NONE;
}
UBJSONTuple::UBJSONTuple() : Type(TYPE_NONE), Length(-1), Int64(0) {}
UBJSONTuple::UBJSONTuple(TYPE type, int64_t length, const char* s) : Type(type), Length(length), String(const_cast<char*>(s)) {}
UBJSONTuple::~UBJSONTuple()
{
  switch(Type)
  {
  case TYPE_STRING:
  case TYPE_BIGNUM:
    if(String != 0)
      delete[] String;
    break;
  case TYPE_ARRAY:
    Array.~DynArray();
    break;
  case TYPE_OBJECT:
    Object.~DynArray();
    break;
  default:
    break;
  }
}
void UBJSONTuple::Parse(std::istream& s, TYPE ty)
{
  Parse(s, ty, nullptr, nullptr, nullptr);
}

int64_t UBJSONTuple::ParseLength(std::istream& s)
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

int64_t UBJSONTuple::ParseTypeCount(std::istream& s, TYPE& type)
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

void UBJSONTuple::WriteTypeCount(std::ostream& s, TYPE type, int64_t count)
{
  if(type != TYPE_NONE)
  {
    s.put(TYPE_TYPE);
    s.put(type);
  }

  if(count >= 0)
  {
    s.put(TYPE_COUNT);
    WriteLength(s, count);
  }
  else if(type != TYPE_NONE)
    throw std::runtime_error("A type was specified, but no count was given. A count MUST follow a type!");
}

void UBJSONTuple::Write(std::ostream& s, TYPE type) const
{
  if(type == TYPE_NONE)
    s.put(Type);
  else
    assert(Type == type);

  switch(Type)
  {
  default:
    throw std::runtime_error("Unexpected type encountered.");
  case TYPE_NO_OP:
  case TYPE_NULL:
  case TYPE_TRUE:
  case TYPE_FALSE:
    break;
  case TYPE_ARRAY:
    if(!Array.size())
    {
      s.put(TYPE_ARRAY_END);
      break;
    }
    type = Array[0].Type;
    for(auto& i : Array)
      if(type != i.Type)
      {
        type = TYPE_NONE;
        break;
      }
    WriteTypeCount(s, type, Array.size());
    for(auto& i : Array)
      i.Write(s, type);
    break;
  case TYPE_OBJECT:
    if(!Object.size())
    {
      s.put(TYPE_OBJECT_END);
      break;
    }
    type = Object[0].second.Type;
    for(auto&[_, t] : Object)
      if(type != t.Type)
      {
        type = TYPE_NONE;
        break;
      }
    WriteTypeCount(s, type, Object.size());
    for(auto&[str, t] : Object)
    {
      WriteLength(s, str.size());
      s.write(str.data(), str.size());
      t.Write(s, type);
    }
    break;
  case TYPE_CHAR:
  case TYPE_INT8: WriteInteger<int8_t>(Int8, s); break;
  case TYPE_UINT8: WriteInteger<uint8_t>(UInt8, s); break;
  case TYPE_INT16: WriteInteger<int16_t>(Int16, s); break;
  case TYPE_INT32: WriteInteger<int32_t>(Int32, s); break;
  case TYPE_INT64: WriteInteger<int64_t>(Int64, s); break;
  case TYPE_FLOAT: WriteInteger<float>(Float, s); break;
  case TYPE_DOUBLE: WriteInteger<double>(Double, s); break;
  case TYPE_BIGNUM:
  case TYPE_STRING:
    WriteLength(s, Length);
    s.write(String, Length);
    break;
  }
}