// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "Shiny.h"
#include "bss_util.h"
#include "bss_DebugInfo.h"
#include "bss_algo.h"
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
#include "cDynArray.h"
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
#include "os.h"
#include "StreamSplitter.h"

#include <fstream>
#include <algorithm>
#include <time.h>

//#define BOOST_FILESYSTEM_VERSION 3
//#define BOOST_ALL_NO_LIB
//#define BOOST_ALL_DYN_LINK
//#define BOOST_ALL_NO_DEPRECATED
//#include <boost/filesystem.hpp>

#pragma warning(disable:4566)

using namespace bss_util;

// --- Define testing utilities ---

struct TEST
{
  typedef std::pair<size_t,size_t> RETPAIR;
  const char* NAME;
  RETPAIR (*FUNC)();
};

#define BEGINTEST TEST::RETPAIR __testret(0,0);
#define ENDTEST return __testret
#define FAILEDTEST(t) BSSLOG(_failedtests,1) << "Test #" << __testret.first << " Failed  < " << MAKESTRING(t) << " >" << std::endl
#define TEST(t) { ++__testret.first; try { if(t) ++__testret.second; else FAILEDTEST(t); } catch(...) { FAILEDTEST(t); } }
#define TESTERROR(t, e) { ++__testret.first; try { (t); FAILEDTEST(t); } catch(e) { ++__testret.second; } }
#define TESTERR(t) TESTERROR(t,...)
#define TESTNOERROR(t) { ++__testret.first; try { (t); ++__testret.second; } catch(...) { FAILEDTEST(t); } }
#define TESTARRAY(t,f) _ITERFUNC(__testret,t,[&](uint i) -> bool { f });
#define TESTALL(t,f) _ITERALL(__testret,t,[&](uint i) -> bool { f });
#define TESTCOUNT(c,t) { for(uint i = 0; i < c; ++i) TEST(t) }
#define TESTCOUNTALL(c,t) { bool __val=true; for(uint i = 0; i < c; ++i) __val=__val&&(t); TEST(__val); }

template<class T, size_t SIZE, class F>
void _ITERFUNC(TEST::RETPAIR& __testret, T (&t)[SIZE], F f) { for(uint i = 0; i < SIZE; ++i) TEST(f(i)) }
template<class T, size_t SIZE, class F>
void _ITERALL(TEST::RETPAIR& __testret, T (&t)[SIZE], F f) { bool __val=true; for(uint i = 0; i < SIZE; ++i) __val=__val&&(f(i)); TEST(__val); }

template<class T>
T naivebitcount(T v)
{
  T c;
  for(c = 0; v; v >>= 1)
    c += (v & 1);
  return c;
}

template<class T>
void testbitcount(TEST::RETPAIR& __testret)
{ //Use fibonacci numbers to test this
  for(T i = 0; i < (((T)1)<<(sizeof(T)<<2)); i=fbnext(i)) {
    TEST(naivebitcount<T>(i)==bitcount<T>(i));
  }
}
template<class T> void VERIFYTYPE(const T& type) { }

// This defines an enormous list of pangrams for a ton of languages, used for text processing in an attempt to expose possible unicode errors.
const char* PANGRAM = "The wizard quickly jinxed the gnomes before they vapourized.";
const wchar_t* PANGRAMS[] = { 
  L"The wizard quickly jinxed the gnomes before they vapourized.",
  L"صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ", //Arabic
  L"Zəfər, jaketini də papağını da götür, bu axşam hava çox soyuq olacaq.", //Azeri
  L"Ах чудна българска земьо, полюшквай цъфтящи жита.", //Bulgarian
  L"Jove xef, porti whisky amb quinze glaçons d'hidrogen, coi!", //Catalan
  L"Příliš žluťoučký kůň úpěl ďábelské ódy.", //Czech
  L"Høj bly gom vandt fræk sexquiz på wc", //Danish
  L"Filmquiz bracht knappe ex-yogi van de wijs", //Dutch
  L"ཨ་ཡིག་དཀར་མཛེས་ལས་འཁྲུངས་ཤེས་བློའི་གཏེར༎ ཕས་རྒོལ་ཝ་སྐྱེས་ཟིལ་གནོན་གདོང་ལྔ་བཞིན༎ ཆགས་ཐོགས་ཀུན་བྲལ་མཚུངས་མེད་འཇམ་དབྱངསམཐུས༎ མཧཱ་མཁས་པའི་གཙོ་བོ་ཉིད་འགྱུར་ཅིག།", //Dzongkha
  L"Eble ĉiu kvazaŭ-deca fuŝĥoraĵo ĝojigos homtipon.", //Esperanto
  L"Põdur Zagrebi tšellomängija-följetonist Ciqo külmetas kehvas garaažis", //Estonian
  L"Törkylempijävongahdus", //Finnish
  L"Falsches Üben von Xylophonmusik quält jeden größeren Zwerg", //German
  L"Τάχιστη αλώπηξ βαφής ψημένη γη, δρασκελίζει υπέρ νωθρού κυνός", //Greek
  L"כך התרסק נפץ על גוזל קטן, שדחף את צבי למים", //Hebrew
  L"दीवारबंद जयपुर ऐसी दुनिया है जहां लगभग हर दुकान का नाम हिन्दी में लिखा गया है। नामकरण की ऐसी तरतीब हिन्दुस्तान में कम दिखती है। दिल्ली में कॉमनवेल्थ गेम्स के दौरान कनॉट प्लेस और पहाड़गंज की नामपट्टिकाओं को एक समान करने का अभियान चला। पत्रकार लिख", //Hindi
  L"Kæmi ný öxi hér, ykist þjófum nú bæði víl og ádrepa.", //Icelandic
  L"いろはにほへと ちりぬるを わかよたれそ つねならむ うゐのおくやま けふこえて あさきゆめみし ゑひもせす（ん）", //Japanese
  L"꧋ ꦲꦤꦕꦫꦏ꧈ ꦢꦠꦱꦮꦭ꧈ ꦥꦝꦗꦪꦚ꧈ ꦩꦒꦧꦛꦔ꧉", //Javanese
  L"    ", //Klingon
  L"키스의 고유조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다.", //Korean
  L"သီဟိုဠ်မှ ဉာဏ်ကြီးရှင်သည် အာယုဝဍ္ဎနဆေးညွှန်းစာကို ဇလွန်ဈေးဘေးဗာဒံပင်ထက် အဓိဋ္ဌာန်လျက် ဂဃနဏဖတ်ခဲ့သည်။", //Myanmar
  L"بر اثر چنین تلقین و شستشوی مغزی جامعی، سطح و پایه‌ی ذهن و فهم و نظر بعضی اشخاص واژگونه و معکوس می‌شود.‏", //Persian
  L"À noite, vovô Kowalsky vê o ímã cair no pé do pingüim queixoso e vovó põe açúcar no chá de tâmaras do jabuti feliz.", //Portuguese
  L"Эх, чужак! Общий съём цен шляп (юфть) – вдрызг!", //Russian
  L"Fin džip, gluh jež i čvrst konjić dođoše bez moljca.", //Serbian
  L"Kŕdeľ ďatľov učí koňa žrať kôru.", //Slovak
  L"เป็นมนุษย์สุดประเสริฐเลิศคุณค่า กว่าบรรดาฝูงสัตว์เดรัจฉาน จงฝ่าฟันพัฒนาวิชาการ อย่าล้างผลาญฤๅเข่นฆ่าบีฑาใคร ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า หัดอภัยเหมือนกีฬาอัชฌาสัย ปฏิบัติประพฤติกฎกำหนดใจ พูดจาให้จ๊ะๆ จ๋าๆ น่าฟังเอยฯ", //Thai
  L"ژالہ باری میں ر‌ضائی کو غلط اوڑھے بیٹھی قرۃ العین اور عظمٰی کے پاس گھر کے ذخیرے سے آناً فاناً ڈش میں ثابت جو، صراحی میں چائے اور پلیٹ میں زردہ آیا۔" //Urdu
};

const int TESTNUM=100000;
int testnums[TESTNUM];
BSSDEBUG _debug;
bss_Log _failedtests("../bin/failedtests.txt"); //This is spawned too early for us to save it with SetWorkDirToCur();

template<unsigned char B, __int64 SMIN, __int64 SMAX, unsigned __int64 UMIN, unsigned __int64 UMAX>
inline void BSS_FASTCALL TEST_ABITLIMIT(TEST::RETPAIR& __testret)
{
  VERIFYTYPE<char>(ABitLimit<1>::SIGNED(0));
  VERIFYTYPE<unsigned char>(ABitLimit<1>::UNSIGNED(0));
  TEST(ABitLimit<B>::SIGNED_MIN==SMIN); 
  TEST(ABitLimit<B>::SIGNED_MAX==SMAX);
  TEST(ABitLimit<B>::UNSIGNED_MIN==UMIN);
  TEST(ABitLimit<B>::UNSIGNED_MAX==UMAX);
}

template<typename T, size_t S> inline static size_t BSS_FASTCALL _ARRSIZE(const T (&a)[S]) { return S; }

#if defined(BSS_CPU_x86) || defined(BSS_CPU_x64)
  /* This is an SSE version of the fast sqrt that calculates x*invsqrt(x) as a speed hack. Sadly, it's still slower and actually LESS accurate than the classic FastSqrt with an added iteration, below, and it isn't even portable. Left here for reference, in case you don't believe me ;) */
  inline BSS_FORCEINLINE float sseFastSqrt(float f)
  {
    float r;
    __m128 in = _mm_load_ss(&f);
    _mm_store_ss( &r, _mm_mul_ss( in, _mm_rsqrt_ss( in ) ) );
    return r;
  }
#endif

 template<typename T>
 T calceps()
 {
    T e = (T)0.5;
    while ((T)(1.0 + (e/2.0)) != 1.0) { e /= (T)2.0; }
    return e;
 }

// --- Begin actual test procedure definitions ---

TEST::RETPAIR test_bss_util()
{
  BEGINTEST;
  TESTNOERROR(SetWorkDirToCur());
  char fbuf[MAX_PATH];
  GetModuleFileNameA(0,fbuf,MAX_PATH);
  TEST(bssFileSize(fbuf)!=0);
  //TEST(bssFileSize(cStrW(fbuf))!=0);
  TESTNOERROR(GetTimeZoneMinutes());
  
  TSignPick<sizeof(long double)>::SIGNED _u1;
  TSignPick<sizeof(double)>::SIGNED _u2;
  TSignPick<sizeof(float)>::SIGNED _u3;
  TSignPick<sizeof(long double)>::UNSIGNED _v1;
  TSignPick<sizeof(double)>::UNSIGNED _v2;
  TSignPick<sizeof(float)>::UNSIGNED _v3;
  TEST(T_CHARGETMSB(0)==0);
  TEST(T_CHARGETMSB(1)==1);
  TEST(T_CHARGETMSB(2)==2);
  TEST(T_CHARGETMSB(3)==2);
  TEST(T_CHARGETMSB(4)==4);
  TEST(T_CHARGETMSB(7)==4);
  TEST(T_CHARGETMSB(8)==8);
  TEST(T_CHARGETMSB(20)==16);
  TEST(T_CHARGETMSB(84)==64);
  TEST(T_CHARGETMSB(189)==128);
  TEST(T_CHARGETMSB(255)==128);

  //TBitLimit conveniently calls TSignPick for us. Note that these tests CAN fail if trying to compile to an unsupported or buggy platform that doesn't have two's complement.
  TEST(TBitLimit<long long>::SIGNED_MIN == std::numeric_limits<long long>::min());
  TEST(TBitLimit<long>::SIGNED_MIN == std::numeric_limits<long>::min());
  TEST(TBitLimit<int>::SIGNED_MIN == std::numeric_limits<int>::min());
  TEST(TBitLimit<short>::SIGNED_MIN == std::numeric_limits<short>::min());
  TEST(TBitLimit<char>::SIGNED_MIN == std::numeric_limits<char>::min());
  TEST(TBitLimit<long long>::SIGNED_MAX == std::numeric_limits<long long>::max());
  TEST(TBitLimit<long>::SIGNED_MAX == std::numeric_limits<long>::max());
  TEST(TBitLimit<int>::SIGNED_MAX == std::numeric_limits<int>::max());
  TEST(TBitLimit<short>::SIGNED_MAX == std::numeric_limits<short>::max());
  TEST(TBitLimit<char>::SIGNED_MAX == std::numeric_limits<char>::max());
  TEST(TBitLimit<long long>::UNSIGNED_MAX == std::numeric_limits<unsigned long long>::max());
  TEST(TBitLimit<long>::UNSIGNED_MAX == std::numeric_limits<unsigned long>::max());
  TEST(TBitLimit<int>::UNSIGNED_MAX == std::numeric_limits<unsigned int>::max());
  TEST(TBitLimit<short>::UNSIGNED_MAX == std::numeric_limits<unsigned short>::max());
  TEST(TBitLimit<char>::UNSIGNED_MAX == std::numeric_limits<unsigned char>::max());
  TEST(TBitLimit<long long>::UNSIGNED_MIN == std::numeric_limits<unsigned long long>::min());
  TEST(TBitLimit<long>::UNSIGNED_MIN == std::numeric_limits<unsigned long>::min());
  TEST(TBitLimit<int>::UNSIGNED_MIN == std::numeric_limits<unsigned int>::min());
  TEST(TBitLimit<short>::UNSIGNED_MIN == std::numeric_limits<unsigned short>::min());
  TEST(TBitLimit<char>::UNSIGNED_MIN == std::numeric_limits<unsigned char>::min());

  // These tests assume twos complement. This is ok because the previous tests would have caught errors relating to that anyway.
  TEST_ABITLIMIT<1,-1,0,0,1>(__testret);
  TEST_ABITLIMIT<2,-2,1,0,3>(__testret);
  TEST_ABITLIMIT<7,-64,63,0,127>(__testret);
  VERIFYTYPE<char>(ABitLimit<8>::SIGNED(0));
  VERIFYTYPE<unsigned char>(ABitLimit<8>::UNSIGNED(0));
  TEST(ABitLimit<8>::SIGNED_MIN==std::numeric_limits<char>::min());
  TEST(ABitLimit<8>::SIGNED_MAX==std::numeric_limits<char>::max());
  TEST(ABitLimit<8>::UNSIGNED_MIN==std::numeric_limits<unsigned char>::min());
  TEST(ABitLimit<8>::UNSIGNED_MAX==std::numeric_limits<unsigned char>::max());
  TEST_ABITLIMIT<9,-256,255,0,511>(__testret);
  TEST_ABITLIMIT<15,-16384,16383,0,32767>(__testret);
  VERIFYTYPE<short>(ABitLimit<16>::SIGNED(0));
  VERIFYTYPE<unsigned short>(ABitLimit<16>::UNSIGNED(0));
  TEST(ABitLimit<16>::SIGNED_MIN==std::numeric_limits<short>::min());
  TEST(ABitLimit<16>::SIGNED_MAX==std::numeric_limits<short>::max());
  TEST(ABitLimit<16>::UNSIGNED_MIN==std::numeric_limits<unsigned short>::min());
  TEST(ABitLimit<16>::UNSIGNED_MAX==std::numeric_limits<unsigned short>::max());
  TEST_ABITLIMIT<17,-65536,65535,0,131071>(__testret);
  TEST_ABITLIMIT<63,-4611686018427387904,4611686018427387903,0,9223372036854775807>(__testret);
  TEST_ABITLIMIT<64,-9223372036854775808,9223372036854775807,0,18446744073709551615>(__testret);
  // For reference, the above strange bit values are used in fixed-point arithmetic found in bss_fixedpt.h

  TEST(GetBitMask<unsigned char>(4)==0x10); // 0001 0000
  TEST(GetBitMask<unsigned char>(2,4)==0x1C); // 0001 1100
  TEST(GetBitMask<unsigned char>(-2,2)==0xC7); // 1100 0111
  TEST(GetBitMask<unsigned char>(-2,-2)==0x40); // 0100 0000
  TEST(GetBitMask<unsigned char>(0,0)==0x01); // 0000 0001
  TEST(GetBitMask<unsigned char>(0,5)==0x3F); // 0011 1111
  TEST(GetBitMask<unsigned char>(0,7)==0xFF); // 1111 1111
  TEST(GetBitMask<unsigned char>(-7,0)==0xFF); // 1111 1111
  TEST(GetBitMask<unsigned char>(-5,0)==0xF9); // 1111 1001
  TEST(GetBitMask<unsigned char>(-5,-1)==0xF8); // 1111 1000
  TEST(GetBitMask<unsigned char>(-6,-3)==0x3C); // 0011 1100
  TEST(GetBitMask<unsigned int>(0,0)==0x00000001);
  TEST(GetBitMask<unsigned int>(0,16)==0x0001FFFF);
  TEST(GetBitMask<unsigned int>(12,30)==0x7FFFF000);
  TEST(GetBitMask<unsigned int>(-10,0)==0xFFC00001); 
  TEST(GetBitMask<unsigned int>(-30,0)==0xFFFFFFFD); 
  TEST(GetBitMask<unsigned int>(-12,-1)==0xFFF00000);
  TEST(GetBitMask<unsigned int>(-15,-12)==0x001e0000);
  for(uint i = 0; i < 8; ++i)
    TEST(GetBitMask<unsigned char>(i)==(1<<i));
  for(uint i = 0; i < 32; ++i)
    TEST(GetBitMask<unsigned int>(i)==(1<<i));
  for(uint i = 0; i < 64; ++i)
    TEST(GetBitMask<unsigned long long>(i)==(((unsigned __int64)1)<<i));

  std::string cpan(PANGRAM);

  strreplace(const_cast<char*>(cpan.c_str()),'m','?');
  TEST(!strchr(cpan.c_str(),'m') && strchr(cpan.c_str(),'?')!=0);
  std::wstring pan;
  for(uint i = 0; i < _ARRSIZE(PANGRAMS); ++i)
  {
    pan=PANGRAMS[i];
    wchar_t f=pan[((i+7)<<3)%pan.length()];
    wchar_t r=pan[((((i*13)>>3)+13)<<3)%pan.length()];
    if(f==r) r=pan[pan.length()-1];
    strreplace(const_cast<wchar_t*>(pan.c_str()),f,r);
    TEST(!wcschr(pan.c_str(),f) && wcschr(pan.c_str(),r)!=0);
  }
  TEST(strccount<char>("10010010101110001",'1')==8);
  TEST(strccount<char>("0100100101011100010",'1')==8);
  TEST(strccount<wchar_t>(L"الرِضَءَجيعُ بِهءَرِا نَجلاءَرِ رِمِعطارِ",L'رِ')==5);

  int ia=0;
  int ib=1;
  std::pair<int,int> sa(1,2);
  std::pair<int,int> sb(2,1);
  std::unique_ptr<int[]> ua(new int[2]);
  std::unique_ptr<int[]> ub((int*)0);
  std::string ta("first");
  std::string tb("second");
  rswap(ia,ib);
  TEST(ia==1);
  TEST(ib==0);
  rswap(sa,sb);
  TEST((sa==std::pair<int,int>(2,1)));
  TEST((sb==std::pair<int,int>(1,2)));
  rswap(ua,ub);
  TEST(ua.get()==0);
  TEST(ub.get()!=0);
  rswap(ta,tb);
  TEST(ta=="second");
  TEST(tb=="first");

  int r[] = { -1,0,2,3,4,5,6 };
  int rr[] = { 6,5,4,3,2,0,-1 };
  bssreverse(r);
  TESTARRAY(r,return (r[0]==rr[0]);)

  const char* LTRIM = "    trim ";
  TEST(!strcmp(strltrim(LTRIM),"trim "));
  char RTRIM[] = {' ','t','r','i','m',' ',' ',0 }; // :|
  TEST(!strcmp(strrtrim(RTRIM)," trim"));
  RTRIM[5]=' ';
  TEST(!strcmp(strtrim(RTRIM),"trim"));

  unsigned int nsrc[] = { 0,1,2,3,4,5,10,13,21,2873,3829847,2654435766 };
  unsigned int num[] = { 1,2,4,5,7,8,17,21,34,4647,6193581,4292720341 };
  transform(nsrc,&fbnext<unsigned int>);
  TESTARRAY(nsrc,return nsrc[i]==num[i];)
    
  int value=8;
  int exact=value;
  int exactbefore=value;

  while(value < 100000)
  {    
    exact+=exactbefore;
    exactbefore=exact-exactbefore;
    value=fbnext(value);
  }

  TEST(tsign(2.8)==1)
  TEST(tsign(-2.8)==-1)
  TEST(tsign(23897523987453.8f)==1)
  TEST(tsign((__int64)0)==1)
  TEST(tsign(0.0)==1)
  TEST(tsign(0.0f)==1)
  TEST(tsign(-28738597)==-1)
  TEST(tsign(INT_MIN)==-1)
  TEST(tsign(INT_MAX)==1)
  TEST(tsignzero(2.8)==1)
  TEST(tsignzero(-2.8)==-1)
  TEST(tsignzero(23897523987453.8f)==1)
  TEST(tsignzero((__int64)0)==0)
  TEST(tsignzero(0.0)==0)
  TEST(tsignzero(0.0f)==0)
  TEST(tsignzero(-28738597)==-1)
  TEST(tsignzero(INT_MIN)==-1)
  TEST(tsignzero(INT_MAX)==1)

  TEST(fcompare(angledist(PI,PI_HALF),PI_HALF))
  TEST(fsmall(angledist(PI,PI+PI_DOUBLE)))
  TEST(fcompare(angledist(PI_DOUBLE+PI,PI+PI_HALF*7.0),PI_HALF))
  TEST(fcompare(angledist(PI+PI_HALF*7.0,PI_DOUBLE+PI),PI_HALF))
  TEST(fcompare(angledist(PIf,PI_HALFf),PI_HALFf))
  TEST(fsmall(angledist(PIf,PIf+PI_DOUBLEf),FLT_EPS*4))
#ifndef BSS_DEBUG // In debug mode, we use precise floating point. In release mode, we use fast floating point. This lets us test both models to reveal any significant differences.
  TEST(fcompare(angledist(PI_DOUBLEf+PIf,PIf+PI_HALFf*7.0f),PI_HALFf,9)) // As one would expect, in fast floating point we need to be more tolerant of minor errors.
  TEST(fcompare(angledist(PIf+PI_HALFf*7.0f,PI_DOUBLEf+PIf),PI_HALFf,9))
#else
  TEST(fcompare(angledist(PI_DOUBLEf+PIf,PIf+PI_HALFf*7.0f),PI_HALFf)) // In precise mode, however, the result is almost exact
  TEST(fcompare(angledist(PIf+PI_HALFf*7.0f,PI_DOUBLEf+PIf),PI_HALFf))
#endif

  TEST(fcompare(angledistsgn(PI,PI_HALF),PI_HALF))
  TEST(fsmall(angledistsgn(PI,PI+PI_DOUBLE)))
  TEST(fcompare(angledistsgn(PI_DOUBLE+PI,PI+PI_HALF*7.0),PI_HALF))
  TEST(fcompare(angledistsgn(PI_HALF,PI),-PI_HALF))
  TEST(fcompare(angledistsgn(PIf,PI_HALFf),PI_HALFf))
  TEST(fsmall(angledistsgn(PIf,PIf+PI_DOUBLEf),FLT_EPS*4))
#ifndef BSS_DEBUG
  TEST(fcompare(angledistsgn(PI_DOUBLEf+PIf,PIf+PI_HALFf*7.0f),PI_HALFf,9))
#else
  TEST(fcompare(angledistsgn(PI_DOUBLEf+PIf,PIf+PI_HALFf*7.0f),PI_HALFf))
#endif
  TEST(fcompare(angledistsgn(PI_HALFf,PIf),-PI_HALFf))

  const float flt=FLT_EPSILON;
  __int32 fi = *(__int32*)(&flt);
  TEST(fsmall(*(float*)(&(--fi))))
  TEST(fsmall(*(float*)(&(++fi))))
  TEST(!fsmall(*(float*)(&(++fi))))
  const double dbl=DBL_EPSILON;
  __int64 di = *(__int64*)(&dbl);
  TEST(fsmall(*(double*)(&(--di))))
  TEST(fsmall(*(double*)(&(++di))))
  TEST(!fsmall(*(double*)(&(++di))))
  
  TEST(fcompare(1.0f,1.0f))
  TEST(fcompare(1.0f,1.0f+FLT_EPSILON))
  TEST(fcompare(10.0f,10.0f+FLT_EPSILON*10))
  TEST(fcompare(10.0f,10.0f))
  TEST(!fcompare(0.1f, 0.1f+FLT_EPSILON*0.1f))
  TEST(!fcompare(0.1f, FLT_EPSILON))
  TEST(fcompare(1.0,1.0))
  TEST(fcompare(1.0,1.0+DBL_EPSILON))
  TEST(fcompare(10.0,10.0+DBL_EPSILON*10))
  TEST(fcompare(10.0,10.0))
  TEST(!fcompare(0.1, 0.1+DBL_EPSILON*0.1))
  TEST(!fcompare(0.1, DBL_EPSILON))

  // This tests our average aggregation formula, which lets you average extremely large numbers while maintaining a fair amount of precision.
  unsigned __int64 total=0;
  uint nc;
  double avg=0;
  double diff;
  for(nc = 1; nc < 10000;++nc)
  {
    total += nc*nc;
    avg=bssavg<double>(avg,(double)(nc*nc),nc);
    diff=bssmax(diff,fabs((total/(double)nc)-avg));
  }
  TEST(diff<FLT_EPSILON*2);

  // FastSqrt testing ground
  //
  //float a=2;
  //float b;
  //double sqrt_avg=0;
  //float NUMBERS[100000];
  ////srand(984753948);
  //for(uint i = 0; i < 100000; ++i)
  //  NUMBERS[i]=RANDFLOATGEN(2,4);

  //char p=_debug.OpenProfiler();
  //CPU_Barrier();
  //for(uint j = 0; j < 10; ++j)
  //{
  //for(uint i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=std::sqrtf(a);
  //}
  ///*for(uint i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrtsse(a);
  //}*/
  ///*for(uint i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrt(a);
  //}*/
  //}
  //CPU_Barrier();
  //sqrt_avg=_debug.CloseProfiler(p);
  //
  //TEST(b==a); //keep things from optimizing out
  //cout << sqrt_avg << std::endl;
  //CPU_Barrier();
  double ddbl = fabs(FastSqrt(2.0) - sqrt(2.0));
#ifndef BSS_DEBUG
  TEST(fabs(FastSqrt(2.0f) - sqrt(2.0f))<=FLT_EPSILON*2);
#else
  TEST(fabs(FastSqrt(2.0f) - sqrt(2.0f))<=FLT_EPSILON);
#endif
  TEST(fabs(FastSqrt(2.0) - sqrt(2.0))<=(DBL_EPSILON*100)); // Take note of the 100 epsilon error here on the fastsqrt for doubles.
  uint nmatch;
  for(nmatch = 1; nmatch < 200000; ++nmatch)
    if(FastSqrt(nmatch)!=(uint)std::sqrtl(nmatch))
      break;
  TEST(nmatch==200000);

  TEST(fFastRound(5.0f)==5);
  TEST(fFastRound(5.0000000001f)==5);
  TEST(fFastRound(4.999999999f)==5);
  TEST(fFastRound(4.5f)==4);
  TEST(fFastRound(5.5f)==6);
  TEST(fFastRound(5.9f)==6);

  TEST(fFastDoubleRound(5.0)==(int)5.0);
  TEST(fFastDoubleRound(5.0000000001f)==(int)5.0000000001f);
  TEST(fFastDoubleRound(4.999999999f)==(int)4.999999999f);
  TEST(fFastDoubleRound(4.5f)==(int)4.5f);
  //TEST(fFastDoubleRound(5.9f)==(int)5.9f); //This test fails, so don't use fFastDoubleRound for precision-critical anything.

  TEST(fcompare(distsqr(2.0f,2.0f,5.0f,6.0f),25.0f));
  TEST(fcompare(dist(2.0f,2.0f,5.0f,6.0f),5.0f,40));
  TEST(fcompare(distsqr(2.0,2.0,5.0,6.0),25.0));
  TEST(fcompare(dist(2.0,2.0,5.0,6.0),5.0,(__int64)150000)); // Do not use this for precision-critical anything.
  TEST(distsqr(2,2,5,6)==5*5); 
  TEST(dist(2,2,5,6)==5); // Yes, you can actually do distance calculations using integers, since we use FastSqrt's integer extension.

  __int64 stuff=2987452983472384720;
  unsigned short find=43271;
  TEST(bytesearch(&stuff,8,&find,1)==(((char*)&stuff)+3));
  TEST(bytesearch(&stuff,8,&find,2)==(((char*)&stuff)+3));
  TEST(bytesearch(&stuff,5,&find,1)==(((char*)&stuff)+3));
  TEST(bytesearch(&stuff,5,&find,2)==(((char*)&stuff)+3));
  TEST(bytesearch(&stuff,4,&find,1)==(((char*)&stuff)+3));
  TEST(!bytesearch(&stuff,4,&find,2));
  TEST(!bytesearch(&stuff,3,&find,2));
  TEST(!bytesearch(&stuff,0,&find,1));
  TEST(!bytesearch(&stuff,2,&find,3));
  find=27344;
  TEST(bytesearch(&stuff,2,&find,2));
  find=41;
  TEST(bytesearch(&stuff,8,&find,1)==(((char*)&stuff)+7));

  testbitcount<unsigned char>(__testret);
  testbitcount<unsigned short>(__testret);
  testbitcount<unsigned int>(__testret);
  testbitcount<unsigned __int64>(__testret);

  for(nmatch = 1; nmatch < 200000; ++nmatch)
  {
    uint test=std::log((double)nmatch);
    if(log2(nmatch)!=(uint)(std::log((double)nmatch)/std::log(2.0)))
      break;
  }
  TEST(nmatch==200000);
  for(nmatch = 2; nmatch < INT_MAX; nmatch <<= 1) // You have to do INT_MAX here even though its unsigned, because 10000... is actually less than 1111... and you overflow.
  {
    if(log2_p2(nmatch)!=(uint)(std::log((double)nmatch)/std::log(2.0)))
      break;
  }
  TEST(nmatch==(1<<31));
  
  TEST(fcompare(lerp<double>(3,4,0.5),3.5))
  TEST(fcompare(lerp<double>(3,4,0),3.0))
  TEST(fcompare(lerp<double>(3,4,1),4.0))
  TEST(fsmall(lerp<double>(-3,3,0.5)))
  TEST(fcompare(lerp<double>(-3,-4,0.5),-3.5))
  TEST(fcompare(lerp<float>(3,4,0.5f),3.5f))
  TEST(fcompare(lerp<float>(3,4,0),3.0f))
  TEST(fcompare(lerp<float>(3,4,1),4.0f))
  TEST(fsmall(lerp<float>(-3,3,0.5f)))
  TEST(fcompare(lerp<float>(-3,-4,0.5f),-3.5f))

  ENDTEST;
}

TEST::RETPAIR test_bss_DEBUGINFO()
{
  BEGINTEST;
  std::stringstream ss;
  std::fstream fs;
  std::wstringstream wss;
  fs.open(L"黑色球体工作室.log");
  auto tf = [&](bss_DebugInfo& di) {
    TEST(di.CloseProfiler(di.OpenProfiler()) < 500000);
    //TEST(cStrW(di.ModulePath(0)).compare(di.ModulePathW(0))==0);
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
    //di.AddTarget(wss);

    di.GetStream() << L"黑色球体工作室";
    di.GetStream() << "Black Sphere Studios";
    di.ClearTargets();
    di.GetStream() << L"黑色球体工作室";
  };

  bss_DebugInfo a(L"黑色球体工作室.txt",&ss); //Supposedly 黑色球体工作室 is Black Sphere Studios in Chinese, but the literal translation appears to be Black Ball Studio. Oh well.
  bss_DebugInfo b("logtest.txt");
  b.AddTarget(fs);
  bss_DebugInfo c;
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
  //for_all(r,[&a](int x, int y)->bool { return binsearch_before<int,uint,CompT<int>>(a,x)==y; },v,u);
  
  //for_all(r,[&a](int x, int y)->bool { return binsearch_after<int,uint,CompT<int>>(a,x)==(lower_bound(std::begin(a),std::end(a),x)-a); },v,u2);
  //for_all([&__testret](bool __x){ ++__testret.first; if(__x) ++__testret.second; },r);
  ENDTEST;
}

template<class T, typename P, int MAXSIZE>
void TEST_ALLOC_FUZZER(TEST::RETPAIR& __testret)
{
  cDynArray<cArraySimple<std::pair<P*,size_t>>> plist;
  for(int k=0; k<10; ++k)
  {
    T _alloc;
    for(int j=0; j<10; ++j)
    {
      bool pass=true;
      for(int i = 0; i < 10000; ++i)
      {
        if(RANDINTGEN(0,10)<5 || plist.Length()<3)
        {
          size_t sz = bssmax(RANDINTGEN(0,MAXSIZE),1); //Weird trick to avoid division by zero but still restrict it to [1,1]
          P* test=_alloc.alloc(sz);
          *test=0xFB;
          plist.Add(std::pair<P*,size_t>(test,sz));
        }
        else
        {
          int index=RANDINTGEN(0,plist.Length());
          if(plist[index].first[0]!=(P)0xFB)
            pass=false;
          _alloc.dealloc(plist[index].first);
          rswap(plist.Back(),plist[index]);
          plist.RemoveLast(); // This little technique lets us randomly remove items from the array without having to move large chunks of data by swapping the invalid element with the last one and then removing the last element (which is cheap)
        }
      }
      TEST(pass);
      plist.Clear(); // BOY I SHOULD PROBABLY CLEAR THIS BEFORE I PANIC ABOUT INVALID MEMORY ALLOCATIONS, HUH?
      _alloc.Clear();
    }
  }
}

TEST::RETPAIR test_bss_ALLOC_ADDITIVE_FIXED()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<cAdditiveFixedAllocator<int>,int,1>(__testret);
  ENDTEST;
}

template<class T>
struct ADDITIVEVARIABLEALLOCATORWRAP : cAdditiveVariableAllocator { inline T* BSS_FASTCALL alloc(size_t num) { return cAdditiveVariableAllocator::alloc<T>(num); } };

TEST::RETPAIR test_bss_ALLOC_ADDITIVE_VARIABLE()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<ADDITIVEVARIABLEALLOCATORWRAP<char>,char,4000>(__testret);
  ENDTEST;
}
TEST::RETPAIR test_bss_ALLOC_FIXED_SIZE()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<cFixedSizeAllocator<__int64>,__int64,1>(__testret);
  ENDTEST;
}

template<class T>
struct FIXEDCHUNKALLOCWRAP : cFixedChunkAlloc<T> { void Clear() { } };

TEST::RETPAIR test_bss_ALLOC_FIXED_CHUNK()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<FIXEDCHUNKALLOCWRAP<size_t>,size_t,1>(__testret);
  ENDTEST;
}
TEST::RETPAIR test_bss_deprecated()
{
  std::vector<bool> test;
  BEGINTEST;
  __time64_t tmval=FTIME(NULL);
  TEST(tmval!=0);
  FTIME(&tmval);
  TEST(tmval!=0);
  tm tms;
  TEST([&]()->bool { GMTIMEFUNC(&tmval,&tms); return true; }())
  //TESTERR(GMTIMEFUNC(0,&tms));
  //char buf[12];
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

  char buf[256];
  buf[9]=0;
  STRNCPY(buf,11,PANGRAM,10);
  wchar_t wbuf[256];
  wbuf[9]=0;
  WCSNCPY(wbuf,11,PANGRAMS[3],10);
  
  STRCPY(buf,256,PANGRAM);
  WCSCPY(wbuf,256,PANGRAMS[2]);
  STRCPYx0(buf,PANGRAM);
  WCSCPYx0(wbuf,PANGRAMS[4]);

  TEST(!STRICMP("fOObAr","Foobar"));
  TEST(!WCSICMP(L"Kæmi ný",L"kæmi ný"));

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

  FixedPt<13> fp(23563.2739);
  float res=fp;
  fp+=27.9;
  res+=27.9;
  TEST(fcompare(res,fp));
  res=fp;
  fp-=8327.9398437;
  res-=8327.9398437;
  TEST(fcompare(res,fp));
  res=fp;
  fp*=6.847399;
  res*=6.847399;
  TEST(fcompare(res,fp,215)); // We start approaching the edge of our fixed point range here so things predictably get out of whack
  res=fp;
  fp/=748.9272;
  res/=748.9272;
  TEST(fcompare(res,fp,6));
  ENDTEST;
}

TEST::RETPAIR test_ALIASTABLE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_ARRAYCIRCULAR()
{
  BEGINTEST;
  cArrayCircular<int> a;
  a.SetSize(25);
  TEST(a.Size()==25);
  for(int i = 0; i < 25; ++i)
    a.Push(i);
  TEST(a.Length()==25);
  
  TEST(a.Pop()==24);
  TEST(a.Pop()==23);
  a.Push(987);
  TEST(a.Pop()==987);
  TEST(a.Length()==23);
  a.Push(23);
  a.Push(24);
  for(int i = 0; i < 50; ++i)
    TEST(a[i]==(24-(i%25)));
  for(int i = 1; i < 50; ++i)
    TEST(a[-i]==((i-1)%25));
  a.Push(25); //This should overwrite 0
  TEST(a[0]=25);  
  TEST(a[-1]=1);  

  //const cArrayCircular<int>& b=a;
  //b[0]=5; // Should cause error

  ENDTEST;
}

template<bool SAFE> struct DEBUG_CDT_SAFE {};
template<> struct DEBUG_CDT_SAFE<false> {};
template<> struct DEBUG_CDT_SAFE<true>
{
  DEBUG_CDT_SAFE(const DEBUG_CDT_SAFE& copy) : __testret(*_testret) { isdead=this; }
  DEBUG_CDT_SAFE() : __testret(*_testret) { isdead=this; }
  ~DEBUG_CDT_SAFE() { TEST(isdead==this) }

  inline DEBUG_CDT_SAFE& operator=(const DEBUG_CDT_SAFE& right) { return *this; }
  
  static TEST::RETPAIR* _testret;
  TEST::RETPAIR& __testret;
  DEBUG_CDT_SAFE* isdead;
};
TEST::RETPAIR* DEBUG_CDT_SAFE<true>::_testret=0;

template<bool SAFE=true>
struct DEBUG_CDT : DEBUG_CDT_SAFE<SAFE> {
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
int DEBUG_CDT<true>::count=0;
int DEBUG_CDT<false>::count=0;

TEST::RETPAIR test_ARRAYSIMPLE()
{
  BEGINTEST;
  DArray<int>::t a(5);
  TEST(a.Size()==5);
  a.Insert(5,2);
  TEST(a.Size()==6);
  TEST(a[2]==5);
  a.Remove(1);
  TEST(a[1]==5);
  a.SetSize(10);
  TEST(a[1]==5);
  TEST(a.Size()==10);

  {
    DEBUG_CDT_SAFE<true>::_testret=&__testret;
    DEBUG_CDT<true>::count=0;
    DArray<DEBUG_CDT<true>>::tSafe b(10);
    b.RemoveShrink(5);
    TEST(b.Size()==9);
    TEST(DEBUG_CDT<true>::count == 9);
    b.SetSize(19);
    TEST(DEBUG_CDT<true>::count == 19);
    TEST(b.Size()==19);
  }
  TEST(!DEBUG_CDT<true>::count);

  {
    DEBUG_CDT<false>::count=0;
    DArray<DEBUG_CDT<false>>::tSafe b(10);
    b.RemoveShrink(5);
    TEST(b.Size()==9);
    TEST(DEBUG_CDT<false>::count == 9);
    b.SetSize(19);
    TEST(DEBUG_CDT<false>::count == 19);
    TEST(b.Size()==19);
  }
  TEST(!DEBUG_CDT<false>::count);

  ENDTEST;
}

struct FWDTEST {
  FWDTEST& operator=(const FWDTEST& right) { return *this; }
  FWDTEST& operator=(FWDTEST&& right) { return *this; }
};

TEST::RETPAIR test_ARRAYSORT()
{
  BEGINTEST;
  
  {
  cArraySort<DEBUG_CDT<true>,CompT<DEBUG_CDT<true>>,unsigned int,cArraySafe<DEBUG_CDT<true>,unsigned int>> arrtest;
  arrtest.Insert(DEBUG_CDT<true>(0));
  arrtest.Insert(DEBUG_CDT<true>(1));
  arrtest.Insert(DEBUG_CDT<true>(2));
  arrtest.Remove(2);
  arrtest.Insert(DEBUG_CDT<true>(3));
  arrtest.Insert(DEBUG_CDT<true>(4));
  arrtest.Insert(DEBUG_CDT<true>(5));
  arrtest.Remove(0);
  arrtest.Insert(DEBUG_CDT<true>(6));
  arrtest.Remove(3);

  TEST(arrtest[0]==1);
  TEST(arrtest[1]==3);
  TEST(arrtest[2]==4);
  TEST(arrtest[3]==6);

  cArraySort<DEBUG_CDT<true>,CompT<DEBUG_CDT<true>>,unsigned int,cArraySafe<DEBUG_CDT<true>,unsigned int>> arrtest2;
  arrtest2.Insert(DEBUG_CDT<true>(7));
  arrtest2.Insert(DEBUG_CDT<true>(8));
  arrtest=arrtest2;
  }
  TEST(!DEBUG_CDT<true>::count)

  ENDTEST;
}

TEST::RETPAIR test_AVLTREE()
{
  BEGINTEST;

  Allocator<AVL_Node<int,int>,FixedChunkPolicy<AVL_Node<int,int>>> fixedavl;
  cAVLtree<int, int,CompT<int>,Allocator<AVL_Node<int,int>,FixedChunkPolicy<AVL_Node<int,int>>>> avlblah(&fixedavl);

  //char prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Insert(testnums[i],testnums[i]);
  //std::cout << _debug.CloseProfiler(prof) << std::endl;

  shuffle(testnums);
  //prof=_debug.OpenProfiler();
  uint c=0;
  for(int i = 0; i<TESTNUM; ++i)
    c+=(avlblah.GetRef(testnums[i])!=0);
  TEST(c==TESTNUM);
  //std::cout << _debug.CloseProfiler(prof) << std::endl;
  
  shuffle(testnums);
  //prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Remove(testnums[i]);
  //std::cout << _debug.CloseProfiler(prof) << std::endl;
  avlblah.Clear();

  c=0;
  for(int i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c+=(avlblah.GetRef(testnums[i])==0);
  TEST(c==TESTNUM);

  cAVLtree<int, std::pair<int,int>*>* tree = new cAVLtree<int, std::pair<int,int>*>();
  std::pair<int,int> test(5,5);
  tree->Insert(test.first,&test);
  tree->Get(test.first);
  tree->ReplaceKey(5,2);
  tree->Remove(test.first);
  tree->Clear();

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
    TESTCOUNTALL(count,a[i]==b[i]);
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

TEST::RETPAIR test_BITARRAY()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_BITFIELD()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_BSS_STACK()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_BYTEQUEUE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_CMDLINEARGS()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_DYNARRAY()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_HIGHPRECISIONTIMER()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_HOLDER()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_INIPARSE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_INISTORAGE()
{
  BEGINTEST;

  cINIstorage ini("test.ini");
  while(ini.RemoveSection("NEWSECTION"));
  ini.AddSection("NEWSECTION");
  ini.AddSection("NEWSECTION");
  ini.EditEntry("NEWSECTION","newkey","fakevalue",0,-1);
  ini.EditEntry("NEWSECTION","newkey","realvalue",0,0);
  ini.EditEntry("NEWSECTION","newkey",0,0,0);
  ini.EditEntry("NEWSECTION","newkey","value",-1,-1);
  const char* test = ini["Video"]["baseheight"].GetString();
  ini.EditEntry("Video","test","fail",-1,-1);
  ini.EditEntry("Video","test","success");

  cStrW testerstr("test");
  cStr tester2str("%s%i","test",2);
  cStrW wtester2str(L"%s%i",L"test",2);
  wtester2str+=tester2str;

  const std::vector<std::pair<cStr,unsigned int>>& sections=ini.BuildSectionList();
  ini.EndINIEdit();

  /*for(unsigned int i=0; i < sections.size(); ++i)
  {
    std::cout << '[' << sections[i].first << ':' << sections[i].second  << ']' << std::endl;
    const std::vector<std::pair<std::pair<cStr,cStr>,unsigned int>>& entries=ini.GetSection(sections[i].first,sections[i].second)->BuildEntryList();
    for(unsigned int j=0; j < entries.size(); ++j)
      std::cout << entries[j].first.first << " - " << entries[j].first.second << " :" << entries[j].second << std::endl;
    std::cout << std::endl;
  }*/

  cINIstorage store("test.ini");
  //cINIstorage store;
  store.AddSection("Hello");

  int get = store.GetEntry("Asdkj", "");
  ENDTEST;
}

TEST::RETPAIR test_INTERVALTREE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_KHASH()
{
  BEGINTEST;
  //cKhash<int, char,false,KH_INT_HASHFUNC,KH_INT_EQUALFUNC<int>,KH_INT_VALIDATEPTR<int>> hashtest;
  //hashtest.Insert(21354,0);
  //hashtest.Insert(34623,0);
  //hashtest.Insert(52,0);
  //hashtest.Insert(1,0);
  //int r=hashtest.GetIterKey(hashtest.GetIterator(1));
  cKhash_Int<bss_DebugInfo*> hasherint;
  hasherint.Insert(25, &_debug);
  hasherint.GetKey(25);
  hasherint.Remove(25);
  cKhash_StringIns<bss_DebugInfo*> hasher;
  int iter = hasher.Insert("",&_debug);
  iter = hasher.Insert("Video",(bss_DebugInfo*)5);
  hasher.SetSize(100);
  iter = hasher.Insert("Physics",0);
  bss_DebugInfo* check = *hasher.GetKey("Video");
  check = *hasher.GetKey("Video");
  //unsigned __int64 diff = _debug.CloseProfiler(ID);

  ENDTEST;
}

TEST::RETPAIR test_LAMBDASTACK()
{
  BEGINTEST;
  cLambdaStack<void(void)> ls;
  auto l = [&](){ ls.Clear(); };
  std::function<void(void)> lf(l);
  ls.DeferLambda([&](){ ls.Clear(); });
  ls.DeferLambda(l);
  ls.DeferLambda(lf);
  ls.DeferLambda(std::function<void(void)>(l));
  ls.DeferLambda(&SetWorkDirToCur);

  ENDTEST;
}

TEST::RETPAIR test_LINKEDARRAY()
{
  BEGINTEST;
  ENDTEST;
}

bool cmplist(cLinkedList<int,Allocator<cLLNode<int>>,true>& list, const char* nums)
{
 // cLLIter<int> cur(list.GetRoot());
  auto cur = list.begin();
  bool r=true;
  while(cur.IsValid() && *nums!=0 && r)
    r=(*(cur++)==(*(nums++) - '0'));
  return r;
}

TEST::RETPAIR test_LINKEDLIST()
{
  BEGINTEST;
  cLinkedList<int,Allocator<cLLNode<int>>,true> test;
  cLLNode<int>* llp[5];

  llp[0] = test.Add(1);
  TEST(cmplist(test,"1"));
  llp[1] = test.Add(2);
  TEST(cmplist(test,"12"));
  llp[3] = test.Add(4);
  TEST(cmplist(test,"124"));
  llp[4] = test.Add(5);
  TEST(cmplist(test,"1245"));
  llp[2] = test.Insert(3,llp[3]);
  TEST(cmplist(test,"12345"));
  test.Remove(llp[3]);
  TEST(cmplist(test,"1235"));
  test.Remove(llp[0]);
  TEST(cmplist(test,"235"));
  test.Remove(llp[4]);
  TEST(cmplist(test,"23"));
  test.Insert(0,0);
  TEST(cmplist(test,"230"));
  TEST(test.Length()==3);
  ENDTEST;
}

TEST::RETPAIR test_LOCKLESSBYTEQUEUE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_MAP()
{
  BEGINTEST;
  cMap<int,uint> test;
  test.Clear();
  int ins[] = { 0,5,6,237,289,12,3 };
  int get[] = { 0,3,5,6,12 };
  uint res[] = { 0,6,1,2,5 };
  uint count=0;
  TESTARRAY(ins,return test.Insert(ins[i],count++)!=-1;);
  sort(std::begin(ins),std::end(ins));
  for(unsigned int i = 0; i < test.Length(); ++i)
  { TEST(test.KeyIndex(i)==ins[i]); }
  for(int i = 0; i < sizeof(get)/sizeof(int); ++i)
  { TEST(test[test.Get(get[i])]==res[i]); }

  TEST(test.Remove(0)==0);
  TEST(test.Get(0)==-1);
  TEST(test.Length()==((sizeof(ins)/sizeof(int))-1));
  
  cMap<int,FWDTEST> tst;
  tst.Insert(0,FWDTEST());
  FWDTEST lval;
  tst.Insert(1,lval);
  ENDTEST;
}

TEST::RETPAIR test_MUTEX()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_OBJSWAP()
{
  BEGINTEST;
  
  unsigned int vals[] = { 0,1,2,3,4,5 };
  const char* strs[] = { "001", "002", "003", "004", "005" };
  unsigned int* zp=vals+0;
  unsigned int* zp2=vals+1;
  unsigned int* zp3=vals+2;
  unsigned int* zp4=vals+3;
  unsigned int* zp5=vals+4;
  for(uint i = 0; i < 5; ++i)
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

    switch(WCSSWAP(PANGRAMS[i],5,PANGRAMS[4],PANGRAMS[3],PANGRAMS[2],PANGRAMS[1],PANGRAMS[0]))
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
  }

  ENDTEST;
}

TEST::RETPAIR test_PRIORITYQUEUE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_RATIONAL()
{
  BEGINTEST;
  cRational<int> tr(1,10);
  cRational<int> tr2(1,11);
  cRational<int> tr3(tr+tr2);
  TEST(tr.N()==1 && tr.D()==10);
  TEST(tr2.N()==1 && tr2.D()==11);
  TEST(tr3.N()==21 && tr3.D()==110);
  tr3=(tr-tr2);
  TEST(tr3.N()==1 && tr3.D()==110);
  tr3=(tr*tr2);
  TEST(tr3.N()==1 && tr3.D()==110);
  tr3=(tr/tr2);
  TEST(tr3.N()==11 && tr3.D()==10);
  tr3=(tr+3);
  TEST(tr3.N()==31 && tr3.D()==10);
  tr3=(tr-3);
  TEST(tr3.N()==-29 && tr3.D()==10);
  tr3=(tr*3);
  TEST(tr3.N()==3 && tr3.D()==10);
  tr3=(tr/3);
  TEST(tr3.N()==1 && tr3.D()==30);
  TEST((tr<3));
  TEST(!(tr>3));
  TEST(!(tr<tr2));
  TEST((tr>tr2));
  TEST(!(tr==3));
  TEST((tr!=3));
  TEST(!(tr==tr2));
  TEST((tr!=tr2));
  ENDTEST;
}

TEST::RETPAIR test_RBT_LIST()
{
  BEGINTEST;
  Allocator<cRBT_Node<int,int>,FixedChunkPolicy<cRBT_Node<int,int>>> fixedalloc;
  cRBT_List<int, int,CompT<int>,Allocator<cRBT_Node<int,int>,FixedChunkPolicy<cRBT_Node<int,int>>>> blah(&fixedalloc);

  //char prof=_debug.OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    //blah.Insert(testnums[i],testnums[i]);
    blah.Insert((i&0x01)?5:testnums[i],testnums[i]);
  //std::cout << _debug.CloseProfiler(prof) << std::endl;

  shuffle(testnums);
  //prof=_debug.OpenProfiler();
  int num=0;

  for(int i = 0; i<TESTNUM; ++i)
  {
  _ReadWriteBarrier();
    //seed+=blah.Get(testnums[i])->_data;
    num+=blah.Get(5)->_data;
  _ReadWriteBarrier();
  }

  //std::cout << _debug.CloseProfiler(prof) << std::endl;
  /*cRBT_PNode<int,int>* pnode=blah.GetFirst();
  int last=-1;
  uint pass=0;
  while(pnode)
  {
    if(pnode->GetData()<=last)
      pass+=1;
    last=pnode->GetData();
    pnode=pnode->GetNext();
  }
  TEST(!pass);*/
  ENDTEST;
}

TEST::RETPAIR test_REFCOUNTER()
{
  BEGINTEST;
  ENDTEST;
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

TEST::RETPAIR test_SETTINGS()
{
  BEGINTEST;
  cSettingManage<1,0>::LoadAllFromINI(cINIstorage("test.ini"));
  cSettingManage<1,0>::SaveAllToINI(cINIstorage("test.ini"));
  ENDTEST;
}

TEST::RETPAIR test_SINGLETON()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_STR()
{
  BEGINTEST;
  cStr sdfhold("blah");
  cStr sdfderp(sdfhold+cStr("temp")+cStr("temp")+cStr("temp")+cStr("temp"));
  ENDTEST;
}

TEST::RETPAIR test_STRTABLE()
{
  BEGINTEST;

  cStrTable<wchar_t> mbstable(PANGRAMS,6);
  cStrTable<wchar_t> wcstable(PANGRAMS,6);
  cStrTable<wchar_t,char> mbstable2(PANGRAMS,6);

  const wchar_t* stv = mbstable.GetString(0);
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
  ENDTEST;
}

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

TEST::RETPAIR test_TASKSTACK()
{
  BEGINTEST;
  
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
  ENDTEST;
}

TEST::RETPAIR test_UNIQUEPTR()
{
  BEGINTEST;

  cOwnerPtr<char> p(new char[56]);
  char* pp=p;
  cOwnerPtr<char> p2(p);
  cOwnerPtr<char> p3(std::move(p));
  TEST(pp==p3);
  ENDTEST;
}

TEST::RETPAIR test_FUNCTOR()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_LLBASE()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_LOCKLESS()
{
  BEGINTEST;
  ENDTEST;
}

TEST::RETPAIR test_OS()
{
  BEGINTEST;
  TEST(FolderExists("../bin"));
#ifdef BSS_PLATFORM_WIN32
  TEST(FolderExists(L"C:/windows/"));
#endif
  TEST(!FolderExists("abasdfwefs"));
  TEST(!FolderExists(L"abasdfwefs/alkjsdfs/sdfjkd/alkjsdfs/sdfjkd/alkjsdfs/sdfjkd/"));
  TEST(FileExists("blank.txt"));
  TEST(FileExists(L"blank.txt"));
  TEST(!FileExists("testaskdjlhfs.sdkj"));
  TEST(!FileExists(L"testaskdjlhfs.sdkj"));
  //TEST(FileExists("testlink"));
  //TEST(FileExists(L"testlink"));
  //TEST(FolderExists("IGNORE/symlink/"));
  //TEST(FolderExists(L"IGNORE/symlink/"));

#if defined(WIN32) || defined(_WINDOWS)

#endif
  ENDTEST;
}

TEST::RETPAIR test_STREAMSPLITTER()
{
  BEGINTEST;
  ENDTEST;
}

// --- Begin main testing function ---

int main(int argc, char** argv)
{
  ForceWin64Crash();
  SetWorkDirToCur();
  srand(time(NULL));
  
  for(int i = 0; i<TESTNUM; ++i)
    testnums[i]=i;
  shuffle(testnums);

  TEST tests[] = {
    { "bss_util.h", &test_bss_util },
    { "bss_DebugInfo.h", &test_bss_DEBUGINFO },
    { "bss_algo.h", &test_bss_algo },
    { "bss_alloc_additive.h:Fix", &test_bss_ALLOC_ADDITIVE_FIXED },
    { "bss_alloc_additive.h:Var", &test_bss_ALLOC_ADDITIVE_VARIABLE },
    { "bss_alloc_fixed.h:Size", &test_bss_ALLOC_FIXED_SIZE },
    { "bss_alloc_fixed.h:Chunk", &test_bss_ALLOC_FIXED_CHUNK },
    { "bss_depracated.h", &test_bss_deprecated },
    { "bss_fixedpt.h", &test_bss_FIXEDPT },
    { "cAliasTable.h", &test_ALIASTABLE },
    { "cArrayCircular.h", &test_ARRAYCIRCULAR },
    { "cArraySimple.h", &test_ARRAYSIMPLE },
    { "cArraySort.h", &test_ARRAYSORT },
    { "cAVLtree.h", &test_AVLTREE },
    { "cBinaryHeap.h", &test_BINARYHEAP },
    //{ "cBitArray.h", &test_BITARRAY },
    //{ "cBitField.h", &test_BITFIELD },
    //{ "cBSS_Stack.h", &test_BSS_STACK },
    //{ "cByteQueue.h", &test_BYTEQUEUE },
    //{ "cCmdLineArgs.h", &test_CMDLINEARGS },
    //{ "cDynArray.h", &test_DYNARRAY },
    //{ "cHighPrecisionTimer.h", &test_HIGHPRECISIONTIMER },
    //{ "cHolder.h", &test_HOLDER },
    //{ "INIparse.h", &test_INIPARSE },
    { "cINIstorage.h", &test_INISTORAGE },
    //{ "cIntervalTree.h", &test_INTERVALTREE },
    { "cKhash.h", &test_KHASH },
    { "cLambdaStack.h", &test_LAMBDASTACK },
    { "cLinkedArray.h", &test_LINKEDARRAY },
    { "cLinkedList.h", &test_LINKEDLIST },
    { "cLocklessByteQueue.h", &test_LOCKLESSBYTEQUEUE },
    { "cMap.h", &test_MAP },
    //{ "cMutex.h", &test_MUTEX },
    { "cObjSwap.h", &test_OBJSWAP },
    //{ "cPriorityQueue.h", &test_PRIORITYQUEUE },
    { "cRational.h", &test_RATIONAL },
    { "cRBT_List.h", &test_RBT_LIST },
    //{ "cRefCounter.h", &test_REFCOUNTER },
    { "cSettings.h", &test_SETTINGS },
    //{ "cSingleton.h", &test_SINGLETON },
    { "cStr.h", &test_STR },
    //{ "cStrTable.h", &test_STRTABLE },
    //{ "cTaskStack.h", &test_TASKSTACK },
    { "cUniquePtr.h", &test_UNIQUEPTR },
    //{ "functior.h", &test_FUNCTOR },
    //{ "LLBase.h", &test_LLBASE },
    //{ "lockless.h", &test_LOCKLESS },
    { "os.h", &test_OS },
    //{ "cStreamSplitter.h", &test_STREAMSPLITTER },
  };

  const size_t NUMTESTS=sizeof(tests)/sizeof(TEST);

  std::cout << "Black Sphere Studios - Utility Library v" << (uint)BSSUTIL_VERSION.Major << '.' << (uint)BSSUTIL_VERSION.Minor << '.' <<
    (uint)BSSUTIL_VERSION.Revision << ": Unit Tests\nCopyright (c)2012 Black Sphere Studios\n" << std::endl;
  const int COLUMNS[3] = { 24, 11, 8 };
  printf("%-*s %-*s %-*s\n",COLUMNS[0],"Test Name", COLUMNS[1],"Subtests", COLUMNS[2],"Pass/Fail");

  TEST::RETPAIR numpassed;
  std::vector<uint> failures;
  for(uint i = 0; i < NUMTESTS; ++i)
  {
    numpassed=tests[i].FUNC(); //First is total, second is succeeded
    if(numpassed.first!=numpassed.second) failures.push_back(i);

    printf("%-*s %*s %-*s\n",COLUMNS[0],tests[i].NAME, COLUMNS[1],cStr("%u/%u",numpassed.second,numpassed.first).c_str(), COLUMNS[2],(numpassed.first==numpassed.second)?"PASS":"FAIL");
  }

  if(failures.empty())
    std::cout << "\nAll tests passed successfully!" << std::endl;
  else
  {
    std::cout << "\nThe following tests failed: " << std::endl;
    for (uint i = 0; i < failures.size(); i++)
      std::cout << "  " << tests[failures[i]].NAME << std::endl;
    std::cout << "\nThese failures indicate either a misconfiguration on your system, or a potential bug. Please report all bugs to http://code.google.com/p/bss-util/issues/list\n\nA detailed list of failed tests was written to failedtests.txt" << std::endl;
  }

  std::cout << "\nPress Enter to exit the program." << std::endl;
  cin.get();

}

// --- The rest of this file is archived dead code ---

//void destroynode(std::pair<int,int>* data)
//{
//  delete data;
//}
//
//struct sp {
//  int x;
//  int y;
//};

//char numarray[] = {3,4,9,14,15,19,28,37,47,50,54,56,59,61,70,73,78,81,92,95,97,99 };
//char numarray[] = { 1, 2, 3, 4, 6 };

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
/*
int PI_ITERATIONS=500;
double pi=((PI_ITERATIONS<<1)-1)+(PI_ITERATIONS*PI_ITERATIONS);

const char* MBSTESTSTRINGS[] = { "test","test2","test3","test4","test5","test6" };
const wchar_t* WCSTESTSTRINGS[] = { L"test",L"test2",L"test3",L"test4",L"test5",L"test6" };

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
  cLLIter<int> cur(list.GetRoot());

  while(cur.IsValid())
    std::cout<<*(cur++);

  std::cout<<std::endl;
}

void printout(cLinkedArray<int>& list)
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

//	return 0;
//}

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
  bss_DebugInfo _debug("log.txt");
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
  

  //int id;
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

// This is painfully slow and I don't even know why its here.
inline bool isprime(int number)
{
  if(number%2==0) return number==2;
  int stop=number/2;
  for(int i = 3; i < stop; ++i)
    if(number%i==0) return false;
  return true;
}

//inline int addrecursive(int start,int prev)
//{
//  int retval=numarray[start]==prev?1:0;
//}

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

