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
    typedef cBinaryHeap<std::pair<double,F>,ST,CompTFirst<std::pair<double,F>,CompTInv<double>>,cArraySafe<std::pair<double,F>,ST>> BASE;
  public:
    // Constructor
    inline cScheduler() {}
    inline cScheduler(double t, const F& f) { Add(t,f); }
    inline cScheduler(double t, F&& f) { Add(t,std::move(f)); }
    inline ~cScheduler() {}
    // Gets number of events
    BSS_FORCEINLINE ST Length() const { return BASE::_length; }
    // Adds an event that will happen t milliseconds in the future, starting from the current time
    BSS_FORCEINLINE void Add(double t, const F& f) { BASE::Insert(std::pair<double,F>(t+_time,f)); }
    BSS_FORCEINLINE void Add(double t, F&& f) { BASE::Insert(std::pair<double,F>(t+_time,std::move(f))); }
    // Updates the scheduler, setting off any events that need to be set off
    inline void Update()
    {
      cHighPrecisionTimer::Update();
      while(BASE::GetRoot().first<=_time)
      {
        double r=BASE::GetRoot().second();
        if(r==0.0)
          BASE::Remove(0);
        else
          BASE::Set(0,std::pair<double,F>(r+_time,BASE::GetRoot().second));
      }
    }
  };
}

#endif