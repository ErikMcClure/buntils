// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HIGHPRECISIONTIMER_H__
#define __C_HIGHPRECISIONTIMER_H__

#include "bss_dlldef.h"

namespace bss_util
{
  class BSS_DLLEXPORT cHighPrecisionTimer //handy dandy highprecisiontimer class
  {
  public:
    cHighPrecisionTimer();
    double BSS_FASTCALL Update();
    double BSS_FASTCALL Update(double timewarp);
    double BSS_FASTCALL GetDelta() const;
    double BSS_FASTCALL GetTime() const;
    void BSS_FASTCALL ResetTime(); //resets the time to 0 (preserves delta)
    void BSS_FASTCALL ResetDelta(); //resets the delta to 0 by resampling

  protected:
    void BSS_FASTCALL _querytime(unsigned __int64* _pval);
    void BSS_FASTCALL _getaffinity();
    void BSS_FASTCALL _assigntime();

    unsigned __int64 _curTime;
    unsigned __int64 _freq;
    double _delta;
    double _time;
    void* _curprocess;

  private:
    __w64 unsigned long _procmask; //DWORD
    __w64 unsigned long _sysmask;
    void* _curthread; //this is private because it only reflects the thread that happened to call Update()
  };
}

#endif