// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/sseVec.h"
#include "buntils/algo.h"

using namespace bun;

// This does not perform well at all
template<int M1, int M2, int M3, int M4>
inline int PriCompare(const int(&li)[4], const int(&ri)[4])
{
  sseVeci m(M1, M2, M3, M4); // positive mask
  sseVeci n(-M1, -M2, -M3, -M4); // negative mask
                                 //sseVeci l(l1,l2,l3,l4);
  sseVeci l(li);
  //sseVeci r(r1,r2,r3,r4);
  sseVeci r(ri);
  sseVeci t = ((l>r)&m) + ((l<r)&n);
  l = BUN_SSE_SHUFFLE_EPI32(t, 0x1B); // Assign t to l, but reversed
  l += t;
  t = BUN_SSE_CAST_PS_SI128(BUN_SSE_MOVEHL_PS(BUN_SSE_CAST_SI128_PS(t), BUN_SSE_CAST_SI128_PS(l))); // Move upper 2 ints to bottom 2 ints
  return BUN_SSE_SI128_SI32(t + l); // return bottom 32bit result
}

BUN_FORCEINLINE int PriComp(int l1, int l2, int l3, int l4, int r1, int r2, int r3, int r4)
{
  BUN_ALIGN(16) int li[4] = { l1,l2,l3,l4 };
  BUN_ALIGN(16) int ri[4] = { r1,r2,r3,r4 };
  return PriCompare<1, 2, 4, 8>(li, ri);
}

inline static uint32_t Interpolate(uint32_t l, uint32_t r, float c)
{
  //float inv=1.0f-c;
  //return ((uint32_t)(((l&0xFF000000)*inv)+((r&0xFF000000)*c))&0xFF000000)|
  //        ((uint32_t)(((l&0x00FF0000)*inv)+((r&0x00FF0000)*c))&0x00FF0000)|
  //       ((uint32_t)(((l&0x0000FF00)*inv)+((r&0x0000FF00)*c))&0x0000FF00)|
  //	    ((uint32_t)(((l&0x000000FF)*inv)+((r&0x000000FF)*c))&0x000000FF);
  //BUN_SSE_M128i xl = _mm_set1_epi32(l); // duplicate l 4 times in the 128-bit register (l,l,l,l)
  //BUN_SSE_M128i xm = _mm_set_epi32(0xFF000000,0x00FF0000,0x0000FF00,0x000000FF); // Channel masks (alpha,red,green,blue)
  //xl=_mm_and_si128(xl,xm); // l&mask
  //xl=_mm_shufflehi_epi16(xl,0xB1); // Now we have to Shuffle these values down because there is no way to convert an uint32_t to a float. In any instruction set. Ever.
  //BUN_SSE_M128 xfl = _mm_cvtepi32_ps(xl); // Convert to float
  //BUN_SSE_M128 xc = _mm_set_ps1(c); // (c,c,c,c)
  //BUN_SSE_M128 xinv = _mm_set_ps1(1.0f);  // (1.0,1.0,1.0,1.0)
  //xinv = _mm_sub_ps(xinv,xc); // (1.0-c,1.0-c,1.0-c,1.0-c)
  //xfl = _mm_mul_ps(xfl,xinv); // Multiply l by 1.0-c (inverted factor)
  //BUN_SSE_M128i xr = _mm_set1_epi32(r); // duplicate r 4 times across the 128-bit register (r,r,r,r)
  //xr=_mm_and_si128(xr,xm); // r & mask
  //xr=_mm_shufflehi_epi16(xr,0xB1); // Do the same shift we did on xl earlier so they match up
  //BUN_SSE_M128 xrl = _mm_cvtepi32_ps(xr); // convert to float
  //xrl = _mm_mul_ps(xrl,xc); // Multiply r by c
  //xfl = _mm_add_ps(xfl,xrl); // Add l and r
  //xl = _mm_cvttps_epi32(xfl); // Convert back to integer
  //xl=_mm_shufflehi_epi16(xl,0xB1); // Shuffle the last two back up (this is actually the same Shuffle, since before we just swapped locations, so we swap locations again and then we're back where we started).
  //xl = _mm_and_si128(xl,xm); // l&mask
  //xr = xl;
  //xr = _mm_shuffle_epi32(xr,0x1B); // Reverses the order of xr so we now have (d,c,b,a)
  //xl = _mm_or_si128(xl,xr); // Or xl and xr so we get (d|a,c|b,b|c,a|d) in xl
  //xr = _mm_castps_si128(_mm_movehl_ps(BUN_SSE_CAST_SI128_PS(xr),BUN_SSE_CAST_SI128_PS(xl))); // Move upper 2 ints to bottom 2 ints in xr so xr = (d,c,d|a,c|b)
  //xl = _mm_or_si128(xl,xr); // Now or them again so we get (d|a,c|b,b|c | d|a,a|d | c|b) which lets us take out the bottom integer as our result

  sseVeci xl(l); // duplicate l 4 times in the 128-bit register (l,l,l,l)
  sseVeci xm(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000); // Channel masks (alpha,red,green,blue), these are loaded in reverse order.
  xl = sseVeci::ShuffleHi<0xB1>(xl&xm); // Now we have to Shuffle (l&m) down because there is no way to convert an uint32_t xmm register to a float. In any instruction set. Ever.
  sseVec xc(c); // (c,c,c,c)
  sseVeci xr(r); // duplicate r 4 times across the 128-bit register (r,r,r,r)
  xr = sseVeci::ShuffleHi<0xB1>(xr&xm); // Shuffle r down just like l
  xl = ((sseVec(xr)*xc) + (sseVec(xl)*(sseVec(1.0f) - xc))); //do the operation (r*c) + (l*(1.0-c)) across all 4 integers, converting to and from floating point in the process.
  xl = sseVeci::ShuffleHi<0xB1>(xl); // reverse our shuffling from before (this is actually the same Shuffle, since before we just swapped locations, so we swap locations again, and then we're back where we started).
  xl &= xm; // mask l with m again.
  xr = sseVeci::Shuffle<0x1B>(xl); // assign the values of xl to xr, but reversed, so we have (d,c,b,a)
  xl |= xr; // OR xl and xr so we get (d|a,c|b,b|c,a|d) in xl
  xr = BUN_SSE_CAST_PS_SI128(BUN_SSE_MOVEHL_PS(BUN_SSE_CAST_SI128_PS(xr), BUN_SSE_CAST_SI128_PS(xl))); // Move upper 2 ints to bottom 2 ints in xr so xr = (d,c,d|a,c|b)
  return (uint32_t)BUN_SSE_SI128_SI32(xl | xr); // Now OR them again so we get (d|a,c|b,b|c | d|a,a|d | c|b), then store the bottom 32-bit integer. What kind of fucked up name is _mm_cvtsi128_si32 anyway?
}

static uint32_t flttoint(const float(&ch)[4])
{
  //return (((uint32_t)(ch[0]*255.0f))<<24)|(((uint32_t)(ch[1]*255.0f))<<16)|(((uint32_t)(ch[2]*255.0f))<<8)|(((uint32_t)(ch[3]*255.0f)));
  sseVeci xch = ((BUN_SSE_M128i)BUN_SSE_SHUFFLEHI_EPI16(sseVeci(sseVec(ch)*sseVec(255.0f, 65280.0f, 255.0f, 65280.0f)), 0xB1));
  xch &= sseVeci(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  sseVeci xh = (BUN_SSE_M128i)BUN_SSE_SHUFFLE_EPI32((BUN_SSE_M128i)xch, 0x1B); // assign the values of xl to xr, but reversed, so we have (d,c,b,a)
  xch |= xh; // OR xl and xr so we get (d|a,c|b,b|c,a|d) in xl
  xh = BUN_SSE_CAST_PS_SI128(BUN_SSE_MOVEHL_PS(BUN_SSE_CAST_SI128_PS(xh), BUN_SSE_CAST_SI128_PS(xch))); // Move upper 2 ints to bottom 2 ints in xr so xr = (d,c,d|a,c|b)
  return (uint32_t)BUN_SSE_SI128_SI32(xch | xh);
}

static void inttoflt(uint32_t from, float(&ch)[4])
{
  sseVec c(BUN_SSE_SHUFFLEHI_EPI16(sseVeci(from)&sseVeci(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000), 0xB1));
  (c / sseVec(255.0f, 65280.0f, 255.0f, 65280.0f)).Set(ch);
}

TESTDEF::RETPAIR test_SSE()
{
  BEGINTEST;

  {
    CPU_Barrier();
    uint32_t r = Interpolate(0xFF00FFAA, 0x00FFAACC, 0.5f);
    TEST(r == 0x7F7FD4BB);
    sseVeci xr(r);
    sseVeci xz(r);
    xr += xz;
    sseVeci xa0 = xr;
    xr -= xz;
    sseVeci xa1 = xr;
    xr &= xz;
    sseVeci xa2 = xr;
    xr |= xz;
    sseVeci xa3 = xr;
    xr ^= xz;
    sseVeci xa4 = xr;
    xr >>= 5;
    sseVeci xa5 = xr;
    sseVeci y = xr >> 3;
    xr <<= 3;
    sseVeci xa6 = xr;
    xr <<= xz;
    sseVeci xa7 = xr;
    xr >>= xz;
    sseVeci xa8 = xr;
    sseVeci xa(r >> 3);
    sseVeci xw = ((xz + xa) | (xz&xa)) - (xa << 2) + ((xz << 4) ^ ((xz >> 5) >> 1));
    sseVeci c1(xw == r);
    sseVeci c2(xw != r);
    sseVeci c3(xw<r);
    sseVeci c4(xw>r);
    sseVeci c5(xw <= r);
    sseVeci c6(xw >= r);
    CPU_Barrier();
    TEST(r == 0x7F7FD4BB);
    BUN_ALIGN(16) int rv[4] = { -1, -1, -1, -1 };
    sseVeci::ZeroVector().Set(rv);
    TESTALLFOUR(rv, 0);
    xz.Set(rv);
    TESTALLFOUR(rv, 2139083963);
    xw.Set(rv);
    TESTALLFOUR(rv, 1169205849);
    c1.Set(rv);
    TESTALLFOUR(rv, 0);
    c2.Set(rv);
    TESTALLFOUR(rv, -1);
    c3.Set(rv);
    TESTALLFOUR(rv, -1);
    c4.Set(rv);
    TESTALLFOUR(rv, 0);
    c5.Set(rv);
    TESTALLFOUR(rv, -1);
    c6.Set(rv);
    TESTALLFOUR(rv, 0);
    c6.Set(rv);
    CPU_Barrier();

    xa0.Set(rv);
    TESTALLFOUR(rv, 0xFEFFA976);
    xa1.Set(rv);
    TESTALLFOUR(rv, 0x7F7FD4BB);
    xa2.Set(rv);
    TESTALLFOUR(rv, 0x7F7FD4BB);
    xa3.Set(rv);
    TESTALLFOUR(rv, 0x7F7FD4BB);
    xa4.Set(rv);
    TESTALLFOUR(rv, 0);
    xa5.Set(rv);
    TESTALLFOUR(rv, 0);
    xa6.Set(rv);
    TESTALLFOUR(rv, 0);
    xa7.Set(rv);
    TESTALLFOUR(rv, 0);
    xa8.Set(rv);
    TESTALLFOUR(rv, 0);

    sseVeci(1, 2, 3, 4).Shuffle<0, 1, 2, 3>().Set(rv);
    TESTFOUR(rv, 1, 2, 3, 4);
    sseVeci(1, 2, 3, 4).Shuffle<3, 2, 1, 0>().Set(rv);
    TESTFOUR(rv, 4, 3, 2, 1);
    TEST(sseVeci(1, 2, 3, 4).Sum() == 10);

    BUN_ALIGN(16) short rw[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    sseVeci16 w1(rw);
    sseVeci16 w2(rw);
    sseVeci16 w3(((w1 + w2)&(sseVeci16(4) | sseVeci16(2))) - w2);
    sseVeci16 w4(w3<w1);

    BUN_ALIGN(16) float ch[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
    uint32_t chr = flttoint(ch);
    TEST(chr == 0xFF7F7FFF);
    inttoflt(chr, ch);
    TESTRELFOUR(ch, 1.0f, 0.49803922f, 0.49803922f, 1.0f);
    CPU_Barrier();
  }

  {
    BUN_ALIGN(16) float arr[4] = { -1, -2, -3, -5 };
    float uarr[4] = { -1, -2, -3, -4 };
    sseVec u(1, 2, 3, 4);
    sseVec v(5);
    sseVec w(arr);
    w.Set(arr);
    TESTFOUR(arr, -1, -2, -3, -5);
    w = sseVec(BUN_UNALIGNED<const float>(uarr));
    w.Set(arr);
    TESTFOUR(arr, -1, -2, -3, -4);
    sseVec uw(u*w);
    uw.Set(arr);
    TESTFOUR(arr, -1, -4, -9, -16);
    sseVec uv(u*v);
    uv.Set(arr);
    TESTFOUR(arr, 5, 10, 15, 20);
    sseVec u_w(u / v);
    u_w.Set(arr);
    TESTFOUR(arr, 0.2f, 0.4f, 0.6f, 0.8f);
    sseVec u_v(u / w);
    u_v.Set(arr);
    TESTFOUR(arr, -1, -1, -1, -1);
    u_v = uw*w / v + u*v - v / w;
    u_v.Set(arr);
    TESTRELFOUR(arr, 10.2f, 14.1f, 22.0666666f, 34.05f);
    (u / w + v - u).Set(arr);
    TESTFOUR(arr, 3, 2, 1, 0);
    (u / w + v - u).Set(BUN_UNALIGNED<float>(uarr));
    TESTFOUR(uarr, 3, 2, 1, 0);
    sseVec m3(1, 3, -1, -2);
    sseVec m4(0, 4, -2, -1);
    sseVec ab = m3.max(m4);
    ab.Set(arr);
    TESTFOUR(arr, 1, 4, -1, -1);
    ab = m3.min(m4);
    ab.Set(arr);
    TESTFOUR(arr, 0, 3, -2, -2);
    sseVec(1, 2, 3, 4).Shuffle<0, 1, 2, 3>().Set(arr);
    TESTFOUR(arr, 1, 2, 3, 4);
    sseVec(1, 2, 3, 4).Shuffle<3, 2, 1, 0>().Set(arr);
    TESTFOUR(arr, 4, 3, 2, 1);
    TEST(sseVec(1, 2, 3, 4).Sum() == 10);
  }

  {
    BUN_ALIGN(16) double arr[2] = { -1, -2 };
    double uarr[2] = { -1, -2 };
    sseVecd u(1, 2);
    sseVecd v(5);
    sseVecd w(arr);
    w.Set(arr);
    TESTTWO(arr, -1, -2);
    w = sseVecd(BUN_UNALIGNED<const double>(uarr));
    w.Set(arr);
    TESTTWO(arr, -1, -2);
    sseVecd uw(u*w);
    uw.Set(arr);
    TESTTWO(arr, -1, -4);
    sseVecd uv(u*v);
    uv.Set(arr);
    TESTTWO(arr, 5, 10);
    sseVecd u_w(u / v);
    u_w.Set(arr);
    TESTTWO(arr, 0.2, 0.4);
    sseVecd u_v(u / w);
    u_v.Set(arr);
    TESTTWO(arr, -1, -1);
    u_v = uw*w / v + u*v - v / w;
    u_v.Set(arr);
    TEST(fCompare((arr)[0], (10.2)) && fCompare((arr)[1], (14.1)));
    (u / w + v - u).Set(arr);
    TESTTWO(arr, 3, 2);
    (u / w + v - u).Set(BUN_UNALIGNED<double>(uarr));
    TESTTWO(uarr, 3, 2);
    sseVecd m3(1, 3);
    sseVecd m4(0, 4);
    sseVecd ab = m3.max(m4);
    ab.Set(arr);
    TESTTWO(arr, 1, 4);
    ab = m3.min(m4);
    ab.Set(arr);
    TESTTWO(arr, 0, 3);
    sseVecd(1, 2).Swap().Set(arr);
    TESTTWO(arr, 2, 1);
    TEST(sseVecd(1, 2).Sum() == 3)
  }

  {
    BUN_ALIGN(16) int64_t arr[2] = { -1, -2 };
    int64_t uarr[2] = { -1, 2 };
    sseVeci64 u(1, 2);
    sseVeci64 v(5);
    sseVeci64 w(arr);
    w.Set(arr);
    TESTTWO(arr, -1, -2);
    w = sseVeci64(BUN_UNALIGNED<const int64_t>(uarr));
    w.Set(arr);
    TESTTWO(arr, -1, 2);
    sseVeci64 uw(u + w);
    uw.Set(arr);
    TESTTWO(arr, 0, 4);
    sseVeci64 uv(u - v);
    uv.Set(arr);
    TESTTWO(arr, -4, -3);
    sseVeci64 u_v(u + w);
    u_v.Set(arr);
    TESTTWO(arr, 0, 4);
    sseVeci64(1, 2).Swap().Set(arr);
    TESTTWO(arr, 2, 1);
    TEST(sseVeci64(1, 2).Sum() == 3)
  }

  //int megatest[TESTNUM*10];
  //for(size_t i = 0; i<TESTNUM*10; ++i)
  //  megatest[i]=log2(i);

  //for(int k=0; k < 30; ++k)
  //{
  //char prof;

  //Shuffle(megatest);
  //int l=0;
  //prof=HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //int v;
  //for(size_t i = 0; i < 1000000; i+=8) {
  //  v=PriComp(megatest[i+0],megatest[i+1],megatest[i+2],megatest[i+3],megatest[i+4],megatest[i+5],megatest[i+6],megatest[i+7]);
  //  l+=SGNCOMPARE(v,0);
  //}
  //CPU_Barrier();
  //std::cout << "SSE:" << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  //Shuffle(megatest);
  //int l2=0;
  //prof=HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(size_t i = 0; i < 1000000; i+=8)
  //{
  //  if(megatest[i]!=megatest[i+1]) {
  //    l2+=SGNCOMPARE(megatest[i],megatest[i+1]);
  //    continue;
  //  }
  //  if(megatest[i+2]!=megatest[i+3]) {
  //    l2+=SGNCOMPARE(megatest[i+2],megatest[i+3]);
  //    continue;
  //  }
  //  if(megatest[i+4]!=megatest[i+5]) {
  //    l2+=SGNCOMPARE(megatest[i+4],megatest[i+5]);
  //    continue;
  //  }
  //  if(megatest[i+6]!=megatest[i+7]) {
  //    l2+=SGNCOMPARE(megatest[i+6],megatest[i+7]);
  //    continue;
  //  }
  //}
  //CPU_Barrier();
  //std::cout << "NORMAL:" << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  //TEST(l==l2);
  //}

  //float rotation,left,right,top,bottom,x,y;

  //float testfloats[7*10000];
  //for(size_t i = 0; i < 7*10000; ++i)
  //  testfloats[i]=0.1*i;

  //float res[4];
  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(int cur=0; cur<70000; cur+=7)
  //{
  //  rotation=testfloats[cur];
  //  left=testfloats[cur+1];
  //  right=testfloats[cur+2];
  //  top=testfloats[cur+3];
  //  bottom=testfloats[cur+4];
  //  x=testfloats[cur+5];
  //  y=testfloats[cur+6];
  //}
  //CPU_Barrier();
  //std::cout << "NORMAL:" << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  //std::cout << left << right << top << bottom << std::endl;
  ENDTEST;
}