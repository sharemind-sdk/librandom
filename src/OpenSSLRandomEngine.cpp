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

OpenSSLRandomEngine::OpenSSLRandomEngine() { }


OpenSSLRandomEngine::~OpenSSLRandomEngine() { }

void OpenSSLRandomEngine::Seed () { 
	// Possibly incorporate more entropy
}


void OpenSSLRandomEngine::GenerateBytes(uint8* ptr, uint32 num) {
	bool RANDbytesOK = (RAND_bytes(ptr, num) == 1);
	assert (RANDbytesOK);
}


uint32 OpenSSLRandomEngine::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	uint32 count = 0;
	for (uint32 i = start; i < end; i++) {
		vec[i] = Generate ();
		count++;
	}
	return count;
}
