// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Thread.h"
#include "bss-util/algo.h"
#include "test.h"

using namespace bss;

#if defined(BSS_CPU_x86) || defined(BSS_CPU_x64)
// This is an SSE version of the fast sqrt that calculates x*invsqrt(x) as a speed hack. Sadly, it's still slower and actually LESS accurate than the classic FastSqrt with an added iteration, below, and it isn't even portable. Left here for reference, in case you don't believe me ;)
BSS_FORCEINLINE float sseFastSqrt(float f)
{
  float r;
  __m128 in = _mm_load_ss(&f);
  _mm_store_ss(&r, _mm_mul_ss(in, _mm_rsqrt_ss(in)));
  return r;
}
#endif

template<typename T>
T calceps()
{
  T e = (T)0.5;
  while((T)(1.0 + (e / 2.0)) != 1.0) { e /= (T)2.0; }
  return e;
}


int getuniformint()
{
  static const int NUM = 8;
  static int cur = NUM;
  static int samples[NUM] = { 0 };
  if(cur >= NUM)
  {
    int last = samples[NUM - 1];
    samples[0] = 0;
    for(int i = 1; i<NUM; ++i)
    {
      int64_t j = bssRandInt(0, i + 1);
      samples[i] = samples[j];
      samples[j] = i;
    }
    if(last == samples[0])
    {
      int64_t j = 1 + bssRandInt(0, NUM - 1);
      samples[0] = samples[j];
      samples[j] = last;
    }
    cur = 0;
  }
  return samples[cur++];
}



/*template<class Alloc, int MAX, int SZ>
void profile_push(Alloc& a, std::atomic<void*>* q)
{
while(!startflag.load(std::memory_order_acquire));
uint64_t c;
while((c=lq_c.fetch_add(1, std::memory_order_acquire))<MAX) {
void* p=a.allocate((c<<3)%SZ);
q[c].store((!p?(void*)1:p), std::memory_order_release);
//q[c] = p;
}
}

std::atomic<size_t> lq_r;

template<class Alloc, int MAX>
void profile_pop(Alloc& a, std::atomic<void*>* q)
{
while(!startflag.load(std::memory_order_acquire));
void* p;
uint64_t c;
while((c=lq_r.fetch_add(1, std::memory_order_acquire))<MAX)
{
while(!(p = q[c].load(std::memory_order_acquire)));
a.deallocate((size_t*)p);
}
startflag.store(false, std::memory_order_release);
}

template<class Alloc, int MAX, int SZ>
uint64_t doprofile(Alloc& a)
{
lq_c.store(0);
lq_r.store(0);
std::atomic<void*> arr[MAX];
memset(arr, 0, sizeof(void*)*MAX);
const int NUM = 8;
Thread threads[NUM];
startflag.store(false);
for(size_t i = 0; i < NUM; ++i)
threads[i] = Thread((i%2)?&profile_pop<Alloc, MAX>:&profile_push<Alloc, MAX, SZ>, std::ref(a), arr);

auto prof = HighPrecisionTimer::OpenProfiler();
startflag.store(true);

while(startflag.load(std::memory_order_acquire));
uint64_t diff = HighPrecisionTimer::CloseProfiler(prof);

for(size_t i = 0; i < NUM; ++i)
threads[i].join();

return diff;
}

void profile_ring_alloc()
{
NullAllocPolicy<size_t> nalloc;
RingPolicy<size_t> ralloc(50000);
StandardAllocPolicy<size_t> salloc;
//std::this_thread::sleep_for(std::chrono::duration<uint64_t>::min());

std::cout << doprofile<NullAllocPolicy<size_t>, 500000, 100>(nalloc) << std::endl;
std::cout << doprofile<StandardAllocPolicy<size_t>, 500000, 100>(salloc) << std::endl;
std::cout << doprofile<RingPolicy<size_t>, 500000, 100>(ralloc) << std::endl;
std::cout << doprofile<NullAllocPolicy<size_t>, 500000, 100>(nalloc) << std::endl;
}*/

/*void subleq_computer(int[] mem)
{
int c=0;
int b;
while(c>=0)
{
b=mem[c+1];
mem[b] = mem[b]-mem[mem[c]];
c=(mem[b]>0)?(c+3):mem[c+2];
}
}*/


// --- The rest of this file is archived dead code ---



/*struct OBJSWAP_TEST {
uint32_t i;
bool operator==(const OBJSWAP_TEST& j) const { return i==j.i; }
bool operator!=(const OBJSWAP_TEST& j) const { return i!=j.i; }
};

TESTDEF::RETPAIR test_OBJSWAP()
{
BEGINTEST;

uint32_t vals[] = { 0,1,2,3,4,5 };
const char* strs[] = { "001", "002", "003", "004", "005" };
uint32_t* zp=vals+0;
uint32_t* zp2=vals+1;
uint32_t* zp3=vals+2;
uint32_t* zp4=vals+3;
uint32_t* zp5=vals+4;
OBJSWAP_TEST o[6] = { {1},{2},{3},{4},{5},{6} };
for(size_t i = 0; i < 5; ++i)
{
switch(PSWAP(vals+i,5,zp,zp2,zp3,zp4,zp5))
{
case 0:
TEST(i==0); break;
case 1:
TEST(i==1); break;
case 2:
TEST(i==2); break;
case 3:
TEST(i==3); break;
case 4:
TEST(i==4); break;
default:
TEST(false);
}

switch(cObjSwap<const bsschar*>(PANGRAMS[i],5,PANGRAMS[4],PANGRAMS[3],PANGRAMS[2],PANGRAMS[1],PANGRAMS[0]))
{
case 0:
TEST(i==4); break;
case 1:
TEST(i==3); break;
case 2:
TEST(i==2); break;
case 3:
TEST(i==1); break;
case 4:
TEST(i==0); break;
default:
TEST(false);
}

switch(STRSWAP(strs[i],5,"000","002","003","004","005")) // Deliberaly meant to test for one failure
{
case 0:
TEST(false); break;
case 1:
TEST(i==1); break;
case 2:
TEST(i==2); break;
case 3:
TEST(i==3); break;
case 4:
TEST(i==4); break;
default:
TEST(i==0);
}

switch(cObjSwap<OBJSWAP_TEST>(o[i],5,o[0],o[1],o[5],o[3],o[4])) // Deliberaly meant to test for one failure
{
case 0:
TEST(i==0); break;
case 1:
TEST(i==1); break;
case 2:
TEST(false); break;
case 3:
TEST(i==3); break;
case 4:
TEST(i==4); break;
default:
TEST(i==2);
}
}

ENDTEST;
}*/

//void destroynode(std::pair<int,int>* data)
//{
//  delete data;
//}
//
//struct sp {
//  int x;
//  int y;
//};

/*
int PI_ITERATIONS=500;
double pi=((PI_ITERATIONS<<1)-1)+(PI_ITERATIONS*PI_ITERATIONS);

const char* MBSTESTSTRINGS[] = { "test","test2","test3","test4","test5","test6" };
const wchar_t* WCSTESTSTRINGS[] = { BSS__L("test"),BSS__L("test2"),BSS__L("test3"),BSS__L("test4"),BSS__L("test5"),BSS__L("test6") };

struct weird
{
void* p1;
int64_t i;
short offset;
char blah;
inline bool valid() { return i==-1 && offset == -1 && blah == -1; }
inline bool invalid() { return i==0 && offset == 0 && blah == 0; }
inline void invalidate() { i=0; offset=0;blah=0; }
inline void validate() { i=-1; offset=-1;blah=-1; }
};

void printout(LinkedList<int,StandardAllocPolicy<LLNode<int>>,true>& list)
{
LLIter<int> cur(list.GetRoot());

while(cur.IsValid())
std::cout<<*(cur++);

std::cout<<std::endl;
}

void printout(LinkedArray<int>& list)
{
auto cur=list.begin();

while(cur.IsValid())
std::cout<<*(cur++);

std::cout<<std::endl;
}*/

//const char* FAKESTRINGLIST[5] = { "FOO", "BAR", "MEH", "SILLY", "EXACERBATION" };

//int main3(int argc, char** argv)
//{  
//char* romanstuff = inttoroman(3333);

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
//for(size_t i = 0; i < prime_divisors.size(); ++i) sum+=prime_divisors[i];

//	return 0;
//}

int rometoint(std::string roman) {
  int total = 0;
  int ascii = 0;
  std::unique_ptr<int[]> cache(new int[roman.length()]);

  for(size_t i = 0; i < roman.length(); i++)
  {
    ascii = int(toupper(roman[i]));

    switch(ascii)
    {
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
      cache[i] = 0;
      break;
    }
  }

  if(roman.length() == 1)
  {
    return (cache[0]);
  }

  for(size_t i = 0; i < (roman.length() - 1); i++)
  {
    if(cache[i] >= cache[i + 1])
    {
      total += cache[i];
    }
    else
    {
      total -= cache[i];
    }
  }

  total += cache[roman.length() - 1];
  return (total);
}

//inline bool backwardscheck(const char* begin, int length)
//{
//  int mid = length/2;
//  --length;
//  for(size_t i = 0; i <= mid; ++i)
//    if(begin[i]!=begin[length-i]) return false;
//
//  return true;
//}

// This is painfully slow and I don't even know why its here.
inline bool isprime(int number)
{
  if(number % 2 == 0) return number == 2;
  int stop = number / 2;
  for(size_t i = 3; i < stop; ++i)
    if(number%i == 0) return false;
  return true;
}

//inline int addrecursive(int start,int prev)
//{
//  int retval=numarray[start]==prev?1:0;
//}


//double x=0; // initial position
//double t=0;
//uint32_t steps=100;
//double step=5.0/steps;
//for(size_t i = 0; i < steps; ++i)
//{
//  double fx = 1 - x*x;
//  //x = x + fx*step;
//  double xt = x + fx*step;
//  double fxt = 1 - xt*xt;
//  x = x + 0.5*(fx + fxt)*step;
//  if(i%10==9) 
//    std::cout << x << std::endl;
//}
//std::cin.get();
//return 0;

char* inttoroman(int in)
{
  int charnum = 0;
  int i2 = 0;
  int hold = in;

  for(int j = 1000; j>0; j /= 5)
  {
    charnum += (i2 = (hold / j));
    hold -= i2*j;
    j /= 2;
    if(!j) break;
    charnum += (i2 = (hold / j));
    hold -= i2*j;
  }

  char* str = new char[charnum + 1];
  for(size_t i = 0; i<charnum + 1; ++i) str[i] = 0;

  int count = -1;
  while(in >= 1000) { str[++count] = 'M'; in -= 1000; };
  while(in >= 900) { str[++count] = 'C'; str[++count] = 'M'; in -= 900; };
  while(in >= 500) { str[++count] = 'D'; in -= 500; };
  while(in >= 400) { str[++count] = 'C'; str[++count] = 'D'; in -= 400; };
  while(in >= 100) { str[++count] = 'C'; in -= 100; };
  while(in >= 90) { str[++count] = 'X'; str[++count] = 'C'; in -= 90; };
  while(in >= 50) { str[++count] = 'L'; in -= 50; };
  while(in >= 40) { str[++count] = 'X'; str[++count] = 'L'; in -= 40; };
  while(in >= 10) { str[++count] = 'X'; in -= 10; };
  while(in >= 9) { str[++count] = 'I'; str[++count] = 'X'; in -= 9; };
  while(in >= 5) { str[++count] = 'V'; in -= 5; };
  while(in >= 4) { str[++count] = 'I'; str[++count] = 'V'; in -= 4; };
  while(in >= 1) { str[++count] = 'I'; in -= 1; };

  return str;
}

