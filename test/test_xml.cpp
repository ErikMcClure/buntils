// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cXML.h"

using namespace bss_util;

TESTDEF::RETPAIR test_XML()
{
  BEGINTEST;
  cStr XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><foo>abc&amp;<bar></bar>zxy<bar2><!-- comment --></bar2  ><   bar/> <  test    />  <!-- comment --><test test=\"attr\" /><!-- comment --></foo> <foo again=\"true\" fail></foo> <bob></bob><!-- comment --><foo test=\"test\" test=\"success\" /><!-- comment -->";
  cXML xml(XML);
  TEST(xml.GetNodes() == 4);
  TEST(xml[0]->GetName() == cStr("foo"));
  TEST(xml[0]->GetAttributes() == 0);
  TEST(xml[0]->GetNodes() == 5);
  const cXMLNode* n = xml[1];
  TEST(xml[1]->GetName() == cStr("foo"));
  TEST(xml[1]->GetAttributes() == 2);
  TEST(xml[1]->GetNodes() == 0);
  n = xml[2];
  TEST(xml[2]->GetName() == cStr("bob"));
  TEST(xml[2]->GetAttributes() == 0);
  TEST(xml[2]->GetNodes() == 0);
  TEST(xml[3]->GetName() == cStr("foo"));
  TEST(xml[3]->GetAttributes() == 1);
  TEST(xml[3]->GetNodes() == 0);

  xml.Write("test.xml");

  // Ensure that, even if we can't recover from various errors, the parser does not crash or go into an infinite loop due to bad data
  for(uint32_t i = 1; i < XML.length(); ++i)
  {
    cXML x(XML.substr(0, i).c_str());
    TEST(xml.GetName() != 0); //keep this from getting optimized out
  }

  // Feed in an enormous amount of random gibberish to try and crash the parser
  const int AMOUNT = 1000000;
  cStr s(AMOUNT);
  for(int i = 0; i < AMOUNT; ++i)
    s += RANDINTGEN(33, 63);

  cXML x(s);
  TEST(x.GetName() != 0);
  ENDTEST;
}
