
#ifndef __DDB_HASH_H__
#define __DDB_HASH_H__



#ifdef LGPL_IS_OK

/* Paul Hsieh's SuperFastHash from http://www.azillionmonkeys.com/qed/hash.html
 * This is LGPL'd and compiled into this program. Some may find that unacceptable
 * In future releases, this will be off by default
 * */
#include <stdlib.h>
#include <stdint.h> /* Replace with <stdint.h> if appropriate */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
               +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

static uint32_t SuperFastHash (const char * data, int len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
    case 3: hash += get16bits (data);
        hash ^= hash << 16;
        hash ^= data[sizeof (uint16_t)] << 18;
        hash += hash >> 11;
        break;
    case 2: hash += get16bits (data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: hash += *data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

#else /* NOT LGPL_IS_OK */


/*-
 * This notice applies to the code below here
 * Public Domain 2014-2019 MongoDB, Inc.
 * Public Domain 2008-2014 WiredTiger, Inc.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Copyright (c) 2011 Google, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * CityHash, by Geoff Pike and Jyrki Alakuijala
 *
 * This file provides CityHash64() and related functions.
 *
 * It's probably possible to create even faster hash functions by
 * writing a program that systematically explores some of the space of
 * possible hash functions, by using SIMD instructions, or by
 * compromising on hash quality.
 */



typedef struct _uint128 uint128;
struct _uint128 {
    uint64_t first;
    uint64_t second;
};

#define	Uint128Low64(x) 	(x).first
#define	Uint128High64(x)	(x).second

#ifdef WORDS_BIGENDIAN
#ifdef _MSC_VER

#include <stdlib.h>
#define	bswap_32(x) _byteswap_ulong(x)
#define	bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define	bswap_32(x) OSSwapInt32(x)
#define	bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define	bswap_32(x) BSWAP_32(x)
#define	bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define	bswap_32(x) bswap32(x)
#define	bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define	bswap_32(x) swap32(x)
#define	bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define	bswap_32(x) bswap32(x)
#define	bswap_64(x) bswap64(x)
#endif


#endif

#define	uint32_in_expected_order(x) (bswap_32(x))
#define	uint64_in_expected_order(x) (bswap_64(x))
#else /* Not WORDS_BIGENDIAN */
#ifdef _MSC_VER
#include <stdlib.h>
#define	bswap_32(x) _byteswap_ulong(x)
#define	bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)
// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define	bswap_32(x) OSSwapInt32(x)
#define	bswap_64(x) OSSwapInt64(x)

#else
#include <endian.h>
#define	bswap_32(x) __bswap_32 (x)
#define	bswap_64(x) __bswap_64 (x)
#endif

#define	uint32_in_expected_order(x) (x)
#define	uint64_in_expected_order(x) (x)
#endif /* WORDS_BIGENDIAN */


static inline void PERMUTE3(uint32_t * a, uint32_t * b, uint32_t * c){
    uint32_t t;
    t = *a;
    *a = *c;
    *c = *b;
    *b = t;
}
static uint64_t UNALIGNED_LOAD64(const char *p) {
    uint64_t result;
    memcpy(&result, p, sizeof(result));
    return (result);
}

static uint32_t UNALIGNED_LOAD32(const char *p) {
    uint32_t result;
    memcpy(&result, p, sizeof(result));
    return (result);
}

static uint64_t Fetch64(const char *p) {
    return uint64_in_expected_order(UNALIGNED_LOAD64(p));
}

static uint32_t Fetch32(const char *p) {
    return uint32_in_expected_order(UNALIGNED_LOAD32(p));
}

/* Some primes between 2^63 and 2^64 for various uses. */
static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
static const uint64_t k1 = 0xb492b66fbe98f273ULL;
static const uint64_t k2 = 0x9ae16a3b2f90404fULL;
static const uint64_t k3 = 0xc949d7c7509e6557ULL;

// Magic numbers for 32-bit hashing.  Copied from Murmur3.
static const uint32_t c1 = 0xcc9e2d51;
static const uint32_t c2 = 0x1b873593;


/*
 * Hash 128 input bits down to 64 bits of output.
 * This is intended to be a reasonably good hash function.
 */
static inline uint64_t Hash128to64(const uint128 x) {
    /* Murmur-inspired hashing. */
    const uint64_t kMul = 0x9ddfea08eb382d69ULL;
    uint64_t a, b;

    a = (Uint128Low64(x) ^ Uint128High64(x)) * kMul;
    a ^= (a >> 47);
    b = (Uint128High64(x) ^ a) * kMul;
    b ^= (b >> 47);
    b *= kMul;
    return (b);
}

/*
 * Bitwise right rotate.  Normally this will compile to a single
 * instruction, especially if the shift is a manifest constant.
 */
static uint64_t Rotate(uint64_t val, int shift) {
    /* Avoid shifting by 64: doing so yields an undefined result. */
    return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

/*
 * Equivalent to Rotate(), but requires the second arg to be non-zero.
 * On x86-64, and probably others, it's possible for this to compile
 * to a single instruction if both args are already in registers.
 */
static uint64_t RotateByAtLeast1(uint64_t val, int shift) {
    return (val >> shift) | (val << (64 - shift));
}

static uint64_t ShiftMix(uint64_t val) {
    return val ^ (val >> 47);
}

static uint64_t HashLen16(uint64_t u, uint64_t v) {
    uint128 result;

    result.first = u;
    result.second = v;
    return Hash128to64(result);
}

static uint64_t HashLen0to16(const char *s, size_t len) {
    uint64_t a64, b64;
    uint32_t y, z;
    uint8_t a8, b8, c8;
    if (len > 8) {
        a64 = Fetch64(s);
        b64 = Fetch64(s + len - 8);
        return HashLen16(
                a64, RotateByAtLeast1(b64 + len, (int)len)) ^ b64;
    }
    if (len >= 4) {
        a64 = Fetch32(s);
        return HashLen16(len + (a64 << 3), Fetch32(s + len - 4));
    }
    if (len > 0) {
        a8 = (uint8_t)s[0];
        b8 = (uint8_t)s[len >> 1];
        c8 = (uint8_t)s[len - 1];
        y = (uint32_t)(a8) + ((uint32_t)(b8) << 8);
        z = (uint32_t)len + ((uint32_t)(c8) << 2);
        return ShiftMix(y * k2 ^ z * k3) * k2;
    }
    return (k2);
}

/*
 * This probably works well for 16-byte strings as well, but it may be overkill
 * in that case.
 */
static uint64_t HashLen17to32(const char *s, size_t len) {
    uint64_t a = Fetch64(s) * k1;
    uint64_t b = Fetch64(s + 8);
    uint64_t c = Fetch64(s + len - 8) * k2;
    uint64_t d = Fetch64(s + len - 16) * k0;
    return HashLen16(Rotate(a - b, 43) + Rotate(c, 30) + d,
                     a + Rotate(b ^ k3, 20) + len - c);
}

/*
 * Return a 16-byte hash for 48 bytes.  Quick and dirty.
 * Callers do best to use "random-looking" values for a and b.
 * static pair<uint64, uint64> WeakHashLen32WithSeeds(
 */
static void WeakHashLen32WithSeeds6(uint64_t w, uint64_t x,
                                    uint64_t y, uint64_t z, uint64_t a, uint64_t b, uint128 *ret) {
    uint64_t c;

    a += w;
    b = Rotate(b + a + z, 21);
    c = a;
    a += x;
    a += y;
    b += Rotate(a, 44);

    ret->first = (uint64_t) (a + z);
    ret->second = (uint64_t) (b + c);
}

/*
 * Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
 * static pair<uint64, uint64> WeakHashLen32WithSeeds(
 */
static void WeakHashLen32WithSeeds(
        const char* s, uint64_t a, uint64_t b, uint128 *ret) {
    WeakHashLen32WithSeeds6(Fetch64(s),
                            Fetch64(s + 8),
                            Fetch64(s + 16),
                            Fetch64(s + 24),
                            a,
                            b,
                            ret);
}

/* Return an 8-byte hash for 33 to 64 bytes. */
static uint64_t HashLen33to64(const char *s, size_t len) {
    uint64_t a, b, c, r, vf, vs, wf, ws, z;
    z = Fetch64(s + 24);
    a = Fetch64(s) + (len + Fetch64(s + len - 16)) * k0;
    b = Rotate(a + z, 52);
    c = Rotate(a, 37);
    a += Fetch64(s + 8);
    c += Rotate(a, 7);
    a += Fetch64(s + 16);
    vf = a + z;
    vs = b + Rotate(a, 31) + c;
    a = Fetch64(s + 16) + Fetch64(s + len - 32);
    z = Fetch64(s + len - 8);
    b = Rotate(a + z, 52);
    c = Rotate(a, 37);
    a += Fetch64(s + len - 24);
    c += Rotate(a, 7);
    a += Fetch64(s + len - 16);
    wf = a + z;
    ws = b + Rotate(a, 31) + c;
    r = ShiftMix((vf + ws) * k2 + (wf + vs) * k0);
    return ShiftMix(r * k0 + vs) * k2;
}

static uint32_t Rotate32(uint32_t val, int shift) {
    // Avoid shifting by 32: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
}

static inline uint32_t Mur(uint32_t a, uint32_t h) {
    // Helper from Murmur3 for combining two 32-bit values.
    a *= c1;
    a = Rotate32(a, 17);
    a *= c2;
    h ^= a;
    h = Rotate32(h, 19);
    return h * 5 + 0xe6546b64;
}

static inline uint32_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}


static inline uint32_t Hash32Len0to4(const char *s, size_t len) {
    uint32_t b = 0;
    uint32_t c = 9;
    for (size_t i = 0; i < len; i++) {
        signed char v = s[i];
        b = b * c1 + v;
        c ^= b;
    }
    return fmix(Mur(b, Mur(len, c)));
}

static inline uint32_t Hash32Len13to24(const char *s, size_t len) {
    uint32_t a = Fetch32(s - 4 + (len >> 1));
    uint32_t b = Fetch32(s + 4);
    uint32_t c = Fetch32(s + len - 8);
    uint32_t d = Fetch32(s + (len >> 1));
    uint32_t e = Fetch32(s);
    uint32_t f = Fetch32(s + len - 4);
    uint32_t h = len;

    return fmix(Mur(f, Mur(e, Mur(d, Mur(c, Mur(b, Mur(a, h)))))));
}
static inline uint32_t Hash32Len5to12(const char *s, size_t len) {
    uint32_t a = len, b = len * 5, c = 9, d = b;
    a += Fetch32(s);
    b += Fetch32(s + len - 4);
    c += Fetch32(s + ((len >> 1) & 4));
    return fmix(Mur(c, Mur(b, Mur(a, d))));
}

static inline uint32_t CityHash32(const char *s, size_t len) {
    if (len <= 24) {
        return len <= 12 ?
               (len <= 4 ? Hash32Len0to4(s, len) : Hash32Len5to12(s, len)) :
               Hash32Len13to24(s, len);
    }

    // len > 24
    uint32_t h = len, g = c1 * len, f = g;
    uint32_t a0 = Rotate32(Fetch32(s + len - 4) * c1, 17) * c2;
    uint32_t a1 = Rotate32(Fetch32(s + len - 8) * c1, 17) * c2;
    uint32_t a2 = Rotate32(Fetch32(s + len - 16) * c1, 17) * c2;
    uint32_t a3 = Rotate32(Fetch32(s + len - 12) * c1, 17) * c2;
    uint32_t a4 = Rotate32(Fetch32(s + len - 20) * c1, 17) * c2;
    h ^= a0;
    h = Rotate32(h, 19);
    h = h * 5 + 0xe6546b64;
    h ^= a2;
    h = Rotate32(h, 19);
    h = h * 5 + 0xe6546b64;
    g ^= a1;
    g = Rotate32(g, 19);
    g = g * 5 + 0xe6546b64;
    g ^= a3;
    g = Rotate32(g, 19);
    g = g * 5 + 0xe6546b64;
    f += a4;
    f = Rotate32(f, 19);
    f = f * 5 + 0xe6546b64;
    size_t iters = (len - 1) / 20;
    do {
        uint32_t a0 = Rotate32(Fetch32(s) * c1, 17) * c2;
        uint32_t a1 = Fetch32(s + 4);
        uint32_t a2 = Rotate32(Fetch32(s + 8) * c1, 17) * c2;
        uint32_t a3 = Rotate32(Fetch32(s + 12) * c1, 17) * c2;
        uint32_t a4 = Fetch32(s + 16);
        h ^= a0;
        h = Rotate32(h, 18);
        h = h * 5 + 0xe6546b64;
        f += a1;
        f = Rotate32(f, 19);
        f = f * c1;
        g += a2;
        g = Rotate32(g, 18);
        g = g * 5 + 0xe6546b64;
        h ^= a3 + a1;
        h = Rotate32(h, 19);
        h = h * 5 + 0xe6546b64;
        g ^= a4;
        g = bswap_32(g) * 5;
        h += a4 * 5;
        h = bswap_32(h);
        f += a0;
        PERMUTE3(&f, &h, &g);
        s += 20;
    } while (--iters != 0);
    g = Rotate32(g, 11) * c1;
    g = Rotate32(g, 17) * c1;
    f = Rotate32(f, 11) * c1;
    f = Rotate32(f, 17) * c1;
    h = Rotate32(h + g, 19);
    h = h * 5 + 0xe6546b64;
    h = Rotate32(h, 17) * c1;
    h = Rotate32(h + f, 19);
    h = h * 5 + 0xe6546b64;
    h = Rotate32(h, 17) * c1;
    return h;
}

static inline uint64_t CityHash64(const char *s, size_t len) {
    uint64_t temp, x, y, z;
    uint128 v, w;

    if (len <= 32) {
        if (len <= 16) {
            return HashLen0to16(s, len);
        } else { /* NOLINT(readability-else-after-return) */
            return HashLen17to32(s, len);
        }
    } else if (len <= 64) {
        return HashLen33to64(s, len);
    }

    /*
     * For strings over 64 bytes we hash the end first, and then as we
     * loop we keep 56 bytes of state: v, w, x, y, and z.
     */
    x = Fetch64(s + len - 40);
    y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
    z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
    WeakHashLen32WithSeeds(s + len - 64, len, z, &v);
    WeakHashLen32WithSeeds(s + len - 32, y + k1, x, &w);
    x = x * k1 + Fetch64(s);

    /*
     * Use len to count multiples of 64, and operate on 64-byte chunks.
     */
    for (len = (len - 1) >> 6; len != 0; len--) {
        x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
        y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
        x ^= w.second;
        y += v.first + Fetch64(s + 40);
        z = Rotate(z + w.first, 33) * k1;
        WeakHashLen32WithSeeds(s, v.second * k1, x + w.first, &v);
        WeakHashLen32WithSeeds(
                s + 32, z + w.second, y + Fetch64(s + 16), &w);
        temp = z;
        z = x;
        x = temp;
        s += 64;
    }
    return HashLen16(HashLen16(v.first, w.first) + ShiftMix(y) * k1 + z,
                     HashLen16(v.second, w.second) + x);
}


static uint32_t SuperFastHash (const char * data, int len) {
    return (CityHash32((const void *) data, (size_t) len));
}
/* End of Segment
 * licensed  by MongoDB, Inc.
 *              WiredTiger, Inc.
 *              Google, Inc.
 */

#endif  /* LGPL_IS_OK */
#endif /* __DDB_HASH_H__ */
