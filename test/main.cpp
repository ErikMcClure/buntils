// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

//#define __NO_UNIQUE_MODIFY__
#include "Shiny.h"
#include "bss_util.h"
#include "BSS_DebugInfo.h"
#include "bss_alloc_additive.h"
#include "bss_alloc_fixed.h"
#include "bss_deprecated.h"
#include "bss_fixedpt.h"
//#include "bss_sort.h"
#include "bss_win32_includes.h"
#include "cArrayCircular.h"
#include "cAVLtree.h"
#include "cBinaryHeap.h"
#include "cBitArray.h"
#include "cBitField.h"
#include "cBSS_Stack.h"
#include "cByteQueue.h"
#include "cCmdLineArgs.h"
#include "cDef.h"
#include "cHolder.h"
#include "cINIstorage.h"
#include "cKhash.h"
#include "cLambdaStack.h"
#include "cLinkedArray.h"
#include "cLinkedList.h"
#include "cLocklessByteQueue.h"
#include "cMap.h"
#include "cMutex.h"
#include "cObjSwap.h"
#include "cPriorityQueue.h"
#include "cRational.h"
#include "cRBT_List.h"
#include "cRefCounter.h"
//#include "cSettings.h"
#include "cSingleton.h"
#include "cSparseArray.h"
#include "cStr.h"
#include "cStrTable.h"
//#include "cTAATree.h"
#include "cTaskStack.h"
#include "cThread.h"
#include "cUniquePtr.h"
#include "functor.h"
//#include "leaktest.h"
#include "lockless.h"
#include "StreamSplitter.h"

#include <fstream>
#include <algorithm>
#include <time.h>

#define BOOST_FILESYSTEM_VERSION 3
//#define BOOST_ALL_NO_LIB
//#define BOOST_ALL_DYN_LINK
#define BOOST_ALL_NO_DEPRECATED
//#include <boost/filesystem.hpp>

#pragma warning(disable:4566)

using namespace bss_util;

struct TEST
{
  typedef std::pair<size_t,size_t> RETPAIR;
  const char* NAME;
  RETPAIR (*FUNC)();
};

#define BEGINTEST TEST::RETPAIR __testret(0,0);
#define ENDTEST return __testret
#define TEST(t) ++__testret.first; try { if(t) ++__testret.second; } catch(...) { }
#define TESTERROR(t, e) ++__testret.first; try { (t); } catch(e) { ++__testret.second; }
#define TESTERR(t) TESTERROR(t,...)
#define TESTNOERROR(t) ++__testret.first; try { (t); ++__testret.second; } catch(...) { }
#define TESTALL(t) for_all([&__testret](bool __x){ ++__testret.first; if(__x) ++__testret.second; },t);
#define TESTANY(t) { bool __val=true; for_all([&__testret,&__val](bool __x){ __val=__val&&__x; },t); TEST(__val); }

TEST::RETPAIR test_bss_util()
{
  BEGINTEST;
  TESTNOERROR(SetWorkDirToCur());
  char fbuf[MAX_PATH];
  GetModuleFileNameA(0,fbuf,MAX_PATH);
  TEST(bssFileSize(fbuf)!=0);
  TEST(bssFileSize(cStrW(fbuf))!=0);
  TESTNOERROR(GetTimeZoneMinutes());
  ENDTEST;
}

TEST::RETPAIR test_bss_DEBUGINFO()
{
  BEGINTEST;
  std::stringstream ss;
  std::wfstream fs;
  std::wstringstream wss;
  fs.open(L"黑色球体工作室.log");
  auto tf = [&](BSS_DebugInfo& di) {
    TEST(di.CloseProfiler(di.OpenProfiler()) < 500000);
    TEST(cStrW(di.GetModulePath(0)).compare(di.GetModulePathW(0))==0);
    TEST(di.GetProcMemInfo()!=0);
    TEST(di.GetWorkingSet()!=0);
    TESTNOERROR(di.ClearProfilers());
    di.OpenProfiler();
    TESTNOERROR(di.ClearProfilers());
    double d=di.GetDelta();
    double t=di.GetTime();

    ss.clear();
    fs.clear();
    wss.clear();
    di.AddTarget(wss);

    di.GetStream() << "黑色球体工作室";
    di.GetStream() << "Black Sphere Studios";
    di.ClearTargets();
    di.GetStream() << "黑色球体工作室";
  };

  BSS_DebugInfo a(L"黑色球体工作室.txt",&ss); //Supposedly 黑色球体工作室 is Black Sphere Studios in Chinese, but the literal translation appears to be Black Ball Studio. Oh well.
  BSS_DebugInfo b("logtest.txt");
  b.AddTarget(fs);
  BSS_DebugInfo c;
  tf(a);
  tf(b);
  tf(c);
  ENDTEST;
}

TEST::RETPAIR test_bss_algo()
{
  BEGINTEST;
  int a[9] = { -5,-1,0,1,6,8,9,26,39 };
  int v[17] = { -6,-5,-4,-2,-1,0,1,2,5,6,7,8,9,10,26,39,40 };
  int u[17] = { -1,0,0,0,1,2,3,3,3,4,4,5,6,6,7,8,8 };
  int u2[17] = { 0,0,1,1,1,2,3,4,4,4,5,5,6,7,7,8,9 };
  bool r[17] = {};
  for_all(r,[&a](int x, int y)->bool { return binsearch_before<int,uint,CompT<int>>(a,x)==y; },v,u);
  
  for_all(r,[&a](int x, int y)->bool { return binsearch_after<int,uint,CompT<int>>(a,x)==(lower_bound(std::begin(a),std::end(a),x)-a); },v,u2);
  for_all([&__testret](bool __x){ ++__testret.first; if(__x) ++__testret.second; },r);
  ENDTEST;
}

TEST::RETPAIR test_bss_ALLOC_ADDITIVE_FIXED()
{
  BEGINTEST;
  ENDTEST;
}
TEST::RETPAIR test_bss_ALLOC_ADDITIVE_VARIABLE()
{
  BEGINTEST;
  ENDTEST;
}
TEST::RETPAIR test_bss_ALLOC_FIXED_SIZE()
{
  BEGINTEST;
  ENDTEST;
}
TEST::RETPAIR test_bss_ALLOC_FIXED_CHUNK()
{
  BEGINTEST;
  ENDTEST;
}
TEST::RETPAIR test_bss_deprecated()
{
  BEGINTEST;
  __time64_t tmval=FTIME(NULL);
  TEST(tmval!=0);
  FTIME(&tmval);
  TEST(tmval!=0);
  tm tms;
  TEST([&]()->bool { GMTIMEFUNC(&tmval,&tms); return true; }())
  //TESTERR(GMTIMEFUNC(0,&tms));
  char buf[12];
//#define VSPRINTF(dest,length,format,list) _vsnprintf_s(dest,length,length,format,list)
//#define VSWPRINTF(dest,length,format,list) _vsnwprintf_s(dest,length,length,format,list)
  FILE* f=0;
  FOPEN(f,"__valtest.tmp","wb");
  TEST(f!=0);
  f=0;
  WFOPEN(f,L"石石石石shi.tmp",L"wb");
  TEST(f!=0);
  size_t a = 0;
  size_t b = -1;
  MEMCPY(&a,sizeof(size_t),&b,sizeof(size_t));
  TEST(a==b);
  a=0;
  MEMCPY(&a,sizeof(size_t)-1,&b,sizeof(size_t)-1);
  TEST(a==(b>>8));

#define STRNCPY(dst,size,src,count) strncpy_s(dst,size,src,count)
#define WCSNCPY(dst,size,src,count) wcsncpy_s(dst,size,src,count)
#define STRCPY(dst,size,src) strcpy_s(dst,size,src)
#define WCSCPY(dst,size,src) wcscpy_s(dst,size,src)
#define STRCPYx0(dst,src) strcpy_s(dst,src)
#define WCSCPYx0(dst,src) wcscpy_s(dst,src)
#define STRICMP(a,b) _stricmp(a,b)
#define WCSICMP(a,b) _wcsicmp(a,b)
#define STRTOK(str,delim,context) strtok_s(str,delim,context)
#define WCSTOK(str,delim,context) wcstok_s(str,delim,context)
#define SSCANF sscanf_s
#define ITOA(v,buf,r) _itoa_s(v,buf,r)
#define ITOA_S(v,buf,bufsize,r) _itoa_s(v,buf,bufsize,r)
  ENDTEST;
}

TEST::RETPAIR test_bss_FIXEDPT()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_ARRAY_CIRCULAR()
{
  BEGINTEST;
  ENDTEST;
}
TEST::RETPAIR test_AVLTREE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_BINARYHEAP()
{
  BEGINTEST;
  int a[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  int a3[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  int fill[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  int a2[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  const int a2_SZ=sizeof(a2)/sizeof(int);

  auto arrtest = [&](int* a, int* b, size_t count){
  bool __val=true;
  for_all([&__val](int x, int y){__val=__val&&(x==y);},count,a,b);
  TEST(__val);
  };

  //__insertion_sort<int,uint,CompT_LT<int>>(a2,sizeof(a2)/sizeof(int));
  std::sort(std::begin(a2),std::end(a2));
  cBinaryHeap<int,unsigned int, CompTInv<int>>::HeapSort(a3);
  arrtest(a2,a3,a2_SZ);

  //__insertion_sort<int,uint,CompT_GT<int>>(a2,sizeof(a2)/sizeof(int));
  std::sort(std::begin(a2),std::end(a2), [](int x, int y)->bool{ return x>y; });
  cBinaryHeap<int>::HeapSort(a3);
  arrtest(a2,a3,a2_SZ);

  std::vector<int> b;
  cBinaryHeap<int> c;
  for(uint i = 0; i < a2_SZ; ++i)
  {
    b.push_back(a[i]);
    std::push_heap(b.begin(),b.end(),[](const int& l, const int& r)->bool { return l>r; });
    c.Insert(a[i]);
    arrtest(&b[0],c,c.Length());
  }
  for(uint i = 0; i < a2_SZ; ++i)
  {
    std::pop_heap(b.begin(),b.end()-i,[](const int& l, const int& r)->bool { return l>r; });
    c.Remove(0);

    //for(uint j = 0; j < c.Length(); ++j)
    //  fill[j]=c[j]; //Let's us visualize C's array
    //for(uint j = 0; j < c.Length(); ++j)
    //  assert(c[j]==b[j]);
    arrtest(&b[0],c,c.Length());
  }
  ENDTEST;
}

TEST::RETPAIR test_bss_sort()
{
  BEGINTEST;
  ENDTEST;
}

int main(int argc, char** argv)
{
  TEST tests[] = {
    { "bss_util.h", &test_bss_util },
    { "bss_DebugInfo.h", &test_bss_DEBUGINFO },
    { "bss_alloc_additive.h", &test_bss_algo },
    { "bss_alloc_additive.h", &test_bss_ALLOC_ADDITIVE_FIXED },
    { "bss_alloc_additive.h", &test_bss_ALLOC_ADDITIVE_VARIABLE },
    { "bss_alloc_fixed.h", &test_bss_ALLOC_FIXED_SIZE },
    { "bss_alloc_fixed.h", &test_bss_ALLOC_FIXED_CHUNK },
    { "bss_depracated.h", &test_bss_deprecated },
    { "bss_fixedpt.h", &test_bss_FIXEDPT },
    { "cArrayCircular.h", &test_ARRAY_CIRCULAR },
    { "cAVLtree.h", &test_AVLTREE },
    { "cBinaryHeap.h", &test_BINARYHEAP },
  };

  const size_t NUMTESTS=sizeof(tests)/sizeof(TEST);

  std::cout << "Black Sphere Studios - Utility Library v." << (uint)BSSUTIL_VERSION.Major << '.' << (uint)BSSUTIL_VERSION.Minor << '.' <<
    (uint)BSSUTIL_VERSION.Revision << ": Unit Tests\nCopyright (c)2011 Black Sphere Studios\n" << std::endl;
  const int COLUMNS[3] = { 24, 11, 8 };
  printf("%-*s %-*s %-*s\n",COLUMNS[0],"Test Name", COLUMNS[1],"Subtests", COLUMNS[2],"Pass/Fail");

  TEST::RETPAIR numpassed;
  std::vector<uint> failures;
  for(uint i = 0; i < NUMTESTS; ++i)
  {
    numpassed=tests[i].FUNC(); //First is total, second is succeeded
    if(numpassed.first!=numpassed.second) failures.push_back(i);

    printf("%-*s %*s %-*s\n",COLUMNS[0],tests[i].NAME, COLUMNS[1],cStr("%u/%u",numpassed.second,numpassed.first).String(), COLUMNS[2],(numpassed.first==numpassed.second)?"PASS":"FAIL");
  }

  if(failures.empty())
    std::cout << "\nAll tests passed successfully!" << std::endl;
  else
  {
    std::cout << "\nThe following tests failed: " << std::endl;
    for (uint i = 0; i < failures.size(); i++)
      std::cout << "  " << tests[failures[i]].NAME << std::endl;
    std::cout << "\nThese failures indicate either a misconfiguration on your system, or a potential bug. Please report all bugs to http://code.google.com/p/bss-util/issues/list" << std::endl;
  }

  std::cout << "\nPress Enter to exit the program." << std::endl;
  cin.get();

}

void destroynode(std::pair<int,int>* data)
{
  delete data;
}

struct sp {
  int x;
  int y;
};

//char numarray[] = {3,4,9,14,15,19,28,37,47,50,54,56,59,61,70,73,78,81,92,95,97,99 };
char numarray[] = { 1, 2, 3, 4, 6 };

struct foobar// : public cClassAllocator<foobar>
{
  float test[3];
  void stupid(unsigned int dumbthing) { std::cout << "Stupid(): " << dumbthing << std::endl; }
  void retarded(int dumbthing, int stuff) { std::cout << "retarded(): " << dumbthing << " " << stuff << std::endl; }
  void idiotic(int dumbthing, int stuff, bool garbage) { std::cout << "idiotic(): " << dumbthing << " " << stuff << " " << garbage << std::endl; }
  void dumber() { std::cout << "dumber(): NULL" << std::endl; }
  void nothing() {}
  void nothing2() {}
  void nothing3() {}
};

//unsigned int __stdcall dorandomcrap(void* arg)
//{
//  cLocklessQueue* args = (cLocklessQueue*)arg;
//
//  FILE* f = fopen("Write.txt","w");
//  int id;
//  while(true)
//  {
//    id=rand()%1000;
//    memset(args->StartWrite(id),id,id);
//    args->FinishWrite();
//    fputs("wrote",f);
//    Sleep(1);
//  }
//  fclose(f);
//  return 0;
//}

/*
static const int NUM_RUNS=1;
static const int MAX_ALLOC=50;
unsigned int __stdcall dorandomcrap(void* arg)
{
  Sleep(1); //lets the other thread catch up
  cLocklessQueue* args = (cLocklessQueue*)arg;

  int id;
  for(int i = 0; i < NUM_RUNS; ++i)
  {
    id=(i<<3)%MAX_ALLOC;
    memset(args->StartWrite(id),id,id);
    args->FinishWrite();
  }
  memset(args->StartWrite(MAX_ALLOC+1),MAX_ALLOC+1,MAX_ALLOC+1);
  args->FinishWrite();
  return 0;
}

unsigned int __stdcall threadtest(void* arg)
{
  cThread* ptr=(cThread*)arg;
  while(!ptr->ThreadStop());
  return 0;
}

char* fliphold;

unsigned int __stdcall flippertest(void* arg)
{
  volatile cLocklessFlipper<char>* ptr = (cLocklessFlipper<char>*)arg;

  char* fliphold2;
  while(true)
  {
    fliphold2=((cLocklessFlipper<char>*)ptr)->StartRead();
    assert(fliphold!=fliphold2);
    ((cLocklessFlipper<char>*)ptr)->EndRead();
  }
}
*/

int PI_ITERATIONS=500;
double pi=((PI_ITERATIONS<<1)-1)+(PI_ITERATIONS*PI_ITERATIONS);

const char* MBSTESTSTRINGS[] = { "test","test2","test3","test4","test5","test6" };
const wchar_t* WCSTESTSTRINGS[] = { L"test",L"test2",L"test3",L"test4",L"test5",L"test6" };

const int TESTNUM=500000;

struct weird
{
  void* p1;
  __int64 i;
  short offset;
  char blah;
  inline bool valid() { return i==-1 && offset == -1 && blah == -1; }
  inline bool invalid() { return i==0 && offset == 0 && blah == 0; }
  inline void invalidate() { i=0; offset=0;blah=0; }
  inline void validate() { i=-1; offset=-1;blah=-1; }
};

void printout(cLinkedList<int,Allocator<cLLNode<int>>,true>& list)
{
  LLIterator<cLLNode<int>> cur(list.GetRoot());

  while(cur.HasNext())
    std::cout<<(++cur)->item;

  std::cout<<std::endl;
}

#define INSTANTIATE_SETTINGS
#include "cSettings.h"

DECL_SETGROUP(0,"main",3);
DECL_SETTING(0,0,float,0.0f,"ANIME",0);
DECL_SETTING(0,1,int,0,"MANGA","-manga");
DECL_SETTING(0,2,double,0.0,"pow",0);
DECL_SETGROUP(1,"submain",2);
DECL_SETTING(1,0,float,15.0f,"zip",0);
DECL_SETTING(1,1,int,5,"poofers",0);
DECL_SETTING(1,2,std::vector<cStr>,std::vector<cStr>(),"lots",0);

struct DEBUG_CDT {
  inline DEBUG_CDT(const DEBUG_CDT& copy) : _index(copy._index) { ++count; isdead=this; }
  inline DEBUG_CDT(int index=0) : _index(index) { ++count; isdead=this; }
  inline ~DEBUG_CDT() { if(isdead!=this) count/=0; --count; isdead=0; }

  inline DEBUG_CDT& operator=(const DEBUG_CDT& right) { _index=right._index; return *this; }
  inline bool operator<(const DEBUG_CDT& other) const { return _index<other._index; }
  inline bool operator>(const DEBUG_CDT& other) const { return _index>other._index; }
  inline bool operator<=(const DEBUG_CDT& other) const { return _index<=other._index; }
  inline bool operator>=(const DEBUG_CDT& other) const { return _index>=other._index; }
  inline bool operator==(const DEBUG_CDT& other) const { return _index==other._index; }
  inline bool operator!=(const DEBUG_CDT& other) const { return _index!=other._index; }

  static int count;
  DEBUG_CDT* isdead;
  int _index;
};
int DEBUG_CDT::count=0;

const char* FAKESTRINGLIST[5] = { "FOO", "BAR", "MEH", "SILLY", "EXACERBATION" };

int main3(int argc, char** argv)
{  
  //cRational<int> tr(1,10);
  //cRational<int> tr2(1,11);
  //cRational<int> tr3(tr+tr2);
  //tr3=(tr-tr2);
  //tr3=(tr*tr2);
  //tr3=(tr/tr2);
  //tr3=(tr+3);
  //tr3=(tr-3);
  //tr3=(tr*3);
  //tr3=(tr/3);
  //bool ttb=tr<3;
  //ttb=tr>3;
  //ttb=tr<tr2;
  //ttb=tr>tr2;
  //ttb=tr==3;
  //ttb=tr!=3;
  //ttb=tr==tr2;
  //ttb=tr!=tr2;

  //bool chk = fsmall(1);
  //bool chk2 = dsmall(-1/900000000000000.0);

  //{
  //cKhash<int, char,false,KH_INT_HASHFUNC,KH_INT_EQUALFUNC<int>,KH_INT_VALIDATEPTR<int>> hashtest;
  //hashtest.Insert(21354,0);
  //hashtest.Insert(34623,0);
  //hashtest.Insert(52,0);
  //hashtest.Insert(1,0);
  //int r=hashtest.GetIterKey(hashtest.GetIterator(1));

  //}
  //return 0;
  cStr sdfhold("blah");
  cStr sdfderp(sdfhold+cStr("temp")+cStr("temp")+cStr("temp")+cStr("temp"));

  //_controlfp( _PC_24, MCW_PC );
  int zsdf = fFastRound(2734.82f);

  unsigned int seed=(unsigned int)GetTickCount();
  srand(seed);
  //srand(433690314);
  rand();
  
  cAdditiveVariableAllocator<64> _variabletest;
  std::vector<char*> phold;
  for(int j=0; j<1000; ++j)
  {
    for(int i = 0; i < 1000; ++i)
    {
      if(RANDINTGEN(0,10)<5 || phold.size()<3)
      {
        char* test=_variabletest.alloc<char>(RANDINTGEN(4,1000));
        *test=0xDEADBEEF;
        phold.push_back(test);
      }
      else
      {
        int index=RANDINTGEN(0,phold.size()-1);
        _variabletest.dealloc(phold[index]);
        phold.erase(phold.begin()+index);
      }
    }
    _variabletest.Clear();
  }
  char prof;
  BSS_DebugInfo _debug("log.txt");
//  BSSLOGONE(&_debug,1,L"Test こんにちは世界 Log %s",L"こんにちは世界");
  
  /*unsigned int avg[2];
  avg[0]=0;
  avg[1]=0;

  for(int j=1; j<100; ++j)
  {
    Allocator<int,AdditiveFixedPolicy<int>> _fixedtest;
  std::vector<int*> phold;

    prof=_debug.OpenProfiler();
  for(int i = 0; i < 100000; ++i)
  {
    if(RANDINTGEN(0,10)<5 || phold.size()<3)
    {
      int* test=_fixedtest.allocate(1);
      *test=0xDEADBEEF;
      phold.push_back(test);
    }
    else
    {
      int index=RANDINTGEN(0,phold.size()-1);
      _fixedtest.deallocate(phold[index]);
      phold.erase(phold.begin()+index);
    }
  }
    std::cout << ((avg[0]+=_debug.CloseProfiler(prof))/j) << std::endl;

    phold.clear();
  Allocator<int> _normaltest;
    prof=_debug.OpenProfiler();
  for(int i = 0; i < 100000; ++i)
  {
    if(RANDINTGEN(0,10)<5 || phold.size()<3)
    {
      int* test=_normaltest.allocate(1);
      *test=0xDEADBEEF;
      phold.push_back(test);
    }
    else
    {
      int index=RANDINTGEN(0,phold.size()-1);
      _normaltest.deallocate(phold[index]);
      phold.erase(phold.begin()+index);
    }
  }
    std::cout << ((avg[1]+=_debug.CloseProfiler(prof))/j) << std::endl;

  }*/

  //FixedPt<12> fp(23563.2739);
  //double res=fp;
  //fp+=27.9;
  //res+=27.9;
  //res-=fp;
  //res=fp;
  //fp-=8327.9398437;
  //res-=8327.9398437;
  //res-=fp;
  //res=fp;
  //fp*=6.847399;
  //res*=6.847399;
  //res-=fp;
  //res=fp;
  //fp/=748.9272;
  //res/=748.9272;
  //res-=fp;

  unsigned char vn2 = GetBitMask<unsigned char>(4); // 0001 0000
  unsigned char vn3 = GetBitMask<unsigned char>(2,4); // 0001 1100
  unsigned char vn4 = GetBitMask<unsigned char>(-2,2); // 1100 0111
  unsigned char vn5 = GetBitMask<unsigned char>(-2,-2); // 0100 0000

  cArrayWrap<cArraySimple<int,unsigned char>> artst(5);
  int v = artst[3];

  {
  std::stringbuf unitbuf;
  std::ostream unit(&unitbuf,true);
  bss_Log log("test.txt");
  log.AddTarget(unit);
  log.GetStream() << "normal";
  log.GetStream() << L"これはサンプルテキストです。";
  log.GetStream() << 5;
  log.GetStream() << 2.0f;

  BSSLOG(log,1) << 1 << std::endl  << 2 << std::endl  << 3 << std::endl  << 4 << std::endl  << 5 << std::endl  << 6 << std::endl; 
  BSSLOG(log,1) << "fail" << 2 << L"これはサン" << 2987324.387453 << 0xFF << "aslkj slkdfjasld kfjsadlkfj sks ss           " << "" << std::endl << std::endl << "";
  }

  int c = STRSWAP("bar",FAKESTRINGLIST);

  {
    cArraySort<DEBUG_CDT,CompT<DEBUG_CDT>,unsigned int,cArraySafe<DEBUG_CDT,unsigned int>> arrtest;
  arrtest.Insert(DEBUG_CDT(0));
  arrtest.Insert(DEBUG_CDT(1));
  arrtest.Insert(DEBUG_CDT(2));
  arrtest.Remove(2);
  arrtest.Insert(DEBUG_CDT(3));
  arrtest.Insert(DEBUG_CDT(4));
  arrtest.Insert(DEBUG_CDT(5));
  arrtest.Remove(0);
  arrtest.Insert(DEBUG_CDT(6));
  arrtest.Remove(3);
  cArraySort<DEBUG_CDT,CompT<DEBUG_CDT>,unsigned int,cArraySafe<DEBUG_CDT,unsigned int>> arrtest2;
  
  for(uint i = 0; i < arrtest.Length(); ++i)
    std::cout << arrtest[i]._index << std::endl;

  arrtest2.Insert(DEBUG_CDT(7));
  arrtest2.Insert(DEBUG_CDT(8));
  arrtest=arrtest2;
  }

  cCmdLineArgsA cmdtest(argc,argv);

  cSettingManage<1,0>::LoadAllFromINI(cINIstorage<char>("test.ini"));
  cSettingManage<1,0>::SaveAllToINI(cINIstorage<char>("test.ini"));
  //{
  //cArrayConstruct<CreateDestroyTracker> ac1(1);
  //cArrayConstruct<CreateDestroyTracker> ac2(0);
  //cArrayConstruct<CreateDestroyTracker> ac3(ac1+ac2);
  //ac3+=ac1;
  //ac2+=ac3;
  //ac1+ac3;

  //}
  //assert(CreateDestroyTracker::count==0);

  //{
  //cLinkedList<int,Allocator<cLLNode<int>>,true> test;
  //cLLNode<int>* llp[5];

  //llp[0] = test.Add(1);
  //printout(test);
  //llp[1] = test.Add(2);
  //printout(test);
  //llp[3] = test.Add(4);
  //printout(test);
  //llp[4] = test.Add(5);
  //printout(test);
  //llp[2] = test.Insert(3,llp[3]);
  //printout(test);
  //test.Remove(llp[3]);
  //printout(test);
  //test.Remove(llp[0]);
  //printout(test);
  //test.Remove(llp[4]);
  //printout(test);
  //test.Insert(0,0);
  //printout(test);
  //test.Length();
  //}
  //return 0;

  //{
  //  Allocator<weird,FixedChunkPolicy<weird>> _fixedtest;
  //  std::vector<weird*> _allocs;
  //  weird* p;
  //  for(int i=1; i<10000000; ++i)
  //  {
  //    if(RANDINTGEN(0,2)!=0 || _allocs.size()<1000) {
  //      p = _fixedtest.allocate(1);
  //      if(!p->valid())
  //        throw "fuuuuuuuuuuuuuuuuuuu";
  //      p->invalidate();
  //      _allocs.push_back(p);
  //    } else {
  //      unsigned int target = RANDINTGEN(0,_allocs.size());
  //      p = _allocs[target];
  //      if(!p->invalid())
  //        throw "fuuuuuuuuuuuuuuuuuuu";
  //      p->validate();
  //      _fixedtest.deallocate(p);
  //      if(target<_allocs.size()-1)
  //      {
  //        p = _allocs[_allocs.size()-1];
  //        _allocs[target]=p;
  //      }
  //      _allocs.pop_back();
  //    }
  //  }
  //}

  std::cout << DEBUG_CDT::count;
  std::cout << std::endl << std::endl << "Press Enter to continue" << std::endl;
  std::cin.get();
  return 0;

  int testnums[TESTNUM];
  for(int i = 0; i<TESTNUM; ++i)
    testnums[i]=i;

  shuffle(testnums);
  /*
  Allocator<AVL_Node<int,int>,FixedChunkPolicy<AVL_Node<int,int>>> fixedavl;
  cAVLtree<int, int,CompareKeys<int>,Allocator<AVL_Node<int,int>,FixedChunkPolicy<AVL_Node<int,int>>>> avlblah(&fixedavl);

  prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Insert(testnums[i],testnums[i]);
  std::cout << _debug.CloseProfiler(prof) << std::endl;

  shuffle(testnums);
  prof=_debug.OpenProfiler();

  for(int i = 0; i<TESTNUM; ++i)
  {
  _ReadWriteBarrier();
    seed+=avlblah.Get(testnums[i]);
  _ReadWriteBarrier();
  }

  std::cout << _debug.CloseProfiler(prof) << std::endl;
  
  shuffle(testnums);
  prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Remove(testnums[i]);
  std::cout << _debug.CloseProfiler(prof) << std::endl;

  std::cin >> seed;
  return 0;
  */

  Allocator<cRBT_Node<int,int>,FixedChunkPolicy<cRBT_Node<int,int>>> fixedalloc;
  cRBT_List<int, int,CompT<int>,Allocator<cRBT_Node<int,int>,FixedChunkPolicy<cRBT_Node<int,int>>>> blah(&fixedalloc);

  prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    //blah.Insert(testnums[i],testnums[i]);
    blah.Insert((i&0x01)?5:testnums[i],testnums[i]);
  std::cout << _debug.CloseProfiler(prof) << std::endl;

  shuffle(testnums);
  prof=_debug.OpenProfiler();
  
  for(int i = 0; i<TESTNUM; ++i)
  {
  _ReadWriteBarrier();
    //seed+=blah.Get(testnums[i])->_data;
    seed+=blah.Get(5)->_data;
  _ReadWriteBarrier();
  }

  std::cout << _debug.CloseProfiler(prof) << std::endl;

  //cRBT_PNode<int,int>* pnode=blah.GetFirst();
  //int last=-1;
  //while(pnode)
  //{
  //  if(pnode->GetData()<=last)
  //    throw "";
  //  last=pnode->GetData();
  //  pnode=pnode->GetNext();
  //}

  shuffle(testnums);
  prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    blah.Remove(testnums[i]);
    //blah.Remove(5);
  std::cout << _debug.CloseProfiler(prof) << std::endl;

  unsigned __int16 s4=RANDINTGEN(90000,90000000000);

  double y = FastSqrt(9.0);

  std::cout << seed;
  std::cin >> s4;
  return 0;

  sp S1 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };
  sp S2 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };
  sp S3 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };
  sp S4 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };
  sp S5 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };
  sp S6 = { RANDFLOATGEN(0.0f,100000.0f), RANDFLOATGEN(0.0f,100000.0f) };

  _ReadWriteBarrier();

  float x = distsqr(S3.x,S3.y,S4.x,S4.y);
  
  _ReadWriteBarrier();


  _ReadWriteBarrier();
  
  std::cout << x << y;

  //char* romanstuff = inttoroman(3333);

  cStrTable<wchar_t> mbstable(WCSTESTSTRINGS,6);
  cStrTable<wchar_t> wcstable(WCSTESTSTRINGS,6);
  cStrTable<wchar_t,char> mbstable2(WCSTESTSTRINGS,6);
  
  /*const wchar_t* stv = mbstable.GetString(0);
  stv = mbstable.GetString(4);
  mbstable+=mbstable2;
  stv = mbstable.GetString(4);
  stv = mbstable.GetString(8);
  std::fstream fs;
  fs.open("dump.txt",std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
  mbstable.DumpToStream(&fs);
  fs.close();
  fs.open("dump.txt",std::ios_base::in | std::ios_base::binary);
  cStrTable<wchar_t> ldtable(&fs,bssFileSize("dump.txt"));
  stv = ldtable.GetString(4);
  stv = ldtable.GetString(11);
  return 0;*/

  //srand(433005251);
  //int length=sizeof(string)-1;
  //cStr cur;
  //for(int i = length/2; i > 3; --i)
  //{
  //  if(cur.length()>i) break; //we already found it
  //  for(int j = 0; j<length; ++j)
  //  {
  //    if(backwardscheck(&string[j],i))
  //    {
  //      cur.reserve(i+1);
  //      cur.UnsafeString()[i]='\0';
  //      strncpy(cur.UnsafeString(),&string[j],i);
  //      cur.RecalcSize();
  //      break; //we don't care if there are any others
  //    }
  //  }
  //}

  //cTAATree<int,int> _aatest;

  //for(int i = 0; i < 100000; ++i)
  //{
  //  _aatest.Insert(rand(),rand());
  //  
  //  const cTAATree<int,int>::TNODE* hold=_aatest.GetFirst();
  //  while(hold)
  //  {
  //    if(hold->next!=0)
  //      assert(hold->key<=hold->next->key);
  //    hold=hold->next;
  //  }
  //}

  //int count=0;
  //const cTAATree<int,int>::TNODE* hold=_aatest.GetFirst();
  //while(hold)
  //{
  //  if(hold->next!=0)
  //    assert(hold->key<=hold->next->key);
  //  hold=hold->next;
  //  ++count;
  //}
  //int prev=0;
  //int cur=1;
  //int res=0;
  //int target=227000;
  //while(cur<=target || !isprime(cur))
  //{
  //  res=cur;
  //  cur+=prev;
  //  prev=res;
  ////}
  unsigned int* zp=&seed;
  unsigned int* zp2=&seed;
  unsigned int* zp3=&seed;
  unsigned int* zp4=0;
  unsigned int* zp5=&seed;
  switch(PSWAP(0,5,zp,zp2,zp3,zp4,zp5))
  {
  case 0:
    seed=seed;
    break;
  case 1:
    seed=seed;
    break;
  case 2:
    seed=seed;
    break;
  case 3:
    seed=seed;
    break;
  case 4:
    seed=seed;
    break;
  }

  switch(STRSWAP("nothing",5,"no","yes","lol","help","fail"))
  {
  case 0:
    seed=seed;
    break;
  case 1:
    seed=seed;
    break;
  case 2:
    seed=seed;
    break;
  case 3:
    seed=seed;
    break;
  case 4:
    seed=seed;
    break;
  }

  //return 0;

  system("Pause");
  {
    cLinkedArray<__int64> tarrlink(2000);
    std::vector<unsigned int> _rm;
    for(int j=0; j<100000;  ++j)
      tarrlink.Add(1);

    char prof=_debug.OpenProfiler();
    unsigned int hold=0;
    unsigned int cur=tarrlink.Start();
    while(cur!=(unsigned int)-1)
    {
      hold+=tarrlink[cur];
      tarrlink.Next(cur);
    }

    //for(int j=0; j<10000; ++j)
    //{
    //  for(int i = RANDINTGEN(100,200); i>0; --i) {
    //    if(_rm.size()<2)
    //      _rm.push_back(tarrlink.Add(0));
    //    else
    //      _rm.push_back(tarrlink.InsertAfter(i,_rm[RANDINTGEN(0,_rm.size()-1)]));
    //  }
    //  for(int i = RANDINTGEN(0,tarrlink.Size()-1); i>0; --i){
    //    if(_rm.size()>2) {
    //      int check=RANDINTGEN(0,_rm.size()-1);
    //      tarrlink.Remove(_rm[check]);
    //      _rm.erase(_rm.begin()+check);
    //    }
    //  }
    //}
    std::cout << _debug.CloseProfiler(prof) << std::endl;
  }
  
  srand(seed); //ensures we get the same numbers
  {
    cLinkedList<__int64*> tarrlink;
    std::vector<cLLNode<__int64*>*> _rm;
    __int64 i=1;
    for(int j=0; j<100000; ++j)
      tarrlink.Add(&i);
    
    char prof=_debug.OpenProfiler();
    unsigned int hold=0;
    cLLNode<__int64*>* cur=tarrlink.GetRoot();
    while(cur!=0)
    {
      hold+=*cur->item;
      cur=cur->next;
    }

    //for(int j=0; j<10000; ++j)
    //{
    //  for(__int64 i = RANDINTGEN(100,200); i>0; --i) {
    //    if(_rm.size()<2)
    //      _rm.push_back(tarrlink.Add(0));
    //    else
    //      _rm.push_back(tarrlink.InsertAfter(&i,_rm[RANDINTGEN(0,_rm.size()-1)]));
    //  }
    //  for(int i = RANDINTGEN(0,tarrlink.Length()-1); i>0; --i){
    //    if(_rm.size()>2) {
    //      int check=RANDINTGEN(0,_rm.size()-1);
    //      tarrlink.Remove(_rm[check]);
    //      _rm.erase(_rm.begin()+check);
    //    }
    //  }
    //}

    std::cout << _debug.CloseProfiler(prof) << std::endl;
  }
  
  //std::vector<int> prime_divisors;

  //int check=2;
  //int newnum=cur+1;
  //while(check*check<=newnum)
  //{
  //  if (isprime(check) && newnum % check == 0)           
  //  {
  //    prime_divisors.push_back(check);
  //  }
  //  ++check;
  //}
  //
  //int sum=0;
  //for(int i = 0; i < prime_divisors.size(); ++i) sum+=prime_divisors[i];

  int input;
  std::cout << "Start sequence at what number? " << std::endl;
  std::cin >> input;
  char value=(char)input;
  int exact=value;
  int exactbefore=bssmax((int)(value*0.618f),1);

  while(value < 100)
  {    
    exact+=exactbefore;
    exactbefore=exact-exactbefore;

    std::cout << (int)(value=fbnext(value)) << "   (" << (exact-value) << ")" << std::endl;
  }

  system("Pause");

  //boost::filesystem::path p("../");
  //if (boost::filesystem::exists(p, boost::system::error_code()))    // does p actually exist?
  //  {
  //    if (boost::filesystem::is_regular_file(p))        // is p a regular file?
  //      std::cout << p << " size is " << boost::filesystem::file_size(p) << '\n';

  //    else if (boost::filesystem::is_directory(p))      // is p a directory?
  //    {
  //      std::cout << p << " is a directory containing:\n";

  //      copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(),  // directory_iterator::value_type
  //        std::ostream_iterator<boost::filesystem::directory_entry>(std::cout, "\n"));  // is directory_entry, which is
  //                                                         // converted to a path by the
  //                                                         // path stream inserter
  //    }
  //    else
  //      std::cout << p << " exists, but is neither a regular file nor a directory\n";
  //  }
  //  else
  //    std::cout << p << " does not exist\n";

  char buf[260];
  //FileDialog(buf, false, 0, "test.bak");

  char arraysize=sizeof(numarray)/sizeof(char);
  int target;
  int add;
  int end;
  int sets=0;
  for(char i = 2; i < arraysize; ++i)
  {
    for(int j=0; j+i<arraysize; ++j)
    {
      target=numarray[j+i];
      end=j+i;
      add=0;
      for(int k=end-1; k>=j; --k) {
        add+=numarray[k];
        if(add>target) break;
      }
      if(add==target) ++sets;
    }
  }
  
  cINIstorage<wchar_t> wini(L"test.ini");
  wini.EditEntry(L"Video",L"test",L"fail",-1,-1);
  wini.EditEntry(L"Video",L"test",L"success");
  wini.EndINIEdit();

  SetWorkDirToCur();
  cINIstorage<char> ini("test.ini");
  while(ini.RemoveSection("NEWSECTION"));
  ini.AddSection("NEWSECTION");
  ini.AddSection("NEWSECTION");
  ini.EditEntry("NEWSECTION","newkey","fakevalue",0,-1);
  ini.EditEntry("NEWSECTION","newkey","realvalue",0,0);
  ini.EditEntry("NEWSECTION","newkey",0,0,0);
  ini.EditEntry("NEWSECTION","newkey","value",-1,-1);
  const char* test = ini["Video"]["baseheight"].GetString();

  cStrW testerstr("test");
  cStr tester2str("%s%i","test",2);
  cStrW wtester2str(L"%s%i",L"test",2);
  wtester2str+=tester2str;

  const std::vector<std::pair<cStr,unsigned int>>& sections=ini.BuildSectionList();
  ini.EndINIEdit();

  for(unsigned int i=0; i < sections.size(); ++i)
  {
    std::cout << '[' << sections[i].first << ':' << sections[i].second  << ']' << std::endl;
    const std::vector<std::pair<std::pair<cStr,cStr>,unsigned int>>& entries=ini.GetSection(sections[i].first,sections[i].second)->BuildEntryList();
    for(unsigned int j=0; j < entries.size(); ++j)
      std::cout << entries[j].first.first << " - " << entries[j].first.second << " :" << entries[j].second << std::endl;
    std::cout << std::endl;
  }
  system("Pause");
  return 0;

  cKhash_Int64<void*> testkhash;

  system("Pause");
	return 0;
}

 int rometoint(std::string roman) {
    int total = 0;
    int ascii = 0;
    int* cache = new int[roman.length()];

    for (uint i = 0; i < roman.length(); i++) {
        ascii = int(toupper(roman[i]));

        switch (ascii) {
            case 73:
                cache[i] = 1;
                break;
            case 86:
                cache[i] = 5;
                break;
            case 88:
                cache[i] = 10;
                break;
            case 76:
                cache[i] = 50;
                break;
            case 67:
                cache[i] = 100;
                break;
            case 68:
                cache[i] = 500;
                break;
            case 77:
                cache[i] = 1000;
                break;
            default:
                cache [i] = 0;
                break;
        }
    }

    if (roman.length() == 1) {
        return (cache[0]);
    }

    for (uint i = 0; i < (roman.length() - 1); i++) {
        if (cache[i] >= cache[i + 1]) {
            total += cache[i];
        } else {
            total -= cache[i];
        }
    }

    total += cache[roman.length() - 1];
    delete [] cache;
    return (total);
}

 
int main2()
{  
  BSS_DebugInfo _debug("log.txt");
  unsigned int seed=time(NULL);
  void* val=(void*)&seed;
  rand();

  char p1=1;
  char p2=2;
  /*volatile cLocklessFlipper<char> flipper(&p1,&p2);

  cThread flipthread(&flippertest);
  flipthread.Start((void*)&flipper);
  Sleep(1);

  while(true)
  {
    int fail=0;
    for(int i = 0; i < 1000000; ++i)
    {
      fliphold=((cLocklessFlipper<char>&)flipper).StartWrite();
      if(!fliphold)
        ++fail;
      else { Sleep(1); fliphold=0; ((cLocklessFlipper<char>&)flipper).EndWrite(); }
    }
    std::cout << "Failed: " << (fail/1000000.0f) << std::endl;
  }
  
  return 0;

  cLocklessQueue qtest(512000);*/
  

  int id;
  //while(true)
  //{
  //  for(unsigned int i = (last=rand()%10); i>0; --i) {
  //    id=rand()%50;
  //    memset(qtest.StartWrite(id),id,id);
  //    qtest.FinishWrite();
  //  }
  //  for(unsigned int i = rand()%10; i>0; --i) {
  //    qtest.ReadNextChunk();
  //    //_debug.WriteLog("read",0,__FILE__,__LINE__);
  //  }
  //}
  
  /*bool expResult = true;

  cThread crapthread(&dorandomcrap);
  crapthread.Start(&qtest);

  FILE* f = fopen("read.txt","w");
  std::pair<void*, unsigned int> hold;
  bool br=true;
  int num=-1;
  while(br)
  {
    while((hold=qtest.ReadNextChunk()).first!=0)
    {
      if(hold.second==MAX_ALLOC+1) br=false;
      else if(hold.second!=(((++num)<<3)%MAX_ALLOC))
      {  expResult=false; assert(expResult); }
    }
  }
  
  crapthread.Join();
  if(qtest.ReadNextChunk().first!=0) { expResult=false; assert(expResult); } //This should have been done by now
  assert(expResult);
  if(!expResult) num=num/0;

  qtest.Clear();*/

  return 0;

  strlen("sadjkfds");

  //StringPoolAlloc<char> stringalloc(512);

  //blah.GetData(25);

  //unsigned int seed=1267805912;
  srand(seed);
  rand();
  char profID;

  const int MAXBANK=50;
  foobar* ptest[MAXBANK];
  memset(ptest,0,sizeof(foobar*)*MAXBANK);
//
//  for(int p = 0; p < 10; ++p)
//  {
//    profID = _debug.OpenProfiler();
//    for(int i = 0; i < 1000000; ++i)
//    {
//      int ind = RANDINTGEN(0,MAXBANK-1);
//      if(ptest[ind])
//      {
//       delete [] ptest[ind];
//        ptest[ind]=0;
//      }
//      else
//      {
//        seed=RANDINTGEN(0,120);
//        ptest[ind]= new foobar[seed];
//        for(unsigned int j = 0; j < seed; ++j)
//          ptest[ind][j] = foobar();
//
//      }
//
//#if defined(DEBUG) || defined(_DEBUG)
//      for(int j = 0; j < MAXBANK; ++j)
//        if(!!ptest[j])
//          for(int k = j; k < MAXBANK;)
//            assert(ptest[j]!=ptest[++k]);
//#endif
//    }
//    std::cout << "TIME: " << _debug.CloseProfiler(profID) << std::endl;
//  }

  //float ftest = 0.0000005f;
  //bool smalltest = fsmall(ftest);

  //cFixedSizeAllocator<foobar> _alloctest;
  //foobar* ptest[MAXBANK];d
  //memset(ptest,0,sizeof(foobar*)*MAXBANK);
  __int64 addon=0;

  for(int j = 0; j < 1; ++j)
  {
    profID = _debug.OpenProfiler();

    /* This is a brutal random allocation test, where we either allocate or deallocate a random chunk of memory. */
    for(int i = 0; i < 1000000; ++i)
    {
      int ind = RANDINTGEN(0,MAXBANK-1);
      if(ptest[ind])
      {
        delete ptest[ind];
        //_alloctest.dealloc(ptest[ind]);
        //_blockalloc.Free(ptest[ind], sizeof(foobar));
        ptest[ind]=0;
      }
      else
        ptest[ind] = new foobar();
        //ptest[ind]=_alloctest.alloc(1);
        //ptest[ind]=(foobar*)_blockalloc.Allocate(sizeof(foobar));
    }
    std::cout << "TIME: " << ((addon+=_debug.CloseProfiler(profID))/(j+1)) << std::endl;
  }
  //for(int j = 0; j < MAXBANK; ++j)
  //  if(ptest[j])
  //    delete ptest[j];

  system("Pause");
  return 0;

  //unsigned __int64 totaltime=0;
  //for(int j = 0; j < 20; ++j)
  //{
  //profID = _debug.OpenProfiler();
  //for(int i = 0; i < 100; ++i)
  //{
  //  cStrW test(L"");
  //  for(int k = 10000; k > 0; --k)
  //  {
  //    test+=L"1";
  //  }

  //  cStrW n00b(L"ANSKDNHGZKJDHFKDHFZKDJSFSFDSGJ LDKJASDKFJSDLKFJS");
  //  cStrW init(L"INITTEST)))");
  //  init=test+n00b;
  //}
  //unsigned __int64 sect = _debug.CloseProfiler(profID);
  //totaltime += sect;
  //std::cout << "TIME: " << sect << " | AVERAGE: " << (totaltime/(j+1)) << std::endl << "MEMORY: " << _debug.GetWorkingSet() << std::endl;
  //}

  SetWorkDirToCur();
  cAVLtree<int, std::pair<int,int>*>* tree = new cAVLtree<int, std::pair<int,int>*>();
  std::pair<int,int> test(5,5);
  tree->Insert(test.first,&test);
  tree->Get(test.first);
  tree->ReplaceKey(5,2);
  tree->Remove(test.first);
  tree->Clear();

  cINIstorage<char> store("release/test.ini");
  //cINIstorage store;
  store.AddSection("Hello");

  char ID = _debug.OpenProfiler();
  int get = store.GetEntry("Asdkj", "");
  
  //foobar* barfoothing= new foobar();
  //cTaskStack<foobar> foostack;
  //foostack.RegisterFunction(FUNC_DEF0<foobar>(&foobar::dumber),0);
  //foostack.RegisterFunction(FUNC_DEF1<foobar,unsigned int>(&foobar::stupid),1);
  //foostack.RegisterFunction(FUNC_DEF2<foobar,int,int>(&foobar::retarded),2);
  //foostack.RegisterFunction(FUNC_DEF3<foobar,int,int,bool>(&foobar::idiotic),3);

  //cThread nthread(&dorandomcrap);
  //std::pair<cTaskStack<foobar>*,cThread*> argpair(&foostack,&nthread);
  //nthread.Start(&argpair);
  //while(true)
  //{
  //foostack.EvaluateStack(barfoothing);
  //}

  //nthread.Stop();
  //delete barfoothing;

  cKhash_Int<BSS_DebugInfo*> hasherint;
  hasherint.Insert(25, &_debug);
  hasherint.GetKey(25);
  hasherint.Remove(25);
  cKhash_StringIns<BSS_DebugInfo*> hasher;
  int iter = hasher.Insert("",&_debug);
  iter = hasher.Insert("Video",(BSS_DebugInfo*)5);
  hasher.SetSize(100);
  iter = hasher.Insert("Physics",0);
  BSS_DebugInfo* check = *hasher.GetKey("Video");
  check = *hasher.GetKey("Video");
  unsigned __int64 diff = _debug.CloseProfiler(ID);
}

//const char string[] = "FourscoreandsevenyearsagoourfaathersbroughtforthonthiscontainentanewnationconceivedinzLibertyanddedicatedtothepropositionthatallmenarecreatedequalNowweareengagedinagreahtcivilwartestingwhetherthatnaptionoranynartionsoconceivedandsodedicatedcanlongendureWeareqmetonagreatbattlefiemldoftzhatwarWehavecometodedicpateaportionofthatfieldasafinalrestingplaceforthosewhoheregavetheirlivesthatthatnationmightliveItisaltogetherfangandproperthatweshoulddothisButinalargersensewecannotdedicatewecannotconsecratewecannothallowthisgroundThebravelmenlivinganddeadwhostruggledherehaveconsecrateditfaraboveourpoorponwertoaddordetractTgheworldadswfilllittlenotlenorlongrememberwhatwesayherebutitcanneverforgetwhattheydidhereItisforusthelivingrathertobededicatedheretotheulnfinishedworkwhichtheywhofoughtherehavethusfarsonoblyadvancedItisratherforustobeherededicatedtothegreattdafskremainingbeforeusthatfromthesehonoreddeadwetakeincreaseddevotiontothatcauseforwhichtheygavethelastpfullmeasureofdevotionthatweherehighlyresolvethatthesedeadshallnothavediedinvainthatthisnationunsderGodshallhaveanewbirthoffreedomandthatgovernmentofthepeoplebythepeopleforthepeopleshallnotperishfromtheearth";
//
//inline bool backwardscheck(const char* begin, int length)
//{
//  int mid = length/2;
//  --length;
//  for(int i = 0; i <= mid; ++i)
//    if(begin[i]!=begin[length-i]) return false;
//
//  return true;
//}

inline bool isprime(int number)
{
  if(number%2==0) return number==2;
  int stop=number/2;
  for(int i = 3; i < stop; ++i)
    if(number%i==0) return false;
  return true;
}

inline int addrecursive(int start,int prev)
{
  int retval=numarray[start]==prev?1:0;

}

extern void kdtestmain();

char* inttoroman(int in)
{
	int charnum=0;
	int i2=0;
	int hold=in;
	
	for(int j=1000; j>0; j/=5)
	{
		charnum+=(i2=(hold/j));
		hold-=i2*j;
		j/=2;
    if(!j) break;
		charnum+=(i2=(hold/j));
		hold-=i2*j;
	}
	
	char* str=new char[charnum+1];
	for(int i=0; i<charnum+1;++i) str[i]=0;
	
	int count=-1;
	while(in>=1000) { str[++count]='M'; in-=1000; };
	while(in>=900) { str[++count]='C'; str[++count]='M'; in-=900; };
	while(in>=500) { str[++count]='D'; in-=500; };
	while(in>=400) { str[++count]='C'; str[++count]='D'; in-=400; };
	while(in>=100) { str[++count]='C'; in-=100; };
	while(in>=90) { str[++count]='X'; str[++count]='C'; in-=90; };
	while(in>=50) { str[++count]='L'; in-=50; };
	while(in>=40) { str[++count]='X'; str[++count]='L'; in-=40; };
	while(in>=10) { str[++count]='X'; in-=10; };
	while(in>=9) { str[++count]='I'; str[++count]='X'; in-=9; };
	while(in>=5) { str[++count]='V'; in-=5; };
	while(in>=4) { str[++count]='I'; str[++count]='V'; in-=4; };
	while(in>=1) { str[++count]='I'; in-=1; };
	
	return str;
}