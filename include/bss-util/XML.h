// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __XML_H__BSS__
#define __XML_H__BSS__

#include "DynArray.h"
#include "Hash.h"
#include "LLBase.h"
#include "Str.h"

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
    BSS_FORCEINLINE operator long() const { return (long)Integer; }
    BSS_FORCEINLINE operator int64_t() const { return (int64_t)Integer; }
    BSS_FORCEINLINE operator uint8_t() const { return (uint8_t)Integer; }
    BSS_FORCEINLINE operator uint16_t() const { return (uint16_t)Integer; }
    BSS_FORCEINLINE operator uint32_t() const { return (uint32_t)Integer; }
    BSS_FORCEINLINE operator unsigned long() const { return (unsigned long)Integer; }
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
    BSS_FORCEINLINE const XMLNode* GetNode(const char* name) const { return GetNode(_nodehash[name]); }
    BSS_FORCEINLINE size_t GetNodes() const { return _nodes.Length(); }
    BSS_FORCEINLINE const XMLValue* GetAttribute(size_t index) const { return index >= _attributes.Length() ? 0 : (_attributes + index); }
    BSS_FORCEINLINE const XMLValue* GetAttribute(const char* name) const { return GetAttribute(_attrhash[name]); }
    BSS_FORCEINLINE const char* GetAttributeString(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->String.c_str(); }
    BSS_FORCEINLINE const int64_t GetAttributeInt(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Integer; }
    BSS_FORCEINLINE const double GetAttributeFloat(const char* name) const { const XMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Float; }
    BSS_FORCEINLINE size_t GetAttributes() const { return _attributes.Length(); }
    BSS_FORCEINLINE const XMLValue& GetValue() const { return _value; }
    BSS_FORCEINLINE void SetName(const char* name) { _name = name; }
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
    BSS_FORCEINLINE const std::unique_ptr<XMLNode>* begin() const noexcept { return _nodes.begin(); }
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
    static bool _match(std::istream& stream, Str& out, const char* pattern, bool reset = false);
    bool _parse(std::istream& stream, Str& buf);
    void _parseInner(std::istream& stream, Str& buf);
    void _parseAttribute(Str& buf);
    static void _parseEntity(std::istream& stream, Str& target);
    static void _evalValue(XMLValue& val);
    void _writeAttribute(std::ostream& stream) const;
    static void _writeString(std::ostream& stream, const char* s);
    void _write(std::ostream& stream, bool pretty, int depth) const;

    friend class XMLFile;

    DynArray<std::unique_ptr<XMLNode>, size_t, ARRAY_MOVE> _nodes;
    Hash<Str, size_t, false, ARRAY_SAFE> _nodehash;
    DynArray<XMLValue, size_t, ARRAY_MOVE> _attributes;
    Hash<Str, size_t, false, ARRAY_SAFE> _attrhash;
    XMLValue _value;
    Str _name;
  };

  // Tiny XML parser
  class BSS_DLLEXPORT XMLFile : public XMLNode
  {
  public:
    XMLFile(const XMLFile& copy);
    XMLFile(XMLFile&& mov);
    explicit XMLFile(const char* source = 0);
    explicit XMLFile(std::istream& stream);
    void Write(const char* file, bool pretty = true) const;
    void Write(std::ostream& stream, bool pretty = true) const;

    inline XMLFile& operator=(const XMLFile& copy) { XMLNode::operator=(copy); return *this; }
    inline XMLFile& operator=(XMLFile&& mov) { XMLNode::operator=(mov); return *this; }
    BSS_FORCEINLINE const XMLNode* operator[](size_t index) const { return GetNode(index); }
    BSS_FORCEINLINE const XMLNode* operator[](const char* name) const { return GetNode(name); }
    BSS_FORCEINLINE const XMLValue* operator()(size_t index) const { return GetAttribute(index); }
    BSS_FORCEINLINE const XMLValue* operator()(const char* name) const { return GetAttribute(name); }

  protected:
    void _initialParse(std::istream& stream, Str& buf);
  };
}



#endif