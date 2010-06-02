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

#include "../common/CommonLibrary.h"

Snow2RandomEngine::Snow2RandomEngine (Console* console) 
  : RandomEngine (console) {
	keystream_ready = 0;
}

Snow2RandomEngine::~Snow2RandomEngine () {}

void Snow2RandomEngine::Seed () {

	WRITE_LOG_FULLDEBUG (m_console, "[Snow2Random] Seeding SNOW 2 randomness engine.");
	// Generate the key
	OpenSSLRandomEngine rng(m_console);
	rng.GenerateBytes (snowkey, 32);

	// Make IVs random too
	uint32 iv0 = rng.Generate ();
	uint32 iv1 = rng.Generate ();
	uint32 iv2 = rng.Generate ();
	uint32 iv3 = rng.Generate ();
	snow_loadkey_fast (snowkey, 128, iv0, iv1, iv2, iv3);
}


uint32 Snow2RandomEngine::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	uint32 count = 0;
	for (uint32 i = start; i < end; i++) {
		vec[i] = Generate ();
		count++;
	}
	return count;
}
