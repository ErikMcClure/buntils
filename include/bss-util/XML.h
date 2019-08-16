// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __XML_H__BSS__
#define __XML_H__BSS__

#include "DynArray.h"
#include "Hash.h"
#include "LLBase.h"
#include "Str.h"
#include "Serializer.h"
#include <sstream>

namespace bss {
  // Represents an XML value converted to various forms.
  struct BSS_COMPILER_DLLEXPORT XMLValue
  {
    XMLValue(const XMLValue& copy) : Name(copy.Name), String(copy.String), Float(copy.Float), Integer(copy.Integer) {}
    XMLValue(XMLValue&& mov) : Name(std::move(mov.Name)), String(std::move(mov.String)), Float(std::move(mov.Float)), Integer(std::move(mov.Integer)) {}
    XMLValue() : Float(0.0), Integer(0) {}
    Str Name;
    Str String;
    double Float;
    int64_t Integer;

    inline XMLValue& operator=(const XMLValue& copy) { Name = copy.Name; String = copy.String; Float = copy.Float; Integer = copy.Integer; return *this; }
    inline XMLValue& operator=(XMLValue&& mov) { Name = std::move(mov.Name); String = std::move(mov.String); Float = mov.Float; Integer = mov.Integer; return *this; }

    BSS_FORCEINLINE operator bool() const { return Integer != 0; } // If the string is "true" the integer gets set to 1 by the parser.
    BSS_FORCEINLINE operator char() const { return (char)Integer; }
    BSS_FORCEINLINE operator short() const { return (short)Integer; }
    BSS_FORCEINLINE operator int() const { return (int)Integer; }
    BSS_FORCEINLINE operator int64_t() const { return (int64_t)Integer; }
    BSS_FORCEINLINE operator uint8_t() const { return (uint8_t)Integer; }
    BSS_FORCEINLINE operator uint16_t() const { return (uint16_t)Integer; }
    BSS_FORCEINLINE operator uint32_t() const { return (uint32_t)Integer; }
    BSS_FORCEINLINE operator uint64_t() const { return (uint64_t)Integer; }
    BSS_FORCEINLINE operator float() const { return (float)Float; }
    BSS_FORCEINLINE operator double() const { return Float; }
    BSS_FORCEINLINE operator const char*() const { return String; }
  };

  // Simple XMLNode. Note that return values from GetAttribute are invalid after adding a node or an attribute.
  struct BSS_DLLEXPORT XMLNode : LLBase<XMLNode>
  {
    XMLNode(const XMLNode& copy);
    XMLNode(XMLNode&& mov);
    explicit XMLNode(const char* parse = 0);
    explicit XMLNode(std::istream& stream);
    BSS_FORCEINLINE const char* GetName() const { return _name; }
    BSS_FORCEINLINE const XMLNode* GetNode(size_t index) const { return index >= _nodes.Length() ? 0 : _nodes[index].get(); }
    BSS_FORCEINLINE XMLNode* GetNode(size_t index) { return index >= _nodes.Length() ? 0 : _nodes[index].get(); }
    BSS_FORCEINLINE const XMLNode* GetNode(const char* name) const { return GetNode(_nodehash[name]); }
    BSS_FORCEINLINE XMLNode* GetNode(const char* name) { return GetNode(_nodehash[name]); }
    BSS_FORCEINLINE size_t GetNodes() const { return _nodes.Length(); }
    BSS_FORCEINLINE const XMLValue* GetAttribute(size_t index) const { return index >= _attributes.Length() ? 0 : (_attributes + index); }
    BSS_FORCEINLINE XMLValue* GetAttribute(size_t index) { return index >= _attributes.Length() ? 0 : (_attributes + index); }
    BSS_FORCEINLINE const XMLValue* GetAttribute(const char* name) const { return GetAttribute(_attrhash[name]); }
    BSS_FORCEINLINE XMLValue* GetAttribute(const char* name) { return GetAttribute(_attrhash[name]); }
    BSS_FORCEINLINE const char* GetAttributeString(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->String.c_str(); }
    BSS_FORCEINLINE const int64_t GetAttributeInt(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Integer; }
    BSS_FORCEINLINE const double GetAttributeFloat(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Float; }
    BSS_FORCEINLINE size_t GetAttributes() const { return _attributes.Length(); }
    BSS_FORCEINLINE const XMLValue& GetValue() const { return _value; }
    BSS_FORCEINLINE XMLValue& GetValue() { return _value; }
    BSS_FORCEINLINE void SetName(const char* name) { _name = name; }
    BSS_FORCEINLINE const Hash<Str, size_t, ARRAY_SAFE>& NodeHash() const { return _nodehash; }
    void Clear();
    XMLNode* AddNode(const XMLNode& node);
    XMLNode* AddNode(const char* name);
    XMLValue* AddAttribute(const XMLValue& value);
    XMLValue* AddAttribute(const char* name);
    bool RemoveNode(size_t index);
    bool RemoveNode(const char* name);
    bool RemoveAttribute(size_t index);
    bool RemoveAttribute(const char* name);
    void SetValue(double value);
    void SetValue(int64_t value);
    void SetValue(const char* value);
    BSS_FORCEINLINE std::unique_ptr<XMLNode>* begin() noexcept { return _nodes.begin(); }
    BSS_FORCEINLINE const std::unique_ptr<XMLNode>* begin() const noexcept { return _nodes.begin(); }
    BSS_FORCEINLINE std::unique_ptr<XMLNode>* end() noexcept { return _nodes.end(); }
    BSS_FORCEINLINE const std::unique_ptr<XMLNode>* end() const noexcept { return _nodes.end(); }

    XMLNode& operator=(const XMLNode& copy);
    XMLNode& operator=(XMLNode&& mov);
    BSS_FORCEINLINE const XMLNode* operator[](size_t index) const { return GetNode(index); }
    BSS_FORCEINLINE const XMLNode* operator[](const char* name) const { return GetNode(name); }
    BSS_FORCEINLINE const XMLValue* operator()(size_t index) const { return GetAttribute(index); }
    BSS_FORCEINLINE const XMLValue* operator()(const char* name) const { return GetAttribute(name); }

  protected:
    XMLNode* _addNode(std::unique_ptr<XMLNode> && n);
    XMLValue* _addAttribute(XMLValue && v);
    bool _parse(std::istream& stream, Str& buf);
    void _parseInner(std::istream& stream, Str& buf);
    void _parseAttribute(Str& buf);
    void _writeAttribute(std::ostream& stream) const;
    void _write(std::ostream& stream, bool pretty, int depth) const;

    static bool _match(std::istream& stream, Str& out, const char* pattern, bool reset = false);
    static void _parseEntity(std::istream& stream, Str& target);
    static void _evalValue(XMLValue& val);
    static void _writeString(std::ostream& stream, const char* s, bool attribute);

    friend class XMLFile;

    DynArray<std::unique_ptr<XMLNode>, size_t, ARRAY_MOVE> _nodes;
    Hash<Str, size_t, ARRAY_SAFE> _nodehash;
    DynArray<XMLValue, size_t, ARRAY_MOVE> _attributes;
    Hash<Str, size_t, ARRAY_SAFE> _attrhash;
    XMLValue _value;
    Str _name;
  };

  // Tiny XML parser
  class BSS_DLLEXPORT XMLFile : public XMLNode
  {
  public:
    XMLFile(const XMLFile& copy);
    XMLFile(XMLFile&& mov);
    explicit XMLFile(const char* source);
    explicit XMLFile(std::istream& stream);
    XMLFile();
    void Write(const char* file, bool pretty = true) const;
    void Write(std::ostream& stream, bool pretty = true) const;
    void Read(const char* source);
    void Read(std::istream& stream);

    inline XMLFile& operator=(const XMLFile& copy) = default;
    inline XMLFile& operator=(XMLFile&& mov) = default;
    BSS_FORCEINLINE const XMLNode* operator[](size_t index) const { return GetNode(index); }
    BSS_FORCEINLINE const XMLNode* operator[](const char* name) const { return GetNode(name); }
    BSS_FORCEINLINE const XMLValue* operator()(size_t index) const { return GetAttribute(index); }
    BSS_FORCEINLINE const XMLValue* operator()(const char* name) const { return GetAttribute(name); }

  protected:
    void _initialParse(std::istream& stream, Str& buf);
  };

  class XMLEngine
  {
  public:
    XMLEngine() : pretty(true), arrayID(0), cur(0), curvalue(0), curindices(0) {}
    static constexpr bool Ordered() { return false; }
    static void Begin(Serializer<XMLEngine>& e)
    {
      e.engine.file.Clear();
      e.engine.arrayID = 0;
      e.engine.cur = &e.engine.file;
      e.engine.curvalue = 0;
      e.engine.curindices = 0;

      if(e.in)
        e.engine.file.Read(*e.in);
    }
    static void End(Serializer<XMLEngine>& e)
    {
      if(e.out)
        e.engine.file.Write(*e.out, e.engine.pretty);
    }
    template<typename T>
    static void Serialize(Serializer<XMLEngine>& e, const T& t, const char* id)
    {
      if constexpr(std::is_base_of<std::string, T>::value)
      {
        XMLValue* v = id ? e.engine.cur->AddAttribute(id) : &e.engine.cur->AddNode(e.engine.arrayID)->GetValue();
        v->String = t.c_str();
      }
      else
      {
        internal::serializer::PushValue<XMLNode*> push(e.engine.cur, e.engine.cur->AddNode(!id ? e.engine.arrayID : id));
        static_assert(internal::serializer::is_serializable<XMLEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
        const_cast<T&>(t).template Serialize<XMLEngine>(e, id);
      }
    }
    template<typename T>
    static void SerializeArray(Serializer<XMLEngine>& e, const T& t, size_t size, const char* id)
    {
      internal::serializer::PushValue<const char*> push(e.engine.arrayID, id);
      auto begin = std::begin(t);
      auto end = std::end(t);
      for(; begin != end; ++begin)
        Serializer<XMLEngine>::ActionBind<remove_cvref_t<decltype(*begin)>>::Serialize(e, *begin, 0);
    }
    template<typename T, size_t... S>
    static void SerializeTuple(Serializer<XMLEngine>& e, const T& t, const char* id, std::index_sequence<S...>)
    {
      internal::serializer::PushValue<const char*> push(e.engine.arrayID, id);
      (Serializer<XMLEngine>::ActionBind<std::tuple_element_t<S, T>>::Serialize(e, std::get<S>(t), 0), ...);
    }
    template<typename T>
    static void SerializeNumber(Serializer<XMLEngine>& e, T t, const char* id)
    {
      XMLValue* v = id ? e.engine.cur->AddAttribute(id) : &e.engine.cur->AddNode(e.engine.arrayID)->GetValue(); // We don't need to update e.engine.cur here because there's nothing else to serialize
      std::ostringstream ss(std::ios_base::out);

      if constexpr(std::is_floating_point<T>::value)
        ss << static_cast<double>(t);
      else
        ss << static_cast<int64_t>(t);
      v->String = ss.str();
    }
    static void SerializeBool(Serializer<XMLEngine>& e, bool t, const char* id)
    {
      XMLValue* v = id ? e.engine.cur->AddAttribute(id) : &e.engine.cur->AddNode(e.engine.arrayID)->GetValue();
      v->String = t ? "true" : "false";
    }

    template<typename T>
    static void Parse(Serializer<XMLEngine>& e, T& obj, const char* id)
    {
      internal::serializer::PushValue<XMLNode*> push(e.engine.cur, e.engine.cur);
      if(id && !e.engine.curvalue)
        e.engine.cur = e.engine.cur->GetNode(id);

      if(e.engine.cur != 0)
      {
        internal::serializer::PushValue<XMLValue*> push2(e.engine.curvalue, e.engine.curvalue);
        if(id && !e.engine.curvalue)
          e.engine.curvalue = &e.engine.cur->GetValue();

        if constexpr(std::is_base_of<std::string, T>::value)
          obj = e.engine.curvalue->String;
        else
        {
          static_assert(internal::serializer::is_serializable<XMLEngine, T>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");
          obj.template Serialize<XMLEngine>(e, id);
        }
      }
    }
    template<typename T, typename E, void (*Add)(Serializer<XMLEngine>& e, T& obj, int& n), bool (*Read)(Serializer<XMLEngine>& e, T& obj, int64_t count)>
    static void ParseArray(Serializer<XMLEngine>& e, T& obj, const char* id)
    {
      if(id && !e.engine.curvalue)
      {
        int n = 0;
        auto end = std::end(*e.engine.cur);

        for(auto begin = std::begin(*e.engine.cur); begin != end; ++begin)
          if(!strcmp(begin[0]->GetName(), id))
          {
            internal::serializer::PushValue<XMLNode*> push1(e.engine.cur, begin[0].get());
            internal::serializer::PushValue<XMLValue*> push2(e.engine.curvalue, &e.engine.cur->GetValue());
            Add(e, obj, n);
          }
      }
      else
      {
        khiter_t i = e.engine.curindices->Iterator(e.engine.cur->GetName());
        if(!e.engine.curindices->ExistsIter(i))
          i = e.engine.curindices->Insert(e.engine.cur->GetName(), 0);
        Add(e, obj, e.engine.curindices->Value(i));
      }
    }
    template<typename T>
    static void ParseNumber(Serializer<XMLEngine>& e, T& t, const char* id)
    {
      const XMLValue* v = e.engine.curvalue;

      if constexpr(std::is_floating_point<T>::value)
        t = static_cast<T>(v->Float);
      else
        t = static_cast<T>(v->Integer);
    }
    static void ParseBool(Serializer<XMLEngine>& e, bool& t, const char* id)
    {
      const XMLValue* v = e.engine.curvalue;

      if(!STRICMP(v->String.c_str(), "true"))
        t = true;
      else if(!STRICMP(v->String.c_str(), "false"))
        t = false;
      else
        t = !!v->Integer;
    }
    template<typename F>
    static void ParseMany(Serializer<XMLEngine>& e, F && f)
    {
      Hash<const char*, int> indices;
      internal::serializer::PushValue<Hash<const char*, int>*> push(e.engine.curindices, &indices);
      size_t n = e.engine.cur->GetAttributes();
      XMLValue* old = e.engine.curvalue;
      for(size_t i = 0; i < n; ++i)
        f(e, (e.engine.curvalue = e.engine.cur->GetAttribute(i))->Name);

      XMLNode* oldnode = e.engine.cur;
      auto end = std::end(*oldnode); // Iterate through each UNIQUE key. The array parser will iterate through all the nodes.
      for(auto begin = std::begin(*oldnode); begin != end; ++begin)
      {
        e.engine.cur = begin[0].get();
        e.engine.curvalue = &e.engine.cur->GetValue();
        f(e, e.engine.cur->GetName());
      }
      e.engine.curvalue = old;
      e.engine.cur = oldnode;
    }

    XMLFile file;
    XMLNode* cur;
    XMLValue* curvalue;
    const char* arrayID;
    Hash<const char*, int>* curindices;
    bool pretty;
  };
}



#endif