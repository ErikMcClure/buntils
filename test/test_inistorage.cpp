// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/INIstorage.h"

using namespace bun;

#define INI_E(s,k,v,nk,ns) TEST(!ini.EditEntry(TXT(s),TXT(k),TXT(v),nk,ns))
#define INI_NE(s,k,v,nk,ns) TEST(ini.EditEntry(TXT(s),TXT(k),TXT(v),nk,ns)<0)
#define INI_R(s,k,nk,ns) TEST(!ini.EditEntry(TXT(s),TXT(k),0,nk,ns))
#define INI_G(s,k,nk,ns) TEST(!ini.EditEntry(TXT(s),TXT(k),TXT(v),nk,ns))

TESTDEF::RETPAIR test_INISTORAGE()
{
  BEGINTEST;

  INIstorage ini("inistorage.ini");

  auto fn = [&](INIentry* e, const char* s, int i) -> bool { return (e != 0 && !strcmp(e->GetString(), s) && e->GetInt() == i); };
  auto fn2 = [&](const char* s) {
    FILE* f;
    FOPEN(f, "inistorage.ini", "rb"); //this will create the file if it doesn't already exist
    TEST(f != 0);
    if(f != 0)
    {
      fseek(f, 0, SEEK_END);
      size_t size = (size_t)ftell(f);
      fseek(f, 0, SEEK_SET);
      Str str(size + 1);
      size = fread(str.UnsafeString(), sizeof(char), size, f); //reads in the entire file
      str.UnsafeString()[size] = '\0';
      fclose(f);
      TEST(!strcmp(str, s));
    }
  };
  auto fn3 = [&]() {
    ini.DEBUGTEST();
    ini.AddSection("1");
    ini.DEBUGTEST();
    INI_E(1, a, 1, -1, 0);
    ini.DEBUGTEST();
    INI_E(1, a, 2, -1, 0);
    ini.DEBUGTEST();
    INI_E(1, a, 3, -1, 0);
    ini.DEBUGTEST();
    INI_E(1, a, 4, -1, 0);
    ini.DEBUGTEST();
    INI_E(1, b, 1, -1, 0);
    ini.DEBUGTEST();
    ini.AddSection("2");
    ini.DEBUGTEST();
    INI_E(2, a, 1, -1, 0);
    INI_E(2, a, 2, -1, 0);
    INI_E(2, b, 1, -1, 0);
    ini.AddSection("2");
    INI_E(2, a, 1, -1, 1);
    INI_E(2, a, 2, -1, 1);
    INI_E(2, b, 1, -1, 1);
    INI_E(1, c, 1, -1, 0); // We do these over here to test adding things into the middle of the file
    INI_E(1, c, 2, -1, 0);
    INI_E(1, d, 1, -1, 0);
    ini.AddSection("2");
  };
  auto fn4 = [&](const char* s, uint32_t index) -> bool {
    INIsection* sec = ini.GetSection(s, index);
    return sec != 0 && sec->GetIndex() == index;
  };

  fn3();

  TEST(fn(ini.GetEntryPtr("1", "a", 0, 0), "1", 1));
  TEST(fn(ini.GetEntryPtr("1", "a", 1, 0), "2", 2));
  TEST(fn(ini.GetEntryPtr("1", "a", 2, 0), "3", 3));
  TEST(fn(ini.GetEntryPtr("1", "a", 3, 0), "4", 4));
  TEST(!ini.GetEntryPtr("1", "a", 4, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "1", 1));
  TEST(!ini.GetEntryPtr("1", "b", 1, 0));
  TEST(fn(ini.GetEntryPtr("1", "c", 0, 0), "1", 1));
  TEST(fn(ini.GetEntryPtr("1", "c", 1, 0), "2", 2));
  TEST(!ini.GetEntryPtr("1", "c", 2, 0));
  TEST(fn(ini.GetEntryPtr("1", "d", 0, 0), "1", 1));
  TEST(!ini.GetEntryPtr("1", "d", 1, 0));
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 0), "1", 1));
  TEST(fn(ini.GetEntryPtr("2", "a", 1, 0), "2", 2));
  TEST(!ini.GetEntryPtr("2", "a", 2, 0));
  TEST(fn(ini.GetEntryPtr("2", "b", 0, 0), "1", 1));
  TEST(!ini.GetEntryPtr("2", "b", 1, 0));
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 1), "1", 1));
  TEST(fn(ini.GetEntryPtr("2", "a", 1, 1), "2", 2));
  TEST(!ini.GetEntryPtr("2", "a", 2, 1));
  TEST(fn(ini.GetEntryPtr("2", "b", 0, 1), "1", 1));
  TEST(!ini.GetEntryPtr("2", "b", 1, 1));
  TEST(!ini.GetEntryPtr("2", "a", 0, 2));
  TEST(!ini.GetEntryPtr("2", "b", 0, 2));
  TEST(ini.GetNumSections("1") == 1);
  TEST(ini.GetNumSections("2") == 3);
  TEST(ini.GetNumSections("3") == 0);
  TEST(ini.GetNumSections("") == 0);
  TEST(ini["1"].GetNumEntries("a") == 4);
  TEST(ini["1"].GetNumEntries("b") == 1);
  TEST(ini["1"].GetNumEntries("asdf") == 0);
  TEST(ini["1"].GetNumEntries("") == 0);

  ini.EndINIEdit();
  fn2("[1]\na=1\na=2\na=3\na=4\nb=1\nc=1\nc=2\nd=1\n\n[2]\na=1\na=2\nb=1\n\n[2]\na=1\na=2\nb=1\n\n[2]");

  int valid = 0;
  for(auto i = ini.begin(); i != ini.end(); ++i)
    for(auto j = i->begin(); j != i->end(); ++j)
      valid += j->IsValid();

  TEST(valid == 14);

  INI_E(1, a, 8, 3, 0); // Out of order to try and catch any bugs that might result from that
  INI_E(1, a, 6, 1, 0);
  INI_E(1, a, 7, 2, 0);
  INI_E(1, a, 5, 0, 0);
  INI_NE(1, a, 9, 4, 0);
  INI_E(1, b, 2, 0, 0);
  INI_NE(1, b, 9, 1, 0);
  INI_E(1, c, 3, 0, 0); // Normal in order attempt
  INI_E(1, c, 4, 1, 0);
  INI_NE(1, c, 9, 2, 0);
  INI_E(1, d, 2, 0, 0);
  INI_NE(1, d, 9, 1, 0);
  INI_E(2, a, 4, 1, 0); // out of order
  INI_E(2, a, 3, 0, 0);
  INI_NE(2, a, 9, 2, 0);
  INI_E(2, b, 2, 0, 0);
  INI_NE(2, b, 9, 1, 0);
  INI_E(2, a, 3, 0, 1); // in order
  INI_E(2, a, 4, 1, 1);
  INI_NE(2, a, 9, 2, 1);
  INI_E(2, b, 2, 0, 1);
  INI_NE(2, b, 9, 1, 1);
  INI_NE(2, a, 9, 0, 2);
  INI_NE(2, b, 9, 0, 2);

  TEST(fn(ini.GetEntryPtr("1", "a", 0, 0), "5", 5));
  TEST(fn(ini.GetEntryPtr("1", "a", 1, 0), "6", 6));
  TEST(fn(ini.GetEntryPtr("1", "a", 2, 0), "7", 7));
  TEST(fn(ini.GetEntryPtr("1", "a", 3, 0), "8", 8));
  TEST(!ini.GetEntryPtr("1", "a", 4, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "2", 2));
  TEST(!ini.GetEntryPtr("1", "b", 1, 0));
  TEST(fn(ini.GetEntryPtr("1", "c", 0, 0), "3", 3));
  TEST(fn(ini.GetEntryPtr("1", "c", 1, 0), "4", 4));
  TEST(!ini.GetEntryPtr("1", "c", 2, 0));
  TEST(fn(ini.GetEntryPtr("1", "d", 0, 0), "2", 2));
  TEST(!ini.GetEntryPtr("1", "d", 1, 0));
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 0), "3", 3));
  TEST(fn(ini.GetEntryPtr("2", "a", 1, 0), "4", 4));
  TEST(!ini.GetEntryPtr("2", "a", 2, 0));
  TEST(fn(ini.GetEntryPtr("2", "b", 0, 0), "2", 2));
  TEST(!ini.GetEntryPtr("2", "b", 1, 0));
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 1), "3", 3));
  TEST(fn(ini.GetEntryPtr("2", "a", 1, 1), "4", 4));
  TEST(!ini.GetEntryPtr("2", "a", 2, 1));
  TEST(fn(ini.GetEntryPtr("2", "b", 0, 1), "2", 2));
  TEST(!ini.GetEntryPtr("2", "b", 1, 1));
  TEST(!ini.GetEntryPtr("2", "a", 0, 2));
  TEST(!ini.GetEntryPtr("2", "b", 0, 2));

  ini.EndINIEdit();
  fn2("[1]\na=5\na=6\na=7\na=8\nb=2\nc=3\nc=4\nd=2\n\n[2]\na=3\na=4\nb=2\n\n[2]\na=3\na=4\nb=2\n\n[2]");

  INI_R(1, a, 1, 0);
  TEST(fn(ini.GetEntryPtr("1", "a", 0, 0), "5", 5));
  TEST(fn(ini.GetEntryPtr("1", "a", 1, 0), "7", 7));
  TEST(fn(ini.GetEntryPtr("1", "a", 2, 0), "8", 8));
  TEST(!ini.GetEntryPtr("1", "a", 3, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "2", 2));
  INI_R(1, a, 2, 0);
  TEST(fn(ini.GetEntryPtr("1", "a", 0, 0), "5", 5));
  TEST(fn(ini.GetEntryPtr("1", "a", 1, 0), "7", 7));
  TEST(!ini.GetEntryPtr("1", "a", 2, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "2", 2));
  INI_R(1, a, 0, 0);
  TEST(fn(ini.GetEntryPtr("1", "a", 0, 0), "7", 7));
  TEST(!ini.GetEntryPtr("1", "a", 1, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "2", 2));
  INI_R(1, c, 0, 0);
  TEST(fn(ini.GetEntryPtr("1", "c", 0, 0), "4", 4));
  INI_R(1, d, 0, 0);
  TEST(!ini.GetEntryPtr("1", "d", 0, 0));
  INI_R(1, a, 0, 0);
  TEST(!ini.GetEntryPtr("1", "a", 0, 0));
  TEST(fn(ini.GetEntryPtr("1", "b", 0, 0), "2", 2));

  INI_R(2, b, 0, 0);
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 0), "3", 3));
  TEST(fn(ini.GetEntryPtr("2", "a", 1, 0), "4", 4));
  TEST(!ini.GetEntryPtr("2", "a", 2, 0));
  TEST(!ini.GetEntryPtr("2", "b", 0, 0));
  INI_R(2, a, 1, 1);
  TEST(fn(ini.GetEntryPtr("2", "a", 0, 1), "3", 3));
  TEST(!ini.GetEntryPtr("2", "a", 1, 1));
  TEST(!ini.GetEntryPtr("2", "b", 1, 1));
  INI_R(2, a, 0, 1);
  TEST(!ini.GetEntryPtr("2", "a", 0, 1));
  TEST(!ini.GetEntryPtr("2", "b", 1, 1));
  INI_R(2, b, 0, 1);
  TEST(!ini.GetEntryPtr("2", "a", 0, 1));
  TEST(!ini.GetEntryPtr("2", "b", 0, 1));
  ini.RemoveSection("2", 0);
  TEST(fn4("2", 0)); // Catches index decrementing errors
  TEST(fn4("2", 1));
  ini.DEBUGTEST();
  ini.RemoveSection("2", 1);
  ini.DEBUGTEST();

  ini.EndINIEdit();
  ini.DEBUGTEST();
  fn2("[1]\nb=2\nc=4\n\n[2]\n\n");
  ini.DEBUGTEST();

  fn3();
  ini.EndINIEdit();
  fn2("[1]\nb=2\nc=4\na=1\na=2\na=3\na=4\nb=1\nc=1\nc=2\nd=1\n\n[2]\na=1\na=2\nb=1\n\n[1]\n\n[2]\na=1\na=2\nb=1\n\n[2]\n\n[2]");

  Str comp;
  for(auto i = ini.Front(); i != 0; i = i->next)
  {
    comp = comp + "\n[" + i->val.GetName() + ']';
    for(auto j = i->val.Front(); j != 0; j = j->next)
      comp = comp + '\n' + j->val.GetKey() + '=' + j->val.GetString();
  }

  // Due to organizational optimizations these come out in a slightly different order than in the INI, depending on when they were added.
  TEST(!strcmp(comp, "\n[1]\nb=2\nb=1\nc=4\nc=1\nc=2\na=1\na=2\na=3\na=4\nd=1\n[1]\n[2]\na=1\na=2\nb=1\n[2]\na=1\na=2\nb=1\n[2]\n[2]"));

  TEST(!remove("inistorage.ini"));
  ENDTEST;
}
