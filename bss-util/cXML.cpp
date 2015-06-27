// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cXML.h"
#include <sstream>

using namespace bss_util;

cXML::cXML(const cXML& copy) : cXMLNode(copy) {}
cXML::cXML(cXML&& mov) : cXMLNode(std::move(mov)) {}
cXML::cXML(const char* source) { if(source) { cStr buf; std::istringstream ss(source); _initialparse(ss, buf); _parse(ss, buf); } }
cXML::cXML(std::istream& stream) { cStr buf; _initialparse(stream, buf); _parse(stream, buf); }

void cXML::_initialparse(std::istream& stream, cStr& buf)
{
  while(!stream.eof() && isspace(stream.peek())) stream.get();
  if(stream.eof() || stream.peek() != '<') return;
  stream.get();
  if(stream.peek() != '?') return;
  _parseattribute(stream, buf);
}


cXMLNode::cXMLNode(const cXMLNode& copy) {}
cXMLNode::cXMLNode(cXMLNode&& mov) {}
cXMLNode::cXMLNode(const char* parse=0) { if(parse) { std::istringstream ss(parse); _parse(ss); } }
cXMLNode::cXMLNode(std::istream& stream) { _parse(stream); }

cXMLNode& cXMLNode::operator=(const cXMLNode& copy)
{
  _nodes = copy._nodes;
  _nodehash = copy._nodehash;
  _attributes = copy._attributes;
  _attrhash = copy._attrhash;
  _value = copy._value;
  _name = copy._name;
  // TODO: REBASE LINKED LIST OF DUPLICATES
  return *this;
}
cXMLNode& cXMLNode::operator=(cXMLNode&& mov)
{
  _nodes = std::move(mov._nodes);
  _nodehash = std::move(mov._nodehash);
  _attributes = std::move(mov._attributes);
  _attrhash = std::move(mov._attrhash);
  _value = std::move(mov._value);
  _name = std::move(mov._name);
  return *this;
}

void cXMLNode::_parse(std::istream& stream, cStr& buf)
{
  buf.clear();
  while(!stream.eof() && isspace(stream.peek())) stream.get();
  if(stream.peek() == '<') stream.get();
  _parseattribute(stream, buf);

}

void cXMLNode::_parseattribute(std::istream& stream, cStr& buf)
{
  buf.clear();
  while(!stream.eof() && !isspace(stream.peek()) && stream.peek() != '>') buf += stream.get();

}
