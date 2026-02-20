// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_TEST_H__
#define __BUN_TEST_H__

#include "buntils/algo.h"
#include "buntils/Hash.h"
#include "buntils/lockless.h"
#include "buntils/Logger.h"
#include "buntils/RefCounter.h"
#include "buntils/Str.h"

#define BUN_ENABLE_PROFILER

#ifdef BUN_PLATFORM_WIN32
  #define BUN_PFUNC_PRE size_t BUN_COMPILER_STDCALL
#else
  #define BUN_PFUNC_PRE void*
#endif

struct TESTDEF
{
  using RETPAIR = std::pair<std::atomic_size_t, std::atomic_size_t>;
  const char* NAME;
  RETPAIR (*FUNC)();
};

extern void shuffle_testnums();

const size_t TESTNUM = 50000;
extern uint16_t testnums[TESTNUM];
extern bun::Logger _failedtests;
extern volatile std::atomic<bool> startflag;

#define BEGINTEST                        \
  TESTDEF::RETPAIR __testret{};          \
  DEBUG_CDT_SAFE::_testret = &__testret; \
  DEBUG_CDT_SAFE::Tracker.Clear();
#define ENDTEST \
  return TESTDEF::RETPAIR(__testret.first.load(std::memory_order_relaxed), __testret.second.load(std::memory_order_relaxed))
#define FAILEDTEST(t) \
  BUNLOG(_failedtests, 1, "Test #", __testret.first.load(std::memory_order_relaxed), " Failed  < ", TXT(t), " >")
#define TESTSTATIC(t) static_assert(t, "Failed: " TXT(t))
#ifdef __cpp_exceptions
  #define TEST(t)                        \
    {                                    \
      __testret.first.fetch_add(1);      \
      try                                \
      {                                  \
        if(t)                            \
          __testret.second.fetch_add(1); \
        else                             \
          FAILEDTEST(t);                 \
      }                                  \
      catch(...)                         \
      {                                  \
        FAILEDTEST(t);                   \
      }                                  \
    }
  #define TESTNOERROR(t)               \
    {                                  \
      __testret.first.fetch_add(1);    \
      try                              \
      {                                \
        (t);                           \
        __testret.second.fetch_add(1); \
      }                                \
      catch(...)                       \
      {                                \
        FAILEDTEST(t);                 \
      }                                \
    }
#else
  #define TEST(t)                        \
    {                                    \
      __testret.first.fetch_add(1);      \
      {                                  \
        if(t)                            \
          __testret.second.fetch_add(1); \
        else                             \
          FAILEDTEST(t);                 \
      }                                  \
    }
  #define TESTNOERROR(t)
#endif
#define TESTARRAY(t, f) _ITERFUNC(__testret, t, [&](size_t i) -> bool { f });
#define TESTALL(t, f)   _ITERALL(__testret, t, [&](size_t i) -> bool { f });
#define TESTCOUNT(c, t)           \
  {                               \
    for(size_t i = 0; i < c; ++i) \
      TEST(t)                     \
  }
#define TESTCOUNTALL(c, t)        \
  {                               \
    bool __val = true;            \
    for(size_t i = 0; i < c; ++i) \
      __val = __val && (t);       \
    TEST(__val);                  \
  }
#define TESTFOUR(s, a, b, c, d) TEST(((s)[0] == (a)) && ((s)[1] == (b)) && ((s)[2] == (c)) && ((s)[3] == (d)))
#define TESTTWO(s, a, b)        TEST(((s)[0] == (a)) && ((s)[1] == (b)))
#define TESTALLFOUR(s, a)       TESTFOUR(s, a, a, a, a)
#define TESTRELFOUR(s, a, b, c, d) \
  TEST(bun::fCompare((s)[0], (a)) && bun::fCompare((s)[1], (b)) && bun::fCompare((s)[2], (c)) && bun::fCompare((s)[3], (d)))

template<class T, size_t SIZE, class F> void _ITERFUNC(TESTDEF::RETPAIR& __testret, T (&t)[SIZE], F f)
{
  for(size_t i = 0; i < SIZE; ++i)
    TEST(f(i));
}
template<class T, size_t SIZE, class F> void _ITERALL(TESTDEF::RETPAIR& __testret, T (&t)[SIZE], F f)
{
  bool __val = true;
  for(size_t i = 0; i < SIZE; ++i)
    __val = __val && (f(i));
  TEST(__val);
}

struct DEBUG_CDT_SAFE
{
  DEBUG_CDT_SAFE(const DEBUG_CDT_SAFE& copy) : _safe(copy._safe), __testret(*_testret) { isdead = this; }
  explicit DEBUG_CDT_SAFE(bool safe) : _safe(safe), __testret(*_testret) { isdead = this; }
  ~DEBUG_CDT_SAFE()
  {
    if(_safe)
      TEST(isdead == this);
  }

  inline DEBUG_CDT_SAFE& operator=(const DEBUG_CDT_SAFE& right) { return *this; }

  static TESTDEF::RETPAIR* _testret;
  static int count;
  static bun::Hash<int> Tracker;
  static int ID;
  TESTDEF::RETPAIR& __testret;
  DEBUG_CDT_SAFE* isdead;
  const bool _safe;
};

template<bool SAFE = true> struct DEBUG_CDT : DEBUG_CDT_SAFE
{
  inline DEBUG_CDT(const DEBUG_CDT& copy) : DEBUG_CDT_SAFE(copy), _index(copy._index), _id(++ID)
  {
    ++count;
    assert(!Tracker.Exists(_id));
    Tracker.Insert(_id);
  }
  inline DEBUG_CDT(DEBUG_CDT&& mov) : DEBUG_CDT_SAFE(mov), _index(mov._index), _id(mov._id) { mov._id = 0; }
  inline DEBUG_CDT(int index = 0) : DEBUG_CDT_SAFE(SAFE), _index(index), _id(++ID)
  {
    ++count;
    assert(!Tracker.Exists(_id));
    Tracker.Insert(_id);
  }
  inline ~DEBUG_CDT()
  {
    if(_id != 0) // if id is zero a successful move was performed
    {
      TEST(Tracker.Exists(_id));
      DEBUG_CDT_SAFE::Tracker.Remove(_id);
      --count;
    }
  }

  inline DEBUG_CDT& operator=(DEBUG_CDT&& right)
  {
    _index = right._index;
    this->~DEBUG_CDT();
    _id       = right._id;
    right._id = 0;
    return *this;
  }
  inline DEBUG_CDT& operator=(const DEBUG_CDT& right)
  {
    _index = right._index;
    return *this;
  }
  inline std::strong_ordering operator<=>(const DEBUG_CDT& other) const { return _index <=> other._index; }
  inline bool operator==(const DEBUG_CDT& other) const { return _index == other._index; }
  void donothing(float f) {}

  int _id;
  int _index;
};

// This defines an enormous list of pangrams for a ton of languages, used for text processing in an attempt to expose
// possible unicode errors.
extern const char* PANGRAM;
static const int NPANGRAMS = 29;
extern const bun::bun_char* PANGRAMS[NPANGRAMS];
extern const wchar_t* TESTUNICODESTR;

struct FWDTEST
{
  FWDTEST() {}
  FWDTEST(const FWDTEST& copy) {}
  FWDTEST(FWDTEST&& mov) {}
  FWDTEST& operator=(const FWDTEST& right) { return *this; }
  FWDTEST& operator=(FWDTEST&& right) { return *this; }
};

struct RCounter : bun::RefCounter
{
  RCounter() {}
  ~RCounter() {}
};

TESTDEF::RETPAIR test_AA_TREE();
TESTDEF::RETPAIR test_ALIASTABLE();
TESTDEF::RETPAIR test_ANIMATION();
TESTDEF::RETPAIR test_ARRAY();
TESTDEF::RETPAIR test_ARRAYCIRCULAR();
TESTDEF::RETPAIR test_ARRAYSORT();
TESTDEF::RETPAIR test_AVLTREE();
TESTDEF::RETPAIR test_BINARYHEAP();
TESTDEF::RETPAIR test_BITFIELD();
TESTDEF::RETPAIR test_BITSTREAM();
TESTDEF::RETPAIR test_COMPACTARRAY();
TESTDEF::RETPAIR test_algo();
TESTDEF::RETPAIR test_ALLOC_BLOCK();
TESTDEF::RETPAIR test_ALLOC_BLOCK_LOCKLESS();
TESTDEF::RETPAIR test_ALLOC_CACHE();
TESTDEF::RETPAIR test_ALLOC_RING();
TESTDEF::RETPAIR test_ALLOC_GREEDY();
TESTDEF::RETPAIR test_ALLOC_GREEDY_BLOCK();
TESTDEF::RETPAIR test_deprecated();
TESTDEF::RETPAIR test_GRAPH();
TESTDEF::RETPAIR test_LOG();
TESTDEF::RETPAIR test_SSE();
TESTDEF::RETPAIR test_buntils();
TESTDEF::RETPAIR test_buntils_c();
TESTDEF::RETPAIR test_COLLISION();
TESTDEF::RETPAIR test_VECTOR();
TESTDEF::RETPAIR test_DELEGATE();
TESTDEF::RETPAIR test_DISJOINTSET();
TESTDEF::RETPAIR test_DUAL();
TESTDEF::RETPAIR test_DYNARRAY();
TESTDEF::RETPAIR test_FIXEDPT();
TESTDEF::RETPAIR test_GEOMETRY();
TESTDEF::RETPAIR test_HASH();
TESTDEF::RETPAIR test_HIGHPRECISIONTIMER();
TESTDEF::RETPAIR test_INISTORAGE();
TESTDEF::RETPAIR test_JSON();
TESTDEF::RETPAIR test_KDTREE();
TESTDEF::RETPAIR test_LINKEDARRAY();
TESTDEF::RETPAIR test_LINKEDLIST();
TESTDEF::RETPAIR test_LITERALS();
TESTDEF::RETPAIR test_LOCKLESS();
TESTDEF::RETPAIR test_LOCKLESSQUEUE();
TESTDEF::RETPAIR test_MAP();
TESTDEF::RETPAIR test_OS();
TESTDEF::RETPAIR test_PRIORITYQUEUE();
TESTDEF::RETPAIR test_PROFILE();
TESTDEF::RETPAIR test_BUN_QUEUE();
TESTDEF::RETPAIR test_RATIONAL();
TESTDEF::RETPAIR test_RANDOMQUEUE();
TESTDEF::RETPAIR test_REFCOUNTER();
TESTDEF::RETPAIR test_RWLOCK();
TESTDEF::RETPAIR test_SCHEDULER();
TESTDEF::RETPAIR test_Serializer();
TESTDEF::RETPAIR test_SINGLETON();
TESTDEF::RETPAIR test_BUN_STACK();
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