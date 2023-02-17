// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Logger.h"
#include <sstream>
#include <fstream>

using namespace bun;

TESTDEF::RETPAIR test_LOG()
{
  BEGINTEST;
  std::stringstream ss;
  std::fstream fs;
  std::wstringstream wss;
  fs.open(BUN__L("兔子.log"));
  auto tf = [&](Logger& di) {
    ss.clear();
    fs.clear();
    wss.clear();
    //di.AddTarget(wss);

    di.GetStream() << "Erik McClure";
    di.ClearTargets();
  };
  Logger a(BUN__L("兔子.txt"), &ss);
  Logger b("logtest.txt");
  b.AddTarget(fs);
  Logger c;
  tf(a);
  tf(b);
  tf(c);
  Logger d(std::move(a));

  Logger lg("logtest2.txt");
  lg.LogHeader(0, "main.cpp", -1, 0) << std::endl;
  lg.LogHeader(0, "main.cpp", 0, 0) << std::endl;
  lg.LogHeader(0, __FILE__, 0, 0) << std::endl;
  lg.LogHeader(0, __FILE__, __LINE__, 0) << std::endl;
  lg.LogHeader(":::::", "main.cpp", __LINE__, 1) << std::endl;
  lg.LogHeader("a", "\\main.cpp", __LINE__, 2) << std::endl;
  lg.LogHeader("\\asfsdbs\\dsfs/ds/", "/main.cpp", __LINE__, 3) << std::endl;
  lg.LogHeader(0, "a\\main.cpp", __LINE__, 4) << std::endl;
  lg.LogHeader("", "a/main.cpp", __LINE__, 5) << std::endl;
  lg.LogHeader("source", "asfsdbs dsfs ds/main.cpp", __LINE__, 1) << std::endl;
  lg.LogHeader(nullptr, "asfsdbs dsfs ds\\main.cpp", __LINE__, -1) << std::endl;
  lg.LogHeader(0, "asfsdbs\\dsfs/ds/main.cpp", __LINE__, -2) << std::endl;
  lg.LogHeader(0, "\\asfsdbs\\dsfs/ds/main.cpp", __LINE__, 0) << std::endl;
  lg.LogHeader(0, "\\asfsdbs/dsfs\\ds/main.cpp", __LINE__, 0) << std::endl;
  lg.Log(0, "main.cpp", __LINE__, 0, 4, "string", 1.0f, 35);
  BUNLOG(lg, 3, 0, "string", 1.0f, 35);
  lg.LogFormat("bun", "\\asfsdbs/dsfs\\ds/main.cpp/", __LINE__, 1, "{1}{0}{4}{{2}} {3}", 0, 1, 2, 3, 4);
  lg.PrintLog("bun2", "\\asfsdbs/dsfs\\ds/main.cpp\\", __LINE__, 0, "%s%i", "test", -28);
  ENDTEST;
}