// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Logger.h"
#include <sstream>
#include <fstream>
#include "test.h"

using namespace bss;

TESTDEF::RETPAIR test_bss_LOG()
{
  BEGINTEST;
  std::stringstream ss;
  std::fstream fs;
  std::wstringstream wss;
  fs.open(BSS__L("黑色球体工作室.log"));
  auto tf = [&](Logger& di) {
    ss.clear();
    fs.clear();
    wss.clear();
    //di.AddTarget(wss);

    di.GetStream() << BSS__L("黑色球体工作室");
    di.GetStream() << "Black Sphere Studios";
    di.ClearTargets();
    di.GetStream() << BSS__L("黑色球体工作室");
  };
  Logger a(BSS__L("黑色球体工作室.txt"), &ss); //Supposedly 黑色球体工作室 is Black Sphere Studios in Chinese, but the literal translation appears to be Black Ball Studio. Oh well.
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
  BSSLOG(lg, 3, 0, "string", 1.0f, 35);
  lg.LogFormat("bss", "\\asfsdbs/dsfs\\ds/main.cpp/", __LINE__, 1, "{1}{0}{4}{{2}} {3}", 0, 1, 2, 3, 4);
  lg.PrintLog("bss2", "\\asfsdbs/dsfs\\ds/main.cpp\\", __LINE__, 0, "%s%i", "test", -28);
  ENDTEST;
}