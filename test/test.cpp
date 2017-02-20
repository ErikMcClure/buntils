// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss_algo.h"
#include <time.h>

#ifdef BSS_COMPILER_MSC
#if defined(BSS_STATIC_LIB)
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/bss-util_s_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/bss-util_s.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../bin32/bss-util32_s_d.lib")
#else
#pragma comment(lib, "../bin32/bss-util32_s.lib")
#endif
#else
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/bss-util_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/bss-util.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../bin32/bss-util32_d.lib")
#else
#pragma comment(lib, "../bin32/bss-util32.lib")
#endif
#endif
#endif

#pragma warning(disable:4566)
using namespace bss_util;

// --- Define global variables ---
const char* PANGRAM = "The wizard quickly jinxed the gnomes before they vapourized.";
const bsschar* PANGRAMS[29] = {
  BSS__L("The wizard quickly jinxed the gnomes before they vapourized."),
  BSS__L("صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ"), //Arabic
  BSS__L("Zəfər, jaketini də papağını da götür, bu axşam hava çox soyuq olacaq."), //Azeri
  BSS__L("Ах чудна българска земьо, полюшквай цъфтящи жита."), //Bulgarian
  BSS__L("Jove xef, porti whisky amb quinze glaçons d'hidrogen, coi!"), //Catalan
  BSS__L("Příliš žluťoučký kůň úpěl ďábelské ódy."), //Czech
  BSS__L("Høj bly gom vandt fræk sexquiz på wc"), //Danish
  BSS__L("Filmquiz bracht knappe ex-yogi van de wijs"), //Dutch
  BSS__L("ཨ་ཡིག་དཀར་མཛེས་ལས་འཁྲུངས་ཤེས་བློའི་གཏེར༎ ཕས་རྒོལ་ཝ་སྐྱེས་ཟིལ་གནོན་གདོང་ལྔ་བཞིན༎ ཆགས་ཐོགས་ཀུན་བྲལ་མཚུངས་མེད་འཇམ་དབྱངསམཐུས༎ མཧཱ་མཁས་པའི་གཙོ་བོ་ཉིད་འགྱུར་ཅིག།"), //Dzongkha
  BSS__L("Eble ĉiu kvazaŭ-deca fuŝĥoraĵo ĝojigos homtipon."), //Esperanto
  BSS__L("Põdur Zagrebi tšellomängija-följetonist Ciqo külmetas kehvas garaažis"), //Estonian
  BSS__L("Törkylempijävongahdus"), //Finnish
  BSS__L("Falsches Üben von Xylophonmusik quält jeden größeren Zwerg"), //German
  BSS__L("Τάχιστη αλώπηξ βαφής ψημένη γη, δρασκελίζει υπέρ νωθρού κυνός"), //Greek
  BSS__L("כך התרסק נפץ על גוזל קטן, שדחף את צבי למים"), //Hebrew
  BSS__L("दीवारबंद जयपुर ऐसी दुनिया है जहां लगभग हर दुकान का नाम हिन्दी में लिखा गया है। नामकरण की ऐसी तरतीब हिन्दुस्तान में कम दिखती है। दिल्ली में कॉमनवेल्थ गेम्स के दौरान कनॉट प्लेस और पहाड़गंज की नामपट्टिकाओं को एक समान करने का अभियान चला। पत्रकार लिख"), //Hindi
  BSS__L("Kæmi ný öxi hér, ykist þjófum nú bæði víl og ádrepa."), //Icelandic
  BSS__L("いろはにほへと ちりぬるを わかよたれそ つねならむ うゐのおくやま けふこえて あさきゆめみし ゑひもせす（ん）"), //Japanese
  BSS__L("꧋ ꦲꦤꦕꦫꦏ꧈ ꦢꦠꦱꦮꦭ꧈ ꦥꦝꦗꦪꦚ꧈ ꦩꦒꦧꦛꦔ꧉"), //Javanese
  BSS__L("    "), //Klingon
  BSS__L("키스의 고유조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다."), //Korean
  BSS__L("သီဟိုဠ်မှ ဉာဏ်ကြီးရှင်သည် အာယုဝဍ္ဎနဆေးညွှန်းစာကို ဇလွန်ဈေးဘေးဗာဒံပင်ထက် အဓိဋ္ဌာန်လျက် ဂဃနဏဖတ်ခဲ့သည်။"), //Myanmar
  BSS__L("بر اثر چنین تلقین و شستشوی مغزی جامعی، سطح و پایه‌ی ذهن و فهم و نظر بعضی اشخاص واژگونه و معکوس می‌شود.‏"), //Persian
  BSS__L("À noite, vovô Kowalsky vê o ímã cair no pé do pingüim queixoso e vovó põe açúcar no chá de tâmaras do jabuti feliz."), //Portuguese
  BSS__L("Эх, чужак! Общий съём цен шляп (юфть) – вдрызг!"), //Russian
  BSS__L("Fin džip, gluh jež i čvrst konjić dođoše bez moljca."), //Serbian
  BSS__L("Kŕdeľ ďatľov učí koňa žrať kôru."), //Slovak
  BSS__L("เป็นมนุษย์สุดประเสริฐเลิศคุณค่า กว่าบรรดาฝูงสัตว์เดรัจฉาน จงฝ่าฟันพัฒนาวิชาการ อย่าล้างผลาญฤๅเข่นฆ่าบีฑาใคร ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า หัดอภัยเหมือนกีฬาอัชฌาสัย ปฏิบัติประพฤติกฎกำหนดใจ พูดจาให้จ๊ะๆ จ๋าๆ น่าฟังเอยฯ"), //Thai
  BSS__L("ژالہ باری میں ر‌ضائی کو غلط اوڑھے بیٹھی قرۃ العین اور عظمٰی کے پاس گھر کے ذخیرے سے آناً فاناً ڈش میں ثابت جو، صراحی میں چائے اور پلیٹ میں زردہ آیا۔") //Urdu
};

const wchar_t* TESTUNICODESTR = L"الرِضَءَجيعُ بِهءَرِا نَجلاءَرِ رِمِعطارِ";

uint16_t testnums[TESTNUM];
cLog _failedtests;
volatile std::atomic<bool> startflag;

TESTDEF::RETPAIR* DEBUG_CDT_SAFE::_testret = 0;
int DEBUG_CDT_SAFE::count = 0;
int DEBUG_CDT_SAFE::ID = 0;
bss_util::cHash<int> DEBUG_CDT_SAFE::Tracker;

//#define BSS_ISOLATE_TEST 2

// --- Begin main testing function ---
int main(int argc, char** argv)
{
  ForceWin64Crash();
  SetWorkDirToCur();
  _failedtests.AddTarget("failedtests.txt");
  uint64_t seed = (uint64_t)time(nullptr);
  //seed = 1425459123;
  bssrandseed(seed);

  //profile_ring_alloc();

  for(int i = 0; i<TESTNUM; ++i)
    testnums[i] = i;
  shuffle(testnums); 

  // For best results on windows, add the test application to Application Verifier before going through the tests.
  TESTDEF tests[] = {
    { "bss_util_c.h", &test_bss_util_c },
    { "bss_util.h", &test_bss_util },
    { "cLog.h", &test_bss_LOG },
    { "bss_algo.h", &test_bss_algo },
    { "bss_alloc_greedy.h", &test_bss_ALLOC_ADDITIVE },
    { "bss_alloc_circular.h", &test_bss_ALLOC_RING },
    { "bss_alloc_block.h", &test_bss_ALLOC_BLOCK },
    { "bss_alloc_block_MT.h", &test_bss_ALLOC_BLOCK_LOCKLESS },
    { "bss_depracated.h", &test_bss_deprecated },
    { "bss_dual.h", &test_bss_DUAL },
    { "bss_fixedpt.h", &test_bss_FIXEDPT },
    { "bss_sse.h", &test_bss_SSE },
    { "bss_stream.h", &test_STREAMSPLITTER },
    { "bss_graph.h", &test_bss_GRAPH },
    { "bss_vector.h", &test_VECTOR },
    { "cAliasTable.h", &test_ALIASTABLE },
    { "cAnimation.h", &test_ANIMATION },
    { "cFXAni.h", &test_FX_ANI },
    { "cArrayCircular.h", &test_ARRAYCIRCULAR },
    { "cArray.h", &test_ARRAY },
    { "cArraySort.h", &test_ARRAYSORT },
    { "cAVLtree.h", &test_AVLTREE },
    { "cAAtree.h", &test_AA_TREE },
    { "cBinaryHeap.h", &test_BINARYHEAP },
    { "cBitField.h", &test_BITFIELD },
    { "cBitStream.h", &test_BITSTREAM },
    { "cQueue.h", &test_BSS_QUEUE },
    { "cStack.h", &test_BSS_STACK },
    { "cDisjointSet.h", &test_DISJOINTSET },
    { "cDynArray.h", &test_DYNARRAY },
    { "cHighPrecisionTimer.h", &test_HIGHPRECISIONTIMER },
    { "cIDHash.h", &test_IDHASH },
    { "cScheduler.h", &test_SCHEDULER },
    { "cINIstorage.h", &test_INISTORAGE },
    { "cKDTree.h", &test_KDTREE },
    { "cJSON.h", &test_JSON },
    { "cUBJSON.h", &test_UBJSON },
    { "cHash.h", &test_HASH },
    { "cLinkedArray.h", &test_LINKEDARRAY },
    { "cLinkedList.h", &test_LINKEDLIST },
    { "lockless.h", &test_LOCKLESS },
    { "cLocklessQueue.h", &test_LOCKLESSQUEUE },
    { "cMap.h", &test_MAP },
    { "cPriorityQueue.h", &test_PRIORITYQUEUE },
    { "cRational.h", &test_RATIONAL },
    { "cRefCounter.h", &test_REFCOUNTER },
    { "rwlock.h", &test_RWLOCK },
    { "cSingleton.h", &test_SINGLETON },
    { "cStr.h", &test_STR },
    { "cStrTable.h", &test_STRTABLE },
    { "cThread.h", &test_THREAD },
    { "cThreadPool.h", &test_THREADPOOL },
    { "cTOML.h", &test_TOML },
    { "cTRBtree.h", &test_TRBTREE },
    { "cTrie.h", &test_TRIE },
    { "cXML.h", &test_XML },
    { "cSmartPtr.h", &test_SMARTPTR },
    { "delegate.h", &test_DELEGATE },
    { "os.h", &test_OS },
    { "profile.h", &test_PROFILE },
    { "variant.h", &test_VARIANT },
  };

  const size_t NUMTESTS = sizeof(tests) / sizeof(TESTDEF);

  std::cout << "Black Sphere Studios - Utility Library v" << (uint32_t)BSSUTIL_VERSION.Major << '.' << (uint32_t)BSSUTIL_VERSION.Minor << '.' <<
    (uint32_t)BSSUTIL_VERSION.Revision << ": Unit Tests\nCopyright (c)2017 Black Sphere Studios\n" << std::endl;
  const int COLUMNS[3] = { 24, 11, 8 };
  printf("%-*s %-*s %-*s\n", COLUMNS[0], "Test Name", COLUMNS[1], "Subtests", COLUMNS[2], "Pass/Fail");

  TESTDEF::RETPAIR numpassed;
  std::vector<uint32_t> failures;
#ifndef BSS_ISOLATE_TEST
  for(uint32_t i = 0; i < NUMTESTS; ++i)
  {
#else
    {
      uint32_t i = BSS_ISOLATE_TEST;
#endif
      numpassed = tests[i].FUNC(); //First is total, second is succeeded
      if(numpassed.first != numpassed.second) failures.push_back(i);

      printf("%-*s %*s %-*s\n", COLUMNS[0], tests[i].NAME, COLUMNS[1], cStrF("%u/%u", numpassed.second, numpassed.first).c_str(), COLUMNS[2], (numpassed.first == numpassed.second) ? "PASS" : "FAIL");
    }

    if(failures.empty())
      std::cout << "\nAll tests passed successfully!" << std::endl;
    else
    {
      std::cout << "\nThe following tests failed (seed = " << seed << "): " << std::endl;
      for(uint32_t i = 0; i < failures.size(); i++)
        std::cout << "  " << tests[failures[i]].NAME << std::endl;
      std::cout << "\nThese failures indicate either a misconfiguration on your system, or a potential bug.\n\nA detailed list of failed tests was written to failedtests.txt" << std::endl;
    }

    std::cout << "\nPress Enter to exit the program." << std::endl;
    std::cin.get();

    return 0;
  }
