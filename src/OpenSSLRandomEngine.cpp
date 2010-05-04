/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner.
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */

#include <GetTime.h>
#include "common/CommonLibrary.h"

OpenSSLRandomEngine::OpenSSLRandomEngine(Console* console)
  : RandomEngine (console)
{ }


OpenSSLRandomEngine::~OpenSSLRandomEngine() { }

void OpenSSLRandomEngine::Seed () { 
	// Possibly incorporate more entropy
	WRITE_LOG_FULLDEBUG (m_console, "Seeding OpenSSL randomness engine.");	
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
