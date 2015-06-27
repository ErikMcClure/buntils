// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_XML_H__BSS__
#define __C_XML_H__BSS__

#include "cDynArray.h"
#include "cHash.h"
#include "LLBase.h"
#include "cStr.h"

namespace bss_util {
  // Represents an XML value converted to various forms.
  struct cXMLValue
  {
    cStr String;
    double Float;
    unsigned __int64 Integer;

    BSS_FORCEINLINE operator bool() const { return Integer!=0; } // If the string is "true" the integer gets set to 1 by the parser.
    BSS_FORCEINLINE operator char() const { return (char)Integer; }
    BSS_FORCEINLINE operator short() const { return (short)Integer; }
    BSS_FORCEINLINE operator int() const { return (int)Integer; }
    BSS_FORCEINLINE operator long() const { return (long)Integer; }
    BSS_FORCEINLINE operator __int64() const { return (__int64)Integer; }
    BSS_FORCEINLINE operator unsigned char() const { return (unsigned char)Integer; }
    BSS_FORCEINLINE operator unsigned short() const { return (unsigned short)Integer; }
    BSS_FORCEINLINE operator unsigned int() const { return (unsigned int)Integer; }
    BSS_FORCEINLINE operator unsigned long() const { return (unsigned long)Integer; }
    BSS_FORCEINLINE operator unsigned __int64() const { return (unsigned __int64)Integer; }
    BSS_FORCEINLINE operator float() const { return (float)Float; }
    BSS_FORCEINLINE operator double() const { return Float; }
    BSS_FORCEINLINE operator const char*() const { return String; }
  };

  // Simple cXMLNode. Note that return values from GetNode and GetAttribute are invalid after adding a node or an attribute.
  struct cXMLNode : LLBase<cXMLNode>
  {
    cXMLNode(const cXMLNode& copy);
    cXMLNode(cXMLNode&& mov);
    explicit cXMLNode(const char* parse=0);
    explicit cXMLNode(std::istream& stream);
    BSS_FORCEINLINE const char* GetName() const { return _name; }
    BSS_FORCEINLINE const cXMLNode* GetNode(unsigned int index) const { return index>=_nodes.Length()?0:(_nodes+index); }
    BSS_FORCEINLINE const cXMLNode* GetNode(const char* name) const { return _nodehash[name]; }
    BSS_FORCEINLINE const cXMLValue* GetAttribute(unsigned int index) const { return index>=_attributes.Length()?0:(_attributes+index); }
    BSS_FORCEINLINE const cXMLValue* GetAttribute(const char* name) const { return _attrhash[name]; }
    BSS_FORCEINLINE const cXMLValue& GetValue() const { return _value; }

    cXMLNode& operator=(const cXMLNode& copy);
    cXMLNode& operator=(cXMLNode&& mov);
    BSS_FORCEINLINE const cXMLNode* operator[](const char* node) const { return GetNode(node); }

  protected:
    void _parse(std::istream& stream, cStr& buf);
    void _parseattribute(std::istream& stream, cStr& buf);

    cDynArray<cXMLNode, unsigned int, CARRAY_SAFE> _nodes;
    cHash<const char*, cXMLNode*> _nodehash;
    cDynArray<cXMLValue, unsigned int, CARRAY_SAFE> _attributes;
    cHash<const char*, cXMLValue*> _attrhash;
    cXMLValue _value;
    cStr _name;
  };

  // Tiny XML parser
  class BSS_DLLEXPORT cXML : public cXMLNode
  {
    cXML(const cXML& copy);
    cXML(cXML&& mov);
    explicit cXML(const char* source=0);
    explicit cXML(std::istream& stream);

    inline cXML& operator=(const cXML& copy) { cXMLNode::operator=(copy); return *this; }
    inline cXML& operator=(cXML&& mov) { cXMLNode::operator=(mov); return *this; }
    BSS_FORCEINLINE const cXMLNode* operator[](const char* node) const { return GetNode(node); }

  protected:
    void _initialparse(std::istream& stream, cStr& buf);
  };
}



#endif