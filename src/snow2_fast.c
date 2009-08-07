/*
 *  FAST IMPLEMENTATION OF STREAM CIPHER SNOW 2.0
 * 
 *  Program: snowblock2.c
 *
 * Synopsis:
 *      Hard coded implementation of SNOW 2.0 stream cipher.
 *      Each call to snow_keystream_block generates 16 words of
 *      keystream symbols.
 *        
 */ 
#include "snow2_fast.h"
#include "snow2tab.h"


#define byte(n,w)   (((w)>>(n*8)) & 0xff)
#define ainv_mul(w) (((w)>>8)^(snow_alphainv_mul[w&0xff]))
#define a_mul(w)    (((w)<<8)^(snow_alpha_mul[w>>24]))


/* By compiling with option -DUSEINLINE some routines can be inlined */
#ifdef USEINLINE
#define INLINE __inline__
#else
#define INLINE
#endif


#define u32 unsigned long
#define u8  unsigned char

u32 s15,s14,s13,s12,s11,s10,s9,s8,s7,s6,s5,s4,s3,s2,s1,s0;
u32 outfrom_fsm,fsmtmp,r1,r2;
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
 */
void snow_loadkey_fast(u8 *key,u32 keysize,u32 IV3,u32 IV2,u32 IV1,u32 IV0)
{
 int i;
 
  if (keysize==128) {
    s15=(((u32)*(key+0))<<24) | (((u32)*(key+1))<<16) |
      (((u32)*(key+2))<<8) | (((u32)*(key+3)));
    s14=(((u32)*(key+4))<<24) | (((u32)*(key+5))<<16) |
      (((u32)*(key+6))<<8) | (((u32)*(key+7)));
    s13=(((u32)*(key+8))<<24) | (((u32)*(key+9))<<16) |
      (((u32)*(key+10))<<8) | (((u32)*(key+11)));
    s12=(((u32)*(key+12))<<24) | (((u32)*(key+13))<<16) |
      (((u32)*(key+14))<<8) | (((u32)*(key+15)));
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
     s15=(((u32)*(key+0))<<24) | (((u32)*(key+1))<<16) |
      (((u32)*(key+2))<<8) | (((u32)*(key+3)));
    s14=(((u32)*(key+4))<<24) | (((u32)*(key+5))<<16) |
      (((u32)*(key+6))<<8) | (((u32)*(key+7)));
    s13=(((u32)*(key+8))<<24) | (((u32)*(key+9))<<16) |
      (((u32)*(key+10))<<8) | (((u32)*(key+11)));
    s12=(((u32)*(key+12))<<24) | (((u32)*(key+13))<<16) |
      (((u32)*(key+14))<<8) | (((u32)*(key+15)));
    s11=(((u32)*(key+16))<<24) | (((u32)*(key+17))<<16) |
      (((u32)*(key+18))<<8) | (((u32)*(key+19)));
    s10=(((u32)*(key+20))<<24) | (((u32)*(key+21))<<16) |
      (((u32)*(key+22))<<8) | (((u32)*(key+23)));
    s9=(((u32)*(key+24))<<24) | (((u32)*(key+25))<<16) |
      (((u32)*(key+26))<<8) | (((u32)*(key+27)));
    s8=(((u32)*(key+28))<<24) | (((u32)*(key+29))<<16) |
      (((u32)*(key+30))<<8) | (((u32)*(key+31)));
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
 for(i=0;i<2;i++)
    {

      outfrom_fsm=(r1+ s15 )^r2;
      s0 =a_mul(s0 )^ s2 ^ainv_mul(s11 )^outfrom_fsm;
      fsmtmp=r2+ s5 ;
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
 *   in keystream_block.
 * 
 * Returns: void
 * 
 */

INLINE void snow_keystream_fast(u32 *keystream_block) 
{

      s0 =a_mul(s0 )^ s2 ^ainv_mul(s11 );
      fsmtmp=r2+ s5 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[0]=(r1+ s0 )^r2^ s1 ;

      s1 =a_mul(s1 )^ s3 ^ainv_mul(s12 );
      fsmtmp=r2+ s6 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[1]=(r1+ s1 )^r2^ s2 ;

      s2 =a_mul(s2 )^ s4 ^ainv_mul(s13 );
      fsmtmp=r2+ s7 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[2]=(r1+ s2 )^r2^ s3 ;

      s3 =a_mul(s3 )^ s5 ^ainv_mul(s14 );
      fsmtmp=r2+ s8 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[3]=(r1+ s3 )^r2^ s4 ;

      s4 =a_mul(s4 )^ s6 ^ainv_mul(s15 );
      fsmtmp=r2+ s9 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[4]=(r1+ s4 )^r2^ s5 ;

      s5 =a_mul(s5 )^ s7 ^ainv_mul(s0 );
      fsmtmp=r2+ s10 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[5]=(r1+ s5 )^r2^ s6 ;

      s6 =a_mul(s6 )^ s8 ^ainv_mul(s1 );
      fsmtmp=r2+ s11 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[6]=(r1+ s6 )^r2^ s7 ;

      s7 =a_mul(s7 )^ s9 ^ainv_mul(s2 );
      fsmtmp=r2+ s12 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[7]=(r1+ s7 )^r2^ s8 ;

      s8 =a_mul(s8 )^ s10 ^ainv_mul(s3 );
      fsmtmp=r2+ s13 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp; 
      keystream_block[8]=(r1+ s8 )^r2^ s9 ;

      s9 =a_mul(s9 )^ s11 ^ainv_mul(s4 );
      fsmtmp=r2+ s14 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[9]=(r1+ s9 )^r2^ s10 ;

      s10 =a_mul(s10 )^ s12 ^ainv_mul(s5 );
      fsmtmp=r2+ s15 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[10]=(r1+ s10 )^r2^ s11 ;

      s11 =a_mul(s11 )^ s13 ^ainv_mul(s6 );
      fsmtmp=r2+ s0 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;    
      keystream_block[11]=(r1+ s11 )^r2^ s12 ;

      s12 =a_mul(s12 )^ s14 ^ainv_mul(s7 );
      fsmtmp=r2+ s1 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;
      keystream_block[12]=(r1+ s12 )^r2^ s13 ;

      s13 =a_mul(s13 )^ s15 ^ainv_mul(s8 );
      fsmtmp=r2+ s2 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;   
      keystream_block[13]=(r1+ s13 )^r2^ s14 ;

      s14 =a_mul(s14 )^ s0 ^ainv_mul(s9 );
      fsmtmp=r2+ s3 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp; 
      keystream_block[14]=(r1+ s14 )^r2^ s15 ;

      s15 =a_mul(s15 )^ s1 ^ainv_mul(s10 );
      fsmtmp=r2+ s4 ;
      r2=snow_T0[byte(0,r1)]^snow_T1[byte(1,r1)]^snow_T2[byte(2,r1)]^snow_T3[byte(3,r1)];
      r1=fsmtmp;    
      keystream_block[15]=(r1+ s15 )^r2^ s0 ;
}


