// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss_log.h"
#include <sstream>
#include <fstream>

using namespace bss_util;

TESTDEF::RETPAIR test_bss_LOG()
{
  BEGINTEST;
  std::stringstream ss;
  std::fstream fs;
  std::wstringstream wss;
  fs.open(BSS__L("黑色球体工作室.log"));
  auto tf = [&](cLog& di) {
    ss.clear();
    fs.clear();
    wss.clear();
    //di.AddTarget(wss);

    di.GetStream() << BSS__L("黑色球体工作室");
    di.GetStream() << "Black Sphere Studios";
    di.ClearTargets();
    di.GetStream() << BSS__L("黑色球体工作室");
  };
  cLog a(BSS__L("黑色球体工作室.txt"), &ss); //Supposedly 黑色球体工作室 is Black Sphere Studios in Chinese, but the literal translation appears to be Black Ball Studio. Oh well.
  cLog b("logtest.txt");
  b.AddTarget(fs);
  cLog c;
  tf(a);
  tf(b);
  tf(c);
  cLog d(std::move(a));

  cLog lg("logtest2.txt");
  lg.FORMATLOG(0, "main.cpp", -1) << std::endl;
  lg.FORMATLOG(0, "main.cpp", 0) << std::endl;
  lg.FORMATLOG(0, __FILE__, 0) << std::endl;
  lg.FORMATLOG(0, __FILE__, __LINE__) << std::endl;
  lg.FORMATLOG(1, "main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(2, "\\main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(3, "/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(4, "a\\main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(5, "a/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(1, "asfsdbs dsfs ds/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(0, "asfsdbs dsfs ds\\main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(0, "asfsdbs\\dsfs/ds/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(0, "\\asfsdbs\\dsfs/ds/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(0, "\\asfsdbs/dsfs\\ds/main.cpp", __LINE__) << std::endl;
  lg.FORMATLOG(0, "\\asfsdbs/dsfs\\ds/main.cpp\\", __LINE__) << std::endl;
  lg.FORMATLOG(0, "\\asfsdbs/dsfs\\ds/main.cpp/", __LINE__) << std::endl;
  lg.WriteLog(0, "main.cpp", __LINE__, 0, "string", 1.0f, 35);
  BSSLOGV(lg, 3, 0, "string", 1.0f, 35);

  ENDTEST;
}