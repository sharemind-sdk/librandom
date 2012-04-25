/*
 * HEADER FILE FOR FAST IMPLEMENTATION OF THE STREAM CIPHER SNOW 2.0
 *
 * Filename: snow2_fast.h
 *
 * Authors:
 * Patrik Ekdahl & Thomas Johansson
 * Dept. of Information Technology
 * P.O. Box 118
 * SE-221 00 Lund, Sweden,
 * email: {patrik,thomas}@it.lth.se
 *
 * Synopsis:
 *  Header file for snow2_fast.c
 *
 *  Interface:
 *    Two functions are exported in snow2_fast.h
 *    snow_loadkey_fast and snow_keystream_fast
 *
 *    Check snow2_fast.h for calling conventions.
 *
 */
#ifndef SHAREMINDCOMMON_SNOW2_FAST_H
#define SHAREMINDCOMMON_SNOW2_FAST_H

#include <stdint.h>
/* Exported functions */


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
 *   thus key[0]->msb of s15
 *        key[1]->second msb of s15
 *         ...
 *        key[3]-> lsb of s15
 *         ...
 */
extern void snow_loadkey_fast(uint8_t *key,
                   uint32_t keysize,
                   uint32_t IV3, uint32_t IV2,
                   uint32_t IV1, uint32_t IV0);

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
extern void snow_keystream_fast(uint32_t *keystream_block);

#endif
