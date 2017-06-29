// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_TEST_H__
#define __BSS_TEST_H__

#include "bss-util/Hash.h"
#include "bss-util/Logger.h"
#include "bss-util/lockless.h"
#include "bss-util/Str.h"
#include "bss-util/RefCounter.h"

#define BSS_ENABLE_PROFILER

#ifdef BSS_PLATFORM_WIN32
#define BSS_PFUNC_PRE size_t BSS_COMPILER_STDCALL
#else
#define BSS_PFUNC_PRE void*
#endif

struct TESTDEF
{
  typedef std::pair<size_t, size_t> RETPAIR;
  const char* NAME;
  RETPAIR(*FUNC)();
};

const size_t TESTNUM = 50000;
extern uint16_t testnums[TESTNUM];
extern bss::Logger _failedtests;
extern volatile std::atomic<bool> startflag;

#define BEGINTEST TESTDEF::RETPAIR __testret(0,0); DEBUG_CDT_SAFE::_testret = &__testret; DEBUG_CDT_SAFE::Tracker.Clear();
#define ENDTEST return __testret
#define FAILEDTEST(t) BSSLOG(_failedtests,1, "Test #",__testret.first," Failed  < ",MAKESTRING(t)," >")
#define TEST(t) { bss::atomic_xadd(&__testret.first); try { if(t) bss::atomic_xadd(&__testret.second); else FAILEDTEST(t); } catch(...) { FAILEDTEST(t); } }
#define TESTERROR(t, e) { bss::atomic_xadd(&__testret.first); try { (t); FAILEDTEST(t); } catch(e) { bss::atomic_xadd(&__testret.second); } }
#define TESTERR(t) TESTERROR(t,...)
#define TESTNOERROR(t) { bss::atomic_xadd(&__testret.first); try { (t); bss::atomic_xadd(&__testret.second); } catch(...) { FAILEDTEST(t); } }
#define TESTARRAY(t,f) _ITERFUNC(__testret,t,[&](size_t i) -> bool { f });
#define TESTALL(t,f) _ITERALL(__testret,t,[&](size_t i) -> bool { f });
#define TESTCOUNT(c,t) { for(size_t i = 0; i < c; ++i) TEST(t) }
#define TESTCOUNTALL(c,t) { bool __val=true; for(size_t i = 0; i < c; ++i) __val=__val&&(t); TEST(__val); }
#define TESTFOUR(s,a,b,c,d) TEST(((s)[0]==(a)) && ((s)[1]==(b)) && ((s)[2]==(c)) && ((s)[3]==(d)))
#define TESTTWO(s,a,b) TEST(((s)[0]==(a)) && ((s)[1]==(b)))
#define TESTALLFOUR(s,a) TESTFOUR(s,a,a,a,a)
#define TESTRELFOUR(s,a,b,c,d) TEST(bss::fCompare((s)[0],(a)) && bss::fCompare((s)[1],(b)) && bss::fCompare((s)[2],(c)) && bss::fCompare((s)[3],(d)))

template<class T, size_t SIZE, class F>
void _ITERFUNC(TESTDEF::RETPAIR& __testret, T(&t)[SIZE], F f) { for(size_t i = 0; i < SIZE; ++i) TEST(f(i)) }
template<class T, size_t SIZE, class F>
void _ITERALL(TESTDEF::RETPAIR& __testret, T(&t)[SIZE], F f) { bool __val = true; for(size_t i = 0; i < SIZE; ++i) __val = __val && (f(i)); TEST(__val); }

struct DEBUG_CDT_SAFE
{
  DEBUG_CDT_SAFE(const DEBUG_CDT_SAFE& copy) : _safe(copy._safe), __testret(*_testret) { isdead = this; }
  explicit DEBUG_CDT_SAFE(bool safe) : _safe(safe), __testret(*_testret) { isdead = this; }
  ~DEBUG_CDT_SAFE() { if(_safe) TEST(isdead == this) }

  inline DEBUG_CDT_SAFE& operator=(const DEBUG_CDT_SAFE& right) { return *this; }

  static TESTDEF::RETPAIR* _testret;
  static int count;
  static bss::Hash<int> Tracker;
  static int ID;
  TESTDEF::RETPAIR& __testret;
  DEBUG_CDT_SAFE* isdead;
  const bool _safe;
};

template<bool SAFE = true>
struct DEBUG_CDT : DEBUG_CDT_SAFE {
  inline DEBUG_CDT(const DEBUG_CDT& copy) : DEBUG_CDT_SAFE(copy), _index(copy._index), _id(++ID) { ++count; assert(!Tracker.Exists(_id)); Tracker.Insert(_id); }
  inline DEBUG_CDT(DEBUG_CDT&& mov) : DEBUG_CDT_SAFE(mov), _index(mov._index), _id(mov._id) { mov._id = 0; }
  inline DEBUG_CDT(int index = 0) : DEBUG_CDT_SAFE(SAFE), _index(index), _id(++ID) { ++count; assert(!Tracker.Exists(_id)); Tracker.Insert(_id); }
  inline ~DEBUG_CDT() {
    if(_id != 0) //if id is zero a successful move was performed
    {
      TEST(Tracker.Exists(_id));
      DEBUG_CDT_SAFE::Tracker.Remove(_id);
      --count;
    }
  }

  inline DEBUG_CDT& operator=(DEBUG_CDT&& right) { _index = right._index; this->~DEBUG_CDT(); _id = right._id; right._id = 0; return *this; }
  inline DEBUG_CDT& operator=(const DEBUG_CDT& right) { _index = right._index; return *this; }
  inline bool operator<(const DEBUG_CDT& other) const { return _index<other._index; }
  inline bool operator>(const DEBUG_CDT& other) const { return _index>other._index; }
  inline bool operator<=(const DEBUG_CDT& other) const { return _index <= other._index; }
  inline bool operator>=(const DEBUG_CDT& other) const { return _index >= other._index; }
  inline bool operator==(const DEBUG_CDT& other) const { return _index == other._index; }
  inline bool operator!=(const DEBUG_CDT& other) const { return _index != other._index; }
  void donothing(float f) {}

  int _id;
  int _index;
};

// This defines an enormous list of pangrams for a ton of languages, used for text processing in an attempt to expose possible unicode errors.
extern const char* PANGRAM;
static const int NPANGRAMS = 29;
extern const bss::bsschar* PANGRAMS[NPANGRAMS];
extern const wchar_t* TESTUNICODESTR;

struct FWDTEST {
  FWDTEST() {}
  FWDTEST(const FWDTEST& copy) {}
  FWDTEST(FWDTEST&& mov) {}
  FWDTEST& operator=(const FWDTEST& right) { return *this; }
  FWDTEST& operator=(FWDTEST&& right) { return *this; }
};

struct RCounter : bss::RefCounter
{
  RCounter() {}
  ~RCounter() {}
};

TESTDEF::RETPAIR test_AA_TREE();
TESTDEF::RETPAIR test_ALIASTABLE();
TESTDEF::RETPAIR test_ANIMATION();
TESTDEF::RETPAIR test_FX_ANI();
TESTDEF::RETPAIR test_ARRAY();
TESTDEF::RETPAIR test_ARRAYCIRCULAR();
TESTDEF::RETPAIR test_ARRAYSORT();
TESTDEF::RETPAIR test_AVLTREE();
TESTDEF::RETPAIR test_BINARYHEAP();
TESTDEF::RETPAIR test_BITFIELD();
TESTDEF::RETPAIR test_BITSTREAM();
TESTDEF::RETPAIR test_COMPACTARRAY();
TESTDEF::RETPAIR test_bss_algo();
TESTDEF::RETPAIR test_bss_ALLOC_BLOCK();
TESTDEF::RETPAIR test_bss_ALLOC_BLOCK_LOCKLESS();
TESTDEF::RETPAIR test_bss_ALLOC_RING();
TESTDEF::RETPAIR test_bss_ALLOC_ADDITIVE();
TESTDEF::RETPAIR test_bss_deprecated();
TESTDEF::RETPAIR test_bss_GRAPH();
TESTDEF::RETPAIR test_bss_LOG();
TESTDEF::RETPAIR test_bss_SSE();
TESTDEF::RETPAIR test_bss_util();
TESTDEF::RETPAIR test_bss_util_c();
TESTDEF::RETPAIR test_VECTOR();
TESTDEF::RETPAIR test_DELEGATE();
TESTDEF::RETPAIR test_DISJOINTSET();
TESTDEF::RETPAIR test_bss_DUAL();
TESTDEF::RETPAIR test_DYNARRAY();
TESTDEF::RETPAIR test_bss_FIXEDPT();
TESTDEF::RETPAIR test_HASH();
TESTDEF::RETPAIR test_HIGHPRECISIONTIMER();
TESTDEF::RETPAIR test_IDHASH();
TESTDEF::RETPAIR test_INISTORAGE();
TESTDEF::RETPAIR test_JSON();
TESTDEF::RETPAIR test_KDTREE();
TESTDEF::RETPAIR test_LINKEDARRAY();
TESTDEF::RETPAIR test_LINKEDLIST();
TESTDEF::RETPAIR test_LOCKLESS();
TESTDEF::RETPAIR test_LOCKLESSQUEUE();
TESTDEF::RETPAIR test_MAP();
TESTDEF::RETPAIR test_OS();
TESTDEF::RETPAIR test_PRIORITYQUEUE();
TESTDEF::RETPAIR test_PROFILE();
TESTDEF::RETPAIR test_BSS_QUEUE();
TESTDEF::RETPAIR test_RATIONAL();
TESTDEF::RETPAIR test_REFCOUNTER();
TESTDEF::RETPAIR test_RWLOCK();
TESTDEF::RETPAIR test_SCHEDULER();
TESTDEF::RETPAIR test_Serializer();
TESTDEF::RETPAIR test_SINGLETON();
TESTDEF::RETPAIR test_SMARTPTR();
TESTDEF::RETPAIR test_BSS_STACK();
TESTDEF::RETPAIR test_STR();
TESTDEF::RETPAIR test_STREAM();
TESTDEF::RETPAIR test_STRTABLE();
TESTDEF::RETPAIR test_THREAD();
TESTDEF::RETPAIR test_THREADPOOL();
TESTDEF::RETPAIR test_TOML();
TESTDEF::RETPAIR test_TRBTREE();
TESTDEF::RETPAIR test_TRIE();
TESTDEF::RETPAIR test_UBJSON();
TESTDEF::RETPAIR test_VARIANT();
TESTDEF::RETPAIR test_XML();

#endif