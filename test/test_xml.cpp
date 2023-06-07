// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/XML.h"
#include <fstream>
#include <sstream>

using namespace bun;

struct XMLtest3
{
  float f;

  template<typename Engine>
  void Serialize(Serializer<Engine>& s, const char*)
  {
    s.template EvaluateType<XMLtest3>(GenPair("f", f));
  }
};


struct XMLtest2
{
  uint16_t a;
  std::vector<XMLtest3> test;

  template<typename Engine>
  void Serialize(Serializer<Engine>& s, const char*)
  {
    s.template EvaluateType<XMLtest2>(GenPair("a", a), GenPair("test", test));
  }
};

struct XMLtest
{
  int64_t a;
  uint16_t b;
  double c;
  Str test;
  XMLtest2 test2;
  bool btrue;
  bool bfalse;
  DynArray<double> d;
  int e[3];
  std::vector<Str> f;
  std::array<bool, 2> g;
  DynArray<XMLtest2, size_t> nested;
  std::tuple<short, Str, double> tuple;

  template<typename Engine>
  void Serialize(Serializer<Engine>& s, const char*)
  {
    s.template EvaluateType<XMLtest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c),
      GenPair("test", test),
      GenPair("test2", test2),
      GenPair("btrue", btrue),
      GenPair("bfalse", bfalse),
      GenPair("d", d),
      GenPair("e", e),
      GenPair("f", f),
      GenPair("g", g),
      GenPair("nested", nested),
      GenPair("tuple", tuple)
      );
  }
};

void dotest_XML(XMLtest& o, TESTDEF::RETPAIR& __testret)
{
  TEST(o.a == -1);
  TEST(o.b == 2);
  TEST(o.c == 0.28);
  TEST(o.btrue == true);
  TEST(o.bfalse == false);
  TEST(o.test == "tEsT");
  TEST(o.d.Length() == 6);
  TEST(o.d[0] == 1e-6);
  TEST(o.d[1] == -2.0);
  TEST(o.d[2] == 0.3);
  TEST(o.d[3] == 0.1);
  TEST(o.d[4] == 1.0);
  TEST(o.d[5] == -2.0);
  TEST(o.e[0] == -2);
  TEST(o.e[1] == 100);
  TEST(o.e[2] == 8);
  TEST(o.f.size() == 4);
  TEST(!strcmp(o.f[0], "\"first\""));
  TEST(!strcmp(o.f[1], "SECOND"));
  TEST(!strcmp(o.f[2], "ThirD"));
  TEST(!strcmp(o.f[3], "fOURTh"));
  TEST(!o.g[0]);
  TEST(o.g[1]);
  TEST(o.test2.a == 5);
  TEST(o.test2.test.size() == 2);
  TEST(o.test2.test[0].f == -3.5f);
  TEST(o.test2.test[1].f == 14.6f);
  TEST(o.nested.Length() == 3);
  TEST(o.nested[0].a == 2);
  TEST(o.nested[0].test.size() == 2);
  TEST(o.nested[0].test[0].f == 80.9f);
  TEST(o.nested[0].test[1].f == -91.2f);
  TEST(o.nested[1].a == 3);
  TEST(o.nested[1].test.size() == 1);
  TEST(o.nested[1].test[0].f == -12.21f);
  TEST(o.nested[2].a == 4);
  TEST(o.nested[2].test.size() == 0);
  auto[a, b, c] = o.tuple;
  TEST(a == -3);
  TEST(b == "2");
  TEST(c == 1.0);
}

TESTDEF::RETPAIR test_XML()
{
  BEGINTEST;

  XMLFile test2("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<room fancyname=\"Space\" desc=\"This is space.\" startroom=\"0xFFFFFFFFFFFFFFFF\" north=\"TESTROOMNORTH\" roomid=\"TESTROOM\" />\n<room roomid = \"TESTROOMNORTH\" nm:fancyname = \"-200000\" desc = \"This is the room to the north.\" south = \"TESTROOM\" />");

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

  Str strXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><foo>abc&amp;<bar></bar>zxy<bar2><!-- comment --></bar2  ><   bar/> <  test    />  <!-- comment --><test test=\"attr\" /><!-- comment --></foo> <foo again=\"true\" fail></foo> <bobasdfghqwertyuiopasdfzcvxnm></bobasdfghqwertyuiopasdfzcvxnm><!-- comment --><foo test=\"test\" test=\"success\" /><!-- comment -->";
  XMLFile xml(strXML);
  TEST(xml.GetNodes() == 4);
  TEST(xml[(size_t)0]->GetName() == Str("foo"));
  TEST(xml[(size_t)0]->GetAttributes() == 0);
  TEST(xml[(size_t)0]->GetNodes() == 5);
  TEST(xml[1]->GetName() == Str("foo"));
  TEST(xml[1]->GetAttributes() == 2);
  TEST(xml[1]->GetNodes() == 0);
  TEST(xml[2]->GetName() == Str("bobasdfghqwertyuiopasdfzcvxnm"));
  TEST(xml[2]->GetAttributes() == 0);
  TEST(xml[2]->GetNodes() == 0);
  TEST(xml[3]->GetName() == Str("foo"));
  TEST(xml[3]->GetAttributes() == 1);
  TEST(xml[3]->GetNodes() == 0);
  
  xml.Write("test.xml");

  XMLFile construct;
  construct.SetName("xml");
  construct.AddAttribute("version")->String = "1.0";
  construct.AddAttribute("encoding")->String = "UTF-8";
  XMLNode* node = construct.AddNode("foo");
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
  Str compare(LoadFile<char, true>("test.xml").first.get());
  construct.Write("test.xml");
  Str compare2(LoadFile<char, true>("test.xml").first.get());
  TEST(compare == compare2);

  Str objtest = TXT(
    <?xml version="1.0" encoding="UTF-8"?>
    <xmltest a="-1" b="2" c="0.28" test="tEsT" btrue="true" bfalse="false">
      <test2 a="5">
        <test f="-3.5" />
        <test f="14.6" />
      </test2>
      <d>1e-6</d>
      <d>-2.0</d>
      <d>0.3</d>
      <d>0.1</d>
      <d>1.0</d>
      <d>-2.0</d>
      <e>-2</e>
      <e>100</e>
      <e>8</e>
      <f>"first"</f>
      <f>SECOND</f>
      <f>ThirD</f>
      <f>fOURTh</f>
      <g>false</g>
      <g>true</g>
      <nested a="2">
        <test f="80.9" />
        <test f="-91.2" />
      </nested>
      <nested a="3">
        <test f="-12.21" />
      </nested>
      <nested a = "4" />
      <tuple>-3</tuple>
      <tuple>2</tuple>
      <tuple>1.0</tuple>
    </xmltest>
  );

  {
    Serializer<XMLEngine> s;
    XMLtest obj;
    std::istringstream ss(objtest);
    s.Parse(obj, ss, "xmltest");
    dotest_XML(obj, __testret);

    std::stringstream ss2;
    s.Serialize(obj, ss2, "xmltest");
    
    XMLtest obj2;
    s.Parse(obj2, ss2, "xmltest");
    dotest_XML(obj2, __testret);
  }

  // Ensure that, even if we can't recover from various errors, the parser does not crash or go into an infinite loop due to bad data
  for(size_t i = 1; i < strXML.length(); ++i)
  {
    XMLFile x(strXML.substr(0, i).c_str());
    TEST(xml.GetName() != 0); //keep this from getting optimized out
  }

  // Feed in an enormous amount of random gibberish to try and crash the parser
  const int AMOUNT = 1000000;
  Str s(AMOUNT);
  for(size_t i = 0; i < AMOUNT; ++i)
    s += RANDINTGEN(33, 63);

  XMLFile x(s);
  TEST(x.GetName() != 0);

  ENDTEST;
}
