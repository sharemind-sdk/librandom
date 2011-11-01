/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "common/Random/Snow2RandomEngine.h"
#include "common/Random/OpenSSLRandomEngine.h"

using namespace sharemind;

Snow2RandomEngine::Snow2RandomEngine (Logger& logger)
  : RandomEngine (logger)
  , keystream_ready (0)
{
}

Snow2RandomEngine::~Snow2RandomEngine () {}

void Snow2RandomEngine::Seed () {

	WRITE_LOG_FULLDEBUG (m_logger, "[Snow2Random] Seeding SNOW 2 randomness engine.");
	// Generate the key
	OpenSSLRandomEngine rng(m_logger);
	rng.GenerateBytes (snowkey, 32);

	// Make IVs random too
	uint32_t iv0 = rng.Generate ();
	uint32_t iv1 = rng.Generate ();
	uint32_t iv2 = rng.Generate ();
	uint32_t iv3 = rng.Generate ();
	snow_loadkey_fast (snowkey, 128, iv0, iv1, iv2, iv3);
}


size_t Snow2RandomEngine::FillVector(val_vector_t& vec, size_t start, size_t end) {
	size_t count = 0;
	for (size_t i = start; i < end; i++) {
		vec[i] = Generate ();
		count++;
	}
	return count;
}
