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

Random::Random(Console* console) 
  : m_console (console) 
{
	engine = new Snow2RandomEngine (m_console);
	WRITE_LOG_DEBUG (m_console, "Defaulting to SNOW 2 randomness engine.");
	assert (engine);
	engine->Seed ();
}


Random::Random(Console* console, RandomEngines selectedEngine)
  : m_console (console) 
{
	if (selectedEngine == RNG_SNOW2) {
		engine = new Snow2RandomEngine (m_console);
		WRITE_LOG_DEBUG (m_console, "Using SNOW 2 randomness engine.");
	} else 	if (selectedEngine == RNG_OPENSSL) {
		engine = new OpenSSLRandomEngine (m_console);
		WRITE_LOG_DEBUG (m_console, "Using OpenSSL randomness engine.");
	} else {
		engine = new Snow2RandomEngine (m_console);
		WRITE_LOG_DEBUG (m_console, "Defaulting to SNOW 2 randomness engine.");
	}
	assert (engine);
	engine->Seed ();
}


Random::~Random() {
    if (engine)
        delete (engine);
}


val_t Random::Generate() {
	return engine->Generate ();
}


uint32 Random::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
    // Verify range
	if (start >= vec.size () || end > vec.size () || start > end) {
		WRITE_LOG_ERROR (m_console, "[Random] Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}

	uint32 methodSectionId = ExecutionProfiler::StartSection (ACTION_RANDOMNESS_GENERATION, end - start);
	uint32 rv = engine->FillVector (vec, start, end);
	ExecutionProfiler::EndSection (methodSectionId);
	return rv;
}

RandomEngine::~RandomEngine() { }
