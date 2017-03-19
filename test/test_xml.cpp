// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cXML.h"
#include <fstream>
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_XML()
{
  BEGINTEST;

  cXML test2("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<room fancyname=\"Space\" desc=\"This is space.\" startroom=\"0xFFFFFFFFFFFFFFFF\" north=\"TESTROOMNORTH\" roomid=\"TESTROOM\" />\n<room roomid = \"TESTROOMNORTH\" nm:fancyname = \"-200000\" desc = \"This is the room to the north.\" south = \"TESTROOM\" />");

  TEST(!strcmp(test2[(size_t)0]->GetAttribute("roomid")->String, "TESTROOM"));
  TEST(!strcmp(test2[(size_t)0]->GetAttribute((size_t)0)->String, "Space"));
  TEST(!strcmp(test2[(size_t)0]->GetAttribute(1)->String, "This is space."));
  TEST(!strcmp(test2[(size_t)0]->GetAttribute(2)->String, "0xFFFFFFFFFFFFFFFF"));
  TEST(((uint64_t)test2[(size_t)0]->GetAttribute(2)->Integer) == 0xFFFFFFFFFFFFFFFF);
  TEST(!strcmp(test2[(size_t)0]->GetAttribute(3)->String, "TESTROOMNORTH"));
  TEST(!strcmp(test2[(size_t)0]->GetAttribute(4)->Name, "roomid"));
  TEST(!strcmp((*test2[1])("roomid")->String, "TESTROOMNORTH"));
  TEST(!strcmp(test2[1]->GetAttribute((size_t)0)->Name, "roomid"));
  TEST(!strcmp(test2[1]->GetAttribute(1)->String, "-200000"));
  TEST(test2[1]->GetAttribute(1)->Integer == -200000);
  TEST(!strcmp(test2[1]->GetAttribute(1)->Name, "nm:fancyname"));
  TEST(!strcmp(test2[1]->GetAttribute(2)->String, "This is the room to the north."));
  TEST(!strcmp(test2[1]->GetAttribute(3)->String, "TESTROOM"));
  TEST(!strcmp(test2[1]->GetAttributeString("south"), "TESTROOM"));
  TEST(!test2[1]->GetAttributeInt("roomid"));
  TEST(!test2[1]->GetAttributeString(0));
  TEST(!test2[1]->GetAttributeInt(0));


  cStr XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><foo>abc&amp;<bar></bar>zxy<bar2><!-- comment --></bar2  ><   bar/> <  test    />  <!-- comment --><test test=\"attr\" /><!-- comment --></foo> <foo again=\"true\" fail></foo> <bobasdfghqwertyuiopasdfzcvxnm></bobasdfghqwertyuiopasdfzcvxnm><!-- comment --><foo test=\"test\" test=\"success\" /><!-- comment -->";
  cXML xml(XML);
  TEST(xml.GetNodes() == 4);
  TEST(xml[(size_t)0]->GetName() == cStr("foo"));
  TEST(xml[(size_t)0]->GetAttributes() == 0);
  TEST(xml[(size_t)0]->GetNodes() == 5);
  const cXMLNode* n = xml[1];
  TEST(xml[1]->GetName() == cStr("foo"));
  TEST(xml[1]->GetAttributes() == 2);
  TEST(xml[1]->GetNodes() == 0);
  n = xml[2];
  TEST(xml[2]->GetName() == cStr("bobasdfghqwertyuiopasdfzcvxnm"));
  TEST(xml[2]->GetAttributes() == 0);
  TEST(xml[2]->GetNodes() == 0);
  TEST(xml[3]->GetName() == cStr("foo"));
  TEST(xml[3]->GetAttributes() == 1);
  TEST(xml[3]->GetNodes() == 0);
  
  xml.Write("test.xml");

  cXML construct;
  construct.SetName("xml");
  construct.AddAttribute("version")->String = "1.0";
  construct.AddAttribute("encoding")->String = "UTF-8";
  cXMLNode* node = construct.AddNode("foo");
  node->SetValue("abc&zxy   ");
  node->AddNode("bar");
  node->AddNode("bar2");
  node->AddNode("bar");
  node->AddNode("test");
  node->AddNode("test")->AddAttribute("test")->String = "attr";
  node = construct.AddNode("foo");
  node->AddAttribute("again")->String = "true";
  node->AddAttribute("failfail");
  construct.AddNode("bobasdfghqwertyuiopasdfzcvxnm");
  construct.AddNode("foo")->AddAttribute("test")->String = "success";
  cStr compare(bssloadfile<char, true>("test.xml").first.get());
  construct.Write("test.xml");
  cStr compare2(bssloadfile<char, true>("test.xml").first.get());
  TEST(compare == compare2);

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
