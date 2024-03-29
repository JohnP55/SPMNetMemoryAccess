// https://github.com/already5chosen/strtod/blob/master/n2_small_strtod.c

#include <stdlib.h>
// #include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifdef __NIOS2
#include "system.h"
#endif

#ifndef ALT_CPU_HARDWARE_MULX_PRESENT
// on systems others than nios2 assume presence of mulx or similar instruction
#define ALT_CPU_HARDWARE_MULX_PRESENT 1
#endif

static const char *skipWhiteSpaces(const char *str)
{ // skip leading white spaces
  while (*str && *str <= ' ') ++str;
  return str;
}

//
// map ASCII characters 'A'-'Z' to 'a' to 'a'-'z'
// map ASCII characters 'a'-'z' to themselves
// all other values are guaranteed to not be mapped intto 'a'-'z' range
static int ascii_tolower(int c)
{
  return c | 0x20;
}

// return bit pattern of IEEE binary64
static uint64_t to_double(uint64_t mant, int exp)
{
  int EXP_BIAS = 1023;
  exp += EXP_BIAS;
  if (exp >= 2047)
    return (uint64_t)2047 << 52; // Inf

  if (exp < 1) {
    if (exp < -52)
      return 0;

    // subnormal
    do {
      // shift mantisa to the right while folding together LS bits
      mant = (mant >> 1) | (mant & 1);
      ++exp;
    } while (exp <= 0);
    exp = 0;
  }

  const uint64_t MANT_MASK = (uint64_t)-1 >> 1;  // 63 LS bits
  const uint32_t REM_MSB   = 1u << 10;
  const uint32_t REM_MASK  = REM_MSB*2-1;        // 11 LS bits

  unsigned rem = mant & REM_MASK;
  uint64_t ret = (mant & MANT_MASK) >> 11; // remove MS bit and shift mantisa to lS bits
  rem |= (ret & 1); // tie breaks to nearest even
  ret |= ((uint64_t)exp << 52);
  ret += (rem > REM_MSB);
  return ret;
}

// on input *pw2 > 0
// return # of leading zeros
static int normailize96(uint32_t* pw2, uint32_t* pw1, uint32_t* pw0)
{
  uint32_t w0 = *pw0;
  uint32_t w1 = *pw1;
  uint32_t w2 = *pw2;
#if 1
  const uint32_t MSB = (uint32_t)1 << 31;
  int lz = 0;
  while (w2 < MSB) {
    w2 += w2 + (w1>>31);
    w1 += w1 + (w0>>31);
    w0 += w0;
    ++lz;
  }
#else
  int lz = __builtin_clz(w2);
  if (lz) {
    w2 = (w2 << lz) | (w1 >> (32-lz));
    w1 = (w1 << lz) | (w0 >> (32-lz));
    w0 = (w0 << lz);
  }
#endif
  *pw2 = w2;
  *pw1 = w1;
  *pw0 = w0;
  return lz;
}

double
__attribute__ ((cold))
strtod(const char* str, char** endptr)
{
  const uint8_t* p = (const uint8_t*)skipWhiteSpaces(str);

  const uint8_t* endptrval = 0;
  uint32_t rdVal0=0, rdVal1=0;
  ptrdiff_t rdExp = 0, exp;
  const uint32_t maxVal = ((UINT64_MAX-9)/10)>>32;
  uint32_t mant0, mant1;
  int lsbits = 0;
  unsigned sticky;
  int neg, nege;
  enum { PARSE_INT = 0, PARSE_FRACT = 1, PARSE_EXP };
  for (int parseState = PARSE_INT;;parseState = PARSE_EXP) {
    unsigned signC = p[0];
    nege = (signC=='-');
    int isPlus =  (signC=='+');
    p += nege | isPlus;

    unsigned c;
    for (;;) {
      for (;;) {
        c = *p++;
        unsigned dig = c - '0';
        if (dig > 9)
          break; // non-digit
        endptrval = p;
        rdExp -= parseState;
        if (rdVal1 < maxVal) {
          #if ALT_CPU_HARDWARE_MULX_PRESENT
          uint64_t x10 = (((uint64_t)rdVal1 << 32)|rdVal0)*10 + dig;
          rdVal0 = (uint32_t)x10;
          rdVal1 = (uint32_t)(x10>>32);
          #else
          uint32_t x10h = (rdVal0>>4)*10;
          rdVal0 = rdVal0*10 + dig;
          dig = (x10h >> 28)+((x10h << 4) > rdVal0);
          rdVal1 = rdVal1*10 + dig;
          #endif
        } else {
          lsbits |= dig;
          ++rdExp;
        }
      }

      // non-digit
      if (parseState == PARSE_INT) {
        parseState = PARSE_FRACT;
        if (c == '.') {
          // decimal point
          if (endptrval != 0) // there were digits before decimal point
            endptrval = p;
          continue;
        }
      }
      break;
    }

    if (parseState != PARSE_EXP)
    {
      // end of mantissa
      neg     = nege;
      exp     = rdExp;
      sticky  = lsbits;
      mant0   = rdVal0;
      mant1   = rdVal1;
      rdVal0  = rdVal1 = 0;

      if (endptrval==0) { // conversion fail
        if (endptr)
          *endptr = (char*)str;
        return 0;
      }
      // conversion succeed
      if (ascii_tolower(c) == 'e')
        continue; // possibly, exponent present
      // exponent absent
    }
    break;
  }

  if (endptr)
    *endptr = (char*)endptrval;

  uint64_t uret = (uint64_t)neg << 63; // set sign bit
  if ((mant0|mant1) != 0) {
    const int MAX_EXP_MAGNITUDE = 345;

    unsigned sexp = nege;
    unsigned mexp = MAX_EXP_MAGNITUDE;
    if (rdVal1 == 0) {
      if (sexp)
        exp = -exp;
      int64_t exp64 = exp;
      exp64 += rdVal0;
      if (exp64 < 0) {
        exp64 = -exp64;
        sexp = !sexp;
      }
      if ((uint64_t)exp64 < mexp)
        mexp = (uint32_t)exp64;
    }

    uint32_t w2 = mant1;
    uint32_t w1 = mant0;
    uint32_t w0 = 0;
    int bine = 63;
    if (w2 == 0) {
      w2   = w1;
      w1   = 0;
      bine = 63 - 32;
    }
    for (;;) {
      // normalize mantissa
      bine -= normailize96(&w2, &w1, &w0);

      if (mexp == 0)
        break;

      // scale w2:w1:w0 * 2**bine by 10**exp
      typedef struct {
        uint32_t MULx_H;
        uint32_t MULx_M;
        uint8_t  MULx_L;
        uint8_t  decExp;
        int16_t  binExp;
      } dec_scale_tab_entry_t;
      static const dec_scale_tab_entry_t DecScaleTab[2][3] = {
        { // scale up
          { 0xFC6F7C40, 0x45812296, 0x4D, 31, 103,}, // (10**31/2**103)*2**72
          { 0xFA000000, 0x00000000, 0x00,  3,  10,}, // (10**3 /2**10 )*2**72
          { 0xA0000000, 0x00000000, 0x00,  1,   4,}, // (10**0 /2**4  )*2**72
        },
        { // scale down
          { 0xE45C10C4, 0x2A2B3B05, 0x8D, 44,-146,}, // (2**146/10**44)*2**72
          { 0xD6BF94D5, 0xE57A42BC, 0x3D,  7, -23,}, // (2**23 /10**7 )*2**72
          { 0xCCCCCCCC, 0xCCCCCCCC, 0xCD,  1,  -3,}, // (2**3  /10**1 )*2**72
        },
      };
      const dec_scale_tab_entry_t* pTab = &DecScaleTab[sexp][0];
      unsigned decExp;
      while ((decExp=pTab->decExp) > mexp)
        ++pTab;

      const uint32_t MULx_L = pTab->MULx_L;
      const uint32_t MULx_M = pTab->MULx_M;
      const uint32_t MULx_H = pTab->MULx_H;
      const int      binExp = pTab->binExp;
      do {
        // multiply by 10**N, where N= (-44, -7, -1, 1, 3, 31)
        // w2*mh
        //    w2*mm
        //       w2*ml
        //    w1*mh
        //       w1*mm
        //       w0*mh
        uint64_t w2w1 =
              ( (uint64_t)w2 * MULx_H)
            + (((uint64_t)w2 * MULx_M) >> 32)
            + (((uint64_t)w1 * MULx_H) >> 32);
        w0 =  (w0 >> 18) * (MULx_H >> 17)
            +((w1        *  MULx_H) >> 3)
            +((w2        *  MULx_M) >> 3)
            + (w1 >> 18) * (MULx_M >> 17)
            + (w2 >> 11) *  MULx_L;
        w2w1 += (w0 >> 29);
        w0  = w0 << 3;
        w1  = (uint32_t)w2w1;
        w2  = (uint32_t)(w2w1 >> 32);
        bine += binExp;
        mexp -= decExp;
      } while (mexp >= decExp);
    }
    // printf("%08x:%08x:%08x\n", w2, w1, w0);
    w1 |= (((w0>>22) | sticky) != 0); // approximately 9-10 MS bits of w0 are good. The rest is garbage
    uret |= to_double(((uint64_t)w2 << 32) | w1, bine);
  }

  double dret;
  memcpy(&dret, &uret, sizeof(dret));

  return dret;
}