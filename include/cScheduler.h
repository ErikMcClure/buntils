// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SCHEDULER_H__BSS__
#define __C_SCHEDULER_H__BSS__

#include "cHighPrecisionTimer.h"
#include "cBinaryHeap.h"

namespace bss_util
{
  // Scheduler object that lets you schedule events that happen x milliseconds into the future. If the event returns a number greater than 0,it will be rescheduled. 
  template<typename F, typename ST=unsigned int> //std::function<double(void)>
  class BSS_COMPILER_DLLEXPORT cScheduler : protected cHighPrecisionTimer, protected cBinaryHeap<std::pair<double,F>,ST,CompTFirst<std::pair<double,F>,CompTInv<double>>,cArraySafe<std::pair<double,F>,ST>>
  {
  public:
    // Constructor
    inline cScheduler() {}
    inline cScheduler(double t, const F& f) { Add(t,f); }
    inline cScheduler(double t, F&& f) { Add(t,std::move(f)); }
    inline ~cScheduler() {}
    // Gets number of events
    inline ST Length() const { return _length; }
    // Adds an event that will happen t milliseconds in the future, starting from the current time
    inline void Add(double t, const F& f) { Insert(std::pair<double,F>(t+_time,f)); }
    inline void Add(double t, F&& f) { Insert(std::pair<double,F>(t+_time,std::move(f))); }
    // Updates the scheduler, setting off any events that need to be set off
    inline void Update()
    {
      cHighPrecisionTimer::Update();
      while(GetRoot().first<=_time)
      {
        double r=GetRoot().second();
        if(r==0.0)
          Remove(0);
        else
          Set(0,std::pair<double,F>(r+_time,GetRoot().second));
      }
    }
  };
}

#endif