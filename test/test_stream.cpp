// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/stream.h"

using namespace bun;

TESTDEF::RETPAIR test_STREAM()
{
  BEGINTEST;
  std::stringstream ss1;
  std::stringstream ss2;
  std::stringstream ss3;
  ss1 << "1 ";
  ss2 << "2 ";
  ss3 << "3 ";

  StreamSplitter splitter;
  std::ostream split(&splitter);
  splitter.AddTarget(&ss1);
  split << "a ";
  splitter.AddTarget(&ss2);
  split << "b ";
  splitter.AddTarget(&ss3);
  split << "c " << std::flush;
  splitter.ClearTargets();
  split << "d " << std::flush;
  splitter.AddTarget(&ss1);
  split << "e " << std::flush;

  TEST(ss1.str() == "1 a b c e ");
  TEST(ss2.str() == "2 b c ");
  TEST(ss3.str() == "3 c ");

  int acheck[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 50000, 50001, 50002, 50003, 50004, 50005, 6, 77777777, 88 };
  int bcheck[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 50000, 50001, 50002, 50003, 50004, 50005, 6, 77777777, 88 };
  DynArray<int> a = DynArray<int>(std::span<int>(acheck));
  TEST(a.Length() == sizeof(acheck)/sizeof(int));
  DynArray<short> b;
  DynArray<int> c;

  StreamBufArray<int> readbuf(a.begin(), a.Length());
  std::istream read(&readbuf);
  read.get();
  read.unget();
  read.read((char*)bcheck, sizeof(int)*2);
  read.get();
  read.unget();
  read.read((char*)(bcheck + 2), sizeof(bcheck) - sizeof(int)*2);
  TEST(!memcmp(acheck, bcheck, sizeof(bcheck)));

  StreamBufDynArray<int> idynbuf(a);
  std::istream idyn(&idynbuf);
  StreamBufDynArray<short> rwdynbuf(b);
  std::iostream rwdyn(&rwdynbuf);
  StreamBufDynArray<int> odynbuf(c);
  std::ostream odyn(&odynbuf);

  auto f = [](std::istream& in, std::ostream& out, int sz) {
    uint64_t sbuf;
    in.get();
    in.unget();
    while(in)
    {
      in.read((char*)&sbuf, sz);
      if(in.gcount() == sz)
        out.write((char*)&sbuf, sz);
      in.get();
      in.unget();
    }
  };

  f(idyn, rwdyn, sizeof(short));
  f(rwdyn, odyn, sizeof(int));

  TEST(a.Length()*2 == b.Length());
  TEST(b.Length() == c.Length()*2);
  if(a.Length() * 2 == b.Length())
    TEST(!memcmp(a.begin(), b.begin(), a.Length() * sizeof(int)));
  if(b.Length() == c.Length() * 2)
    TEST(!memcmp(b.begin(), c.begin(), b.Length() * sizeof(short)));

  DynArray<uint8_t> dbuf;
  {
    StreamBufArray<uint8_t> dynbuf(dbuf.begin(), dbuf.Length());
    std::istream dynstream(&dynbuf);
    TEST(dynstream.get() == -1);
    TEST(dynstream.eof());
  }
  dbuf.Add('h');
  {
    StreamBufArray<uint8_t> dynbuf(dbuf.begin(), dbuf.Length());
    std::istream dynstream(&dynbuf);
    TEST(dynstream.get() == 'h');
    dynstream.unget();
    TEST(dynstream.peek() == 'h');
    TEST(dynstream.peek() == 'h');
    TEST(dynstream.get() == 'h');
    TEST(dynstream.get() == -1);
    TEST(dynstream.eof());
  }
  dbuf.Add('e');
  dbuf.Add('l');
  dbuf.Add('l');
  dbuf.Add('o');
  {
    StreamBufArray<uint8_t> dynbuf(dbuf.begin(), dbuf.Length());
    std::istream dynstream(&dynbuf);
    char buf[8] = { 0 };
    dynstream.get(buf, 8);
    TEST(dynstream.gcount() == 5);
    TEST(!strcmp(buf, "hello"));
    TEST(dynstream.peek() == -1);
    TEST(dynstream.eof());
  }

  ENDTEST;
}//*/