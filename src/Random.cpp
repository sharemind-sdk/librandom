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
#include "Sharemind.h"


Random::Random() {
	if (NodeConfiguration::rngEngine == RNG_SNOW2) {
		engine = new Snow2RandomEngine ();
		WRITE_TO_LOG (LOG_DEBUG, "Using SNOW 2 randomness engine.");
	} else 	if (NodeConfiguration::rngEngine == RNG_OPENSSL) {
		engine = new OpenSSLRandomEngine ();
		WRITE_TO_LOG (LOG_DEBUG, "Using OpenSSL randomness engine.");
	} else {
		engine = new Snow2RandomEngine ();
		WRITE_TO_LOG (LOG_DEBUG, "Defaulting to SNOW 2 randomness engine.");
	}
	assert (engine);
	engine->Seed ();
}


Random::~Random() { }


val_t Random::Generate() {
	return engine->Generate ();
}


uint32 Random::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
    // Verify range
	if (start >= vec.size () || end > vec.size () || start > end) {
		WRITE_TO_LOG (LOG_MINIMAL, "Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}

	uint32 methodSectionId = ExecutionProfiler::StartSection (ACTION_RANDOMNESS_GENERATION, end - start);
	uint32 rv = engine->FillVector (vec, start, end);
	ExecutionProfiler::EndSection (methodSectionId);
	return rv;
}

RandomEngine::~RandomEngine() { }
