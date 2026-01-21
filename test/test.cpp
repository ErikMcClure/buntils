// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/algo.h"
#include <time.h>
#include <iostream>
#include <filesystem>

#ifdef BUN_COMPILER_MSC
  #if defined(BUN_STATIC_LIB)
    #if defined(BUN_DEBUG) && defined(BUN_CPU_x86_64)
      #pragma comment(lib, "../bin/buntils_s_d.lib")
    #elif defined(BUN_CPU_x86_64)
      #pragma comment(lib, "../bin/buntils_s.lib")
    #elif defined(BUN_DEBUG)
      #pragma comment(lib, "../bin32/buntils32_s_d.lib")
    #else
      #pragma comment(lib, "../bin32/buntils32_s.lib")
    #endif
  #else
    #if defined(BUN_DEBUG) && defined(BUN_CPU_x86_64)
      #pragma comment(lib, "../bin/buntils_d.lib")
    #elif defined(BUN_CPU_x86_64)
      #pragma comment(lib, "../bin/buntils.lib")
    #elif defined(BUN_DEBUG)
      #pragma comment(lib, "../bin32/buntils32_d.lib")
    #else
      #pragma comment(lib, "../bin32/buntils32.lib")
    #endif
  #endif
#endif

#pragma warning(disable : 4566)
using namespace bun;

// --- Define global variables ---
const char* PANGRAM          = "The wizard quickly jinxed the gnomes before they vapourized.";
const bun_char* PANGRAMS[29] = {
  BUN__L("The wizard quickly jinxed the gnomes before they vapourized."),
  BUN__L("صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ"),           // Arabic
  BUN__L("Zəfər, jaketini də papağını da götür, bu axşam hava çox soyuq olacaq."), // Azeri
  BUN__L("Ах чудна българска земьо, полюшквай цъфтящи жита."),                     // Bulgarian
  BUN__L("Jove xef, porti whisky amb quinze glaçons d'hidrogen, coi!"),            // Catalan
  BUN__L("Příliš žluťoučký kůň úpěl ďábelské ódy."),                               // Czech
  BUN__L("Høj bly gom vandt fræk sexquiz på wc"),                                  // Danish
  BUN__L("Filmquiz bracht knappe ex-yogi van de wijs"),                            // Dutch
  BUN__L(
    "ཨ་ཡིག་དཀར་མཛེས་ལས་འཁྲུངས་ཤེས་བློའི་གཏེར༎ ཕས་རྒོལ་ཝ་སྐྱེས་ཟིལ་གནོན་གདོང་ལྔ་བཞིན༎ ཆགས་ཐོགས་ཀུན་བྲལ་མཚུངས་མེད་འཇམ་དབྱངསམཐུས༎ མཧཱ་མཁས་པའི་གཙོ་བོ་ཉིད་འགྱུར་ཅིག།"), // Dzongkha
  BUN__L("Eble ĉiu kvazaŭ-deca fuŝĥoraĵo ĝojigos homtipon."),                      // Esperanto
  BUN__L("Põdur Zagrebi tšellomängija-följetonist Ciqo külmetas kehvas garaažis"), // Estonian
  BUN__L("Törkylempijävongahdus"),                                                 // Finnish
  BUN__L("Falsches Üben von Xylophonmusik quält jeden größeren Zwerg"),            // German
  BUN__L("Τάχιστη αλώπηξ βαφής ψημένη γη, δρασκελίζει υπέρ νωθρού κυνός"),         // Greek
  BUN__L("כך התרסק נפץ על גוזל קטן, שדחף את צבי למים"),                            // Hebrew
  BUN__L(
    "दीवारबंद जयपुर ऐसी दुनिया है जहां लगभग हर दुकान का नाम हिन्दी में लिखा गया है। नामकरण की ऐसी तरतीब हिन्दुस्तान में कम दिखती है। दिल्ली में कॉमनवेल्थ गेम्स के दौरान कनॉट प्लेस और पहाड़गंज की नामपट्टिकाओं को एक समान करने का अभियान चला। पत्रकार लिख"), // Hindi
  BUN__L("Kæmi ný öxi hér, ykist þjófum nú bæði víl og ádrepa."), // Icelandic
  BUN__L(
    "いろはにほへと ちりぬるを わかよたれそ つねならむ うゐのおくやま けふこえて あさきゆめみし ゑひもせす（ん）"), // Japanese
  BUN__L("꧋ ꦲꦤꦕꦫꦏ꧈ ꦢꦠꦱꦮꦭ꧈ ꦥꦝꦗꦪꦚ꧈ ꦩꦒꦧꦛꦔ꧉"), // Javanese
  BUN__L(
    "    "), // Klingon
  BUN__L("키스의 고유조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다."),      // Korean
  BUN__L("သီဟိုဠ်မှ ဉာဏ်ကြီးရှင်သည် အာယုဝဍ္ဎနဆေးညွှန်းစာကို ဇလွန်ဈေးဘေးဗာဒံပင်ထက် အဓိဋ္ဌာန်လျက် ဂဃနဏဖတ်ခဲ့သည်။"), // Myanmar
  BUN__L(
    "بر اثر چنین تلقین و شستشوی مغزی جامعی، سطح و پایه‌ی ذهن و فهم و نظر بعضی اشخاص واژگونه و معکوس می‌شود.‏"), // Persian
  BUN__L(
    "À noite, vovô Kowalsky vê o ímã cair no pé do pingüim queixoso e vovó põe açúcar no chá de tâmaras do jabuti feliz."), // Portuguese
  BUN__L("Эх, чужак! Общий съём цен шляп (юфть) – вдрызг!"),      // Russian
  BUN__L("Fin džip, gluh jež i čvrst konjić dođoše bez moljca."), // Serbian
  BUN__L("Kŕdeľ ďatľov učí koňa žrať kôru."),                     // Slovak
  BUN__L(
    "เป็นมนุษย์สุดประเสริฐเลิศคุณค่า กว่าบรรดาฝูงสัตว์เดรัจฉาน จงฝ่าฟันพัฒนาวิชาการ อย่าล้างผลาญฤๅเข่นฆ่าบีฑาใคร ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า หัดอภัยเหมือนกีฬาอัชฌาสัย ปฏิบัติประพฤติกฎกำหนดใจ พูดจาให้จ๊ะๆ จ๋าๆ น่าฟังเอยฯ"), // Thai
  BUN__L(
    "ژالہ باری میں ر‌ضائی کو غلط اوڑھے بیٹھی قرۃ العین اور عظمٰی کے پاس گھر کے ذخیرے سے آناً فاناً ڈش میں ثابت جو، صراحی میں چائے اور پلیٹ میں زردہ آیا۔") // Urdu
};

const wchar_t* TESTUNICODESTR = L"الرِضَءَجيعُ بِهءَرِا نَجلاءَرِ رِمِعطارِ";

uint16_t testnums[TESTNUM] = { 0 };
Logger _failedtests;
volatile std::atomic<bool> startflag;

TESTDEF::RETPAIR* DEBUG_CDT_SAFE::_testret = 0;
int DEBUG_CDT_SAFE::count                  = 0;
int DEBUG_CDT_SAFE::ID                     = 0;
bun::Hash<int> DEBUG_CDT_SAFE::Tracker;

// #define BUN_ISOLATE_TEST 56

void shuffle_testnums() { Shuffle(const_cast<uint16_t(&)[50000]>(testnums)); }

// --- Begin main testing function ---
int main(int argc, char** argv)
{
  ForceWin64Crash();
  SetWorkDirToCur();
  _failedtests.AddTarget("failedtests.txt");
  uint64_t seed = (uint64_t)time(nullptr);
  seed          = 1686163994;
  bun_RandSeed(seed);
  // profile_ring_alloc();

  for(uint16_t i = 0; i < TESTNUM; ++i)
    testnums[i] = i;
  shuffle_testnums();

  // For best results on windows, add the test application to Application Verifier before going through the tests.
  TESTDEF tests[] = {
    { "AAtree.h", &test_AA_TREE },
    { "algo.h", &test_algo },
    { "AliasTable.h", &test_ALIASTABLE },
    { "BlockAlloc.h", &test_ALLOC_BLOCK },
    { "BlockAllocMT.h", &test_ALLOC_BLOCK_LOCKLESS },
    { "CacheAlloc.h", &test_ALLOC_CACHE },
    { "GreedyAlloc.h", &test_ALLOC_GREEDY },
    { "GreedyBlockAlloc.h", &test_ALLOC_GREEDY_BLOCK },
    { "Animation.h", &test_ANIMATION },
    { "Array.h", &test_ARRAY },
    { "ArrayCircular.h", &test_ARRAYCIRCULAR },
    { "ArraySort.h", &test_ARRAYSORT },
    { "AVLtree.h", &test_AVLTREE },
    { "BinaryHeap.h", &test_BINARYHEAP },
    { "BitField.h", &test_BITFIELD },
    { "BitStream.h", &test_BITSTREAM },
    { "buntils_c.h", &test_buntils_c },
    { "buntils.h", &test_buntils },
    { "Collision.h", &test_COLLISION },
    { "CompactArray.h", &test_COMPACTARRAY },
    { "depracated.h", &test_deprecated },
    { "Delegate.h", &test_DELEGATE },
    { "DisjointSet.h", &test_DISJOINTSET },
    { "Dual.h", &test_DUAL },
    { "DynArray.h", &test_DYNARRAY },
    { "FixedPt.h", &test_FIXEDPT },
    { "Geometry.h", &test_GEOMETRY },
    { "Graph.h", &test_GRAPH },
    { "Hash.h", &test_HASH },
    { "HighPrecisionTimer.h", &test_HIGHPRECISIONTIMER },
    { "INIstorage.h", &test_INISTORAGE },
    { "JSON.h", &test_JSON },
    { "KDTree.h", &test_KDTREE },
    { "LinkedArray.h", &test_LINKEDARRAY },
    { "LinkedList.h", &test_LINKEDLIST },
    { "literals.h", &test_LITERALS },
    { "lockless.h", &test_LOCKLESS },
    { "LocklessQueue.h", &test_LOCKLESSQUEUE },
    { "Log.h", &test_LOG },
    { "Map.h", &test_MAP },
    { "os.h", &test_OS },
    { "PriorityQueue.h", &test_PRIORITYQUEUE },
    { "profile.h", &test_PROFILE },
    { "Queue.h", &test_BUN_QUEUE },
    { "RandomQueue.h", &test_RANDOMQUEUE },
    { "Rational.h", &test_RATIONAL },
    { "RefCounter.h", &test_REFCOUNTER },
    { "RWLock.h", &test_RWLOCK },
    { "Scheduler.h", &test_SCHEDULER },
    { "Singleton.h", &test_SINGLETON },
    { "sseVec.h", &test_SSE },
    { "Stack.h", &test_BUN_STACK },
    { "Str.h", &test_STR },
    { "stream.h", &test_STREAM },
    { "StringTable.h", &test_STRTABLE },
    { "Thread.h", &test_THREAD },
    { "ThreadPool.h", &test_THREADPOOL },
    { "TOML.h", &test_TOML },
    { "TRBtree.h", &test_TRBTREE },
    { "Trie.h", &test_TRIE },
    { "UBJSON.h", &test_UBJSON },
    { "Variant.h", &test_VARIANT },
    { "vector.h", &test_VECTOR },
    { "XML.h", &test_XML },
  };

  // TODO: replace with std::print from C++23
  std::cout << std::vformat("Bunny Utility Library v{}.{}.{}: Unit Tests\nCopyright (c)2026 Erik McClure\nSeed: {}\n",
                            std::make_format_args(bun_Version.Major, bun_Version.Minor, bun_Version.Revision, seed))
            << std::endl;

  assert(bun_Version.version == ((uint64_t)bun_Version.Major << 48) + ((uint64_t)bun_Version.Minor << 32) +
                                  ((uint64_t)bun_Version.Revision << 16));

  std::cout << std::format("{: ^{}} {: >{}} {}", "Test Name", 24, "Subtests ", 12, "Pass/Fail") << std::endl;

  std::vector<size_t> failures;
#ifndef BUN_ISOLATE_TEST
  for(size_t i = 0; i < std::ranges::size(tests); ++i)
  {
#else
  {
    size_t i = BUN_ISOLATE_TEST;
#endif
    auto [atomic_total, atomic_passed] = tests[i].FUNC();
    auto [total, passed] =
      std::make_pair(atomic_total.load(std::memory_order_relaxed), atomic_passed.load(std::memory_order_relaxed));
    if(total != passed)
      failures.push_back(i);

    const int COL[3] = { 24, 16, 4 };
    auto ratio = std::format("{}/{}", passed, total);
    std::cout << std::vformat("{: <{}} {: ^{}} {: >{}}", std::make_format_args(tests[i].NAME, COL[0], ratio, COL[1],
                                                                               (total == passed) ? "PASS" : "FAIL", COL[2]))
              << std::endl;
  }

  if(failures.empty())
    std::cout << "\nAll tests passed successfully!" << std::endl;
  else
  {
    std::cout << "\nThe following tests failed (seed = " << seed << "): " << std::endl;
    for(size_t i = 0; i < failures.size(); i++)
      std::cout << "  " << tests[failures[i]].NAME << std::endl;
    std::cout
      << "\nThese failures indicate either a misconfiguration on your system, or a potential bug.\n\nA detailed list of failed tests was written to failedtests.txt"
      << std::endl;
  }

  std::cout << "\nPress Enter to exit the program." << std::endl;
  std::cin.get();

  return static_cast<int>(failures.size());
}