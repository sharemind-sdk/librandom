/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */

#include "common/CommonLibrary.h"

extern "C" {
#include "snow2_fast.h"
}

FastRandom::FastRandom () {
	keystream_ready = 0;
}

FastRandom::~FastRandom () {}

void FastRandom::Seed () {
	// Generate the key
	AutoSeededRandom asr;
	asr.GenerateBytes (snowkey, 32);

	// Make IVs random too
	uint32 iv0 = asr.Generate ();
	uint32 iv1 = asr.Generate ();
	uint32 iv2 = asr.Generate ();
	uint32 iv3 = asr.Generate ();
	snow_loadkey_fast (snowkey, 128, iv0, iv1, iv2, iv3);
}


val_t FastRandom::Generate () {


	if (keystream_ready == 0) {
		snow_keystream_fast((unsigned long*)&keystream);
		keystream_ready = 16;
	}
	keystream_ready--;
	return keystream[keystream_ready];
}
