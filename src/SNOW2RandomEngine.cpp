/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "SNOW2RandomEngine.h"

#include "RandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#define byte(n,w)   (((w)>>(n*8)) & 0xff)
#define ainv_mul(w) (((w)>>8)^(snow_alphainv_mul[w&0xff]))
#define a_mul(w)    (((w)<<8)^(snow_alpha_mul[w>>24]))

using namespace sharemind;

namespace /* anonymous */ {

// Intentionally included inside anonymous namespace!
#include "snow2tab.h"

extern "C" {
void SNOW2RandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size);
void SNOW2RandomEngine_free(SharemindRandomEngine* rng_);
}

class SNOW2RandomEngine: public SharemindRandomEngine {
public: /* Methods: */

    inline explicit SNOW2RandomEngine(const void * const memptr) noexcept
        : SharemindRandomEngine {
              SNOW2RandomEngine_fill_bytes,
              SNOW2RandomEngine_free
          }
        , keystream_ready(sizeof(keystream))
    {
        #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
        VALGRIND_MAKE_MEM_DEFINED(this, sizeof(SNOW2RandomEngine));
        #endif

        uint8_t snowkey[32];
        uint32_t iv [4];

        memcpy(snowkey, memptr, sizeof (snowkey));
        memcpy(iv, ptrAdd(memptr, sizeof(snowkey)), sizeof(iv));
        snow_loadkey_fast_p(snowkey, 256, iv[0], iv[1], iv[2], iv[3]);
    }

    inline static SNOW2RandomEngine& fromWrapper(SharemindRandomEngine& base) noexcept {
        return static_cast<SNOW2RandomEngine&>(base);
    }

    inline void snow_loadkey_fast_p(uint8_t * key, uint32_t keysize, uint32_t IV3, uint32_t IV2, uint32_t IV1, uint32_t IV0) noexcept;

    inline void snow_keystream_fast_p() noexcept;

public: /* Fields: */

    uint32_t s15, s14, s13, s12, s11, s10, s9, s8, s7, s6, s5, s4, s3, s2, s1, s0;
    uint32_t r1, r2;
    uint8_t keystream_ready;
    union {
        uint32_t keystream[16];
        uint8_t un_byte_keystream[sizeof(uint32_t) * 16];
    };
};

/*
 * Function:  snow_loadkey_fast
 *
 * Synopsis:
 *   Loads the key material and performs the initial mixing.
 *
 * Returns: void
 *
 * Assumptions:
 *   keysize is either 128 or 256.
 *   key is of proper length, for keysize=128, key is of lenght 16 bytes
 *      and for keysize=256, key is of length 32 bytes.
 *   key is given in big endian format,
 *   For 128 bit key:
 *        key[0]-> msb of k_3
 *         ...
 *        key[3]-> lsb of k_3
 *         ...
 *        key[12]-> msb of k_0
 *         ...
 *        key[15]-> lsb of k_0
 *
 *   For 256 bit key:
 *        key[0]-> msb of k_7
 *          ...
 *        key[3]-> lsb of k_7
 *          ...
 *        key[28]-> msb of k_0
 *          ...
 *        key[31]-> lsb of k_0
 *
 * Authors:
 * Patrik Ekdahl & Thomas Johansson
 * Dept. of Information Technology
 * P.O. Box 118
 * SE-221 00 Lund, Sweden,
 * email: {patrik,thomas}@it.lth.se
 */

inline void SNOW2RandomEngine::snow_loadkey_fast_p (uint8_t *key, uint32_t keysize,uint32_t IV3, uint32_t IV2, uint32_t IV1, uint32_t IV0) noexcept
{
    if (keysize==128) {
        s15=(((uint32_t)*(key+0))<<24) | (((uint32_t)*(key+1))<<16) |
                (((uint32_t)*(key+2))<<8) | (((uint32_t)*(key+3)));
        s14=(((uint32_t)*(key+4))<<24) | (((uint32_t)*(key+5))<<16) |
                (((uint32_t)*(key+6))<<8) | (((uint32_t)*(key+7)));
        s13=(((uint32_t)*(key+8))<<24) | (((uint32_t)*(key+9))<<16) |
                (((uint32_t)*(key+10))<<8) | (((uint32_t)*(key+11)));
        s12=(((uint32_t)*(key+12))<<24) | (((uint32_t)*(key+13))<<16) |
                (((uint32_t)*(key+14))<<8) | (((uint32_t)*(key+15)));
        s11 =~s15; /* bitwise inverse */
        s10 =~s14;
        s9  =~s13;
        s8  =~s12;
        s7  = s15; /* just copy */
        s6  = s14;
        s5  = s13;
        s4  = s12;
        s3  =~s15; /* bitwise inverse */
        s2  =~s14;
        s1  =~s13;
        s0  =~s12;
    }
    else {  /* assume keysize=256 */
        assert(keysize == 256u);
        s15=(((uint32_t)*(key+0))<<24) | (((uint32_t)*(key+1))<<16) |
                (((uint32_t)*(key+2))<<8) | (((uint32_t)*(key+3)));
        s14=(((uint32_t)*(key+4))<<24) | (((uint32_t)*(key+5))<<16) |
                (((uint32_t)*(key+6))<<8) | (((uint32_t)*(key+7)));
        s13=(((uint32_t)*(key+8))<<24) | (((uint32_t)*(key+9))<<16) |
                (((uint32_t)*(key+10))<<8) | (((uint32_t)*(key+11)));
        s12=(((uint32_t)*(key+12))<<24) | (((uint32_t)*(key+13))<<16) |
                (((uint32_t)*(key+14))<<8) | (((uint32_t)*(key+15)));
        s11=(((uint32_t)*(key+16))<<24) | (((uint32_t)*(key+17))<<16) |
                (((uint32_t)*(key+18))<<8) | (((uint32_t)*(key+19)));
        s10=(((uint32_t)*(key+20))<<24) | (((uint32_t)*(key+21))<<16) |
                (((uint32_t)*(key+22))<<8) | (((uint32_t)*(key+23)));
        s9=(((uint32_t)*(key+24))<<24) | (((uint32_t)*(key+25))<<16) |
                (((uint32_t)*(key+26))<<8) | (((uint32_t)*(key+27)));
        s8=(((uint32_t)*(key+28))<<24) | (((uint32_t)*(key+29))<<16) |
                (((uint32_t)*(key+30))<<8) | (((uint32_t)*(key+31)));
        s7 =~s15; /* bitwise inverse */
        s6 =~s14;
        s5 =~s13;
        s4 =~s12;
        s3 =~s11;
        s2 =~s10;
        s1 =~s9;
        s0 =~s8;
    }

    /* XOR IV values */
    s15^=IV0;
    s12^=IV1;
    s10^=IV2;
    s9^=IV3;


    r1=0;
    r2=0;

    /* Do 32 initial clockings */
    for(int i=0;i<2;i++)
    {

        uint32_t outfrom_fsm=(r1+ s15 )^r2;
        s0 =a_mul(s0 )^ s2 ^ainv_mul(s11 )^outfrom_fsm;
        uint32_t fsmtmp=r2+ s5 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s0 )^r2;
        s1 =a_mul(s1 )^ s3 ^ainv_mul(s12 )^outfrom_fsm;
        fsmtmp=r2+ s6 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s1 )^r2;
        s2 =a_mul(s2 )^ s4 ^ainv_mul(s13 )^outfrom_fsm;
        fsmtmp=r2+ s7 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s2 )^r2;
        s3 =a_mul(s3 )^ s5 ^ainv_mul(s14 )^outfrom_fsm;
        fsmtmp=r2+ s8 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s3 )^r2;
        s4 =a_mul(s4 )^ s6 ^ainv_mul(s15 )^outfrom_fsm;
        fsmtmp=r2+ s9 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s4 )^r2;
        s5 =a_mul(s5 )^ s7 ^ainv_mul(s0 )^outfrom_fsm;
        fsmtmp=r2+ s10 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s5 )^r2;
        s6 =a_mul(s6 )^ s8 ^ainv_mul(s1 )^outfrom_fsm;
        fsmtmp=r2+ s11 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s6 )^r2;
        s7 =a_mul(s7 )^ s9 ^ainv_mul(s2 )^outfrom_fsm;
        fsmtmp=r2+ s12 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s7 )^r2;
        s8 =a_mul(s8 )^ s10 ^ainv_mul(s3 )^outfrom_fsm;
        fsmtmp=r2+ s13 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s8 )^r2;
        s9 =a_mul(s9 )^ s11 ^ainv_mul(s4 )^outfrom_fsm;
        fsmtmp=r2+ s14 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s9 )^r2;
        s10 =a_mul(s10 )^ s12 ^ainv_mul(s5 )^outfrom_fsm;
        fsmtmp=r2+ s15 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s10 )^r2;
        s11 =a_mul(s11 )^ s13 ^ainv_mul(s6 )^outfrom_fsm;
        fsmtmp=r2+ s0 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s11 )^r2;
        s12 =a_mul(s12 )^ s14 ^ainv_mul(s7 )^outfrom_fsm;
        fsmtmp=r2+ s1 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s12 )^r2;
        s13 =a_mul(s13 )^ s15 ^ainv_mul(s8 )^outfrom_fsm;
        fsmtmp=r2+ s2 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s13 )^r2;
        s14 =a_mul(s14 )^ s0 ^ainv_mul(s9 )^outfrom_fsm;
        fsmtmp=r2+ s3 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;

        outfrom_fsm=(r1+ s14 )^r2;
        s15 =a_mul(s15 )^ s1 ^ainv_mul(s10 )^outfrom_fsm;
        fsmtmp=r2+ s4 ;
        r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
        r1=fsmtmp;
    }
}

/*
 * Function: snow_keystream_fast
 *
 * Synopsis:
 *   Clocks the cipher 16 times and returns 16 words of keystream symbols
 *   in keystream.
 *
 * Returns: void
 *
 * Authors:
 * Patrik Ekdahl & Thomas Johansson
 * Dept. of Information Technology
 * P.O. Box 118
 * SE-221 00 Lund, Sweden,
 * email: {patrik,thomas}@it.lth.se
 *
 */
inline void SNOW2RandomEngine::snow_keystream_fast_p() noexcept
{

    s0 =a_mul(s0 )^ s2 ^ainv_mul(s11 );
    uint32_t fsmtmp=r2+ s5 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[0]=(r1+ s0 )^r2^ s1 ;

    s1 =a_mul(s1 )^ s3 ^ainv_mul(s12 );
    fsmtmp=r2+ s6 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[1]=(r1+ s1 )^r2^ s2 ;

    s2 =a_mul(s2 )^ s4 ^ainv_mul(s13 );
    fsmtmp=r2+ s7 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[2]=(r1+ s2 )^r2^ s3 ;

    s3 =a_mul(s3 )^ s5 ^ainv_mul(s14 );
    fsmtmp=r2+ s8 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[3]=(r1+ s3 )^r2^ s4 ;

    s4 =a_mul(s4 )^ s6 ^ainv_mul(s15 );
    fsmtmp=r2+ s9 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[4]=(r1+ s4 )^r2^ s5 ;

    s5 =a_mul(s5 )^ s7 ^ainv_mul(s0 );
    fsmtmp=r2+ s10 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[5]=(r1+ s5 )^r2^ s6 ;

    s6 =a_mul(s6 )^ s8 ^ainv_mul(s1 );
    fsmtmp=r2+ s11 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[6]=(r1+ s6 )^r2^ s7 ;

    s7 =a_mul(s7 )^ s9 ^ainv_mul(s2 );
    fsmtmp=r2+ s12 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[7]=(r1+ s7 )^r2^ s8 ;

    s8 =a_mul(s8 )^ s10 ^ainv_mul(s3 );
    fsmtmp=r2+ s13 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[8]=(r1+ s8 )^r2^ s9 ;

    s9 =a_mul(s9 )^ s11 ^ainv_mul(s4 );
    fsmtmp=r2+ s14 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[9]=(r1+ s9 )^r2^ s10 ;

    s10 =a_mul(s10 )^ s12 ^ainv_mul(s5 );
    fsmtmp=r2+ s15 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[10]=(r1+ s10 )^r2^ s11 ;

    s11 =a_mul(s11 )^ s13 ^ainv_mul(s6 );
    fsmtmp=r2+ s0 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[11]=(r1+ s11 )^r2^ s12 ;

    s12 =a_mul(s12 )^ s14 ^ainv_mul(s7 );
    fsmtmp=r2+ s1 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[12]=(r1+ s12 )^r2^ s13 ;

    s13 =a_mul(s13 )^ s15 ^ainv_mul(s8 );
    fsmtmp=r2+ s2 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[13]=(r1+ s13 )^r2^ s14 ;

    s14 =a_mul(s14 )^ s0 ^ainv_mul(s9 );
    fsmtmp=r2+ s3 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[14]=(r1+ s14 )^r2^ s15 ;

    s15 =a_mul(s15 )^ s1 ^ainv_mul(s10 );
    fsmtmp=r2+ s4 ;
    r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
    r1=fsmtmp;
    keystream[15]=(r1+ s15 )^r2^ s0 ;
}

extern "C"
void SNOW2RandomEngine_fill_bytes(SharemindRandomEngine * rng_,
                                  void * memptr,
                                  size_t size)
{
    assert(rng_);
    assert(memptr);

    auto& rng = SNOW2RandomEngine::fromWrapper(*rng_);

    size_t currentKeystreamSize = sizeof (rng.keystream) - rng.keystream_ready;
    size_t offsetStart = 0;
    size_t offsetEnd = currentKeystreamSize;

    // Fill big chunks
    while (offsetEnd <= size) {
        memcpy(ptrAdd(memptr, offsetStart), &rng.un_byte_keystream[rng.keystream_ready], currentKeystreamSize);
        rng.snow_keystream_fast_p();
        rng.keystream_ready = 0;
        currentKeystreamSize = sizeof (rng.keystream);
        offsetStart = offsetEnd;
        offsetEnd += sizeof (rng.keystream);
    }

    const size_t remainingSize = size - offsetStart;
    memcpy(ptrAdd(memptr, offsetStart), &rng.un_byte_keystream[rng.keystream_ready], remainingSize);
    rng.keystream_ready += remainingSize;
    assert(rng.keystream_ready <= sizeof(rng.keystream)); // the supply may deplete
}

extern "C"
void SNOW2RandomEngine_free(SharemindRandomEngine* rng_) {
    assert(rng_);
    delete &SNOW2RandomEngine::fromWrapper(*rng_);
}

} // namespace anonymous

namespace sharemind {


size_t SNOW2_random_engine_seed_size () noexcept {
    return sizeof(uint8_t)*32 + sizeof(uint32_t)*4;
}

SharemindRandomEngine* make_SNOW2_random_engine(const void* memptr_) {
    return new SNOW2RandomEngine {memptr_};
}

}
