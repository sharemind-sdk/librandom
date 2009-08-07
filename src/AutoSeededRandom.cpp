/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */

#include <GetTime.h>
#include "Sharemind.h"
#include <openssl/rand.h>

AutoSeededRandom::AutoSeededRandom() { }


AutoSeededRandom::~AutoSeededRandom() { }


val_t AutoSeededRandom::Generate() {
	unsigned char rbuf[4];
	bool RANDbytesOK = (RAND_bytes(rbuf, 4) == 1);
	assert (RANDbytesOK);
	
	return (uint32)(rbuf[3] << 24 | rbuf[2] << 16 | rbuf[1] << 8 | rbuf[0]);
}


void AutoSeededRandom::GenerateBytes(uint8* ptr, uint32 num) {
	bool RANDbytesOK = (RAND_bytes(ptr, num) == 1);
	assert (RANDbytesOK);
}
