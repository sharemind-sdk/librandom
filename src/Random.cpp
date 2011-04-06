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
#include "../common/CommonLibrary.h"

Random::Random(Logger* logger)
  : m_logger (logger)
{
	engine = new Snow2RandomEngine (m_logger);
	WRITE_LOG_DEBUG (m_logger, "Defaulting to SNOW 2 randomness engine.");
	assert (engine);
	engine->Seed ();
}


Random::Random(Logger* logger, RandomEngines selectedEngine)
  : m_logger (logger)
{
	if (selectedEngine == RNG_SNOW2) {
		engine = new Snow2RandomEngine (m_logger);
		WRITE_LOG_DEBUG (m_logger, "Using SNOW 2 randomness engine.");
	} else 	if (selectedEngine == RNG_OPENSSL) {
		engine = new OpenSSLRandomEngine (m_logger);
		WRITE_LOG_DEBUG (m_logger, "Using OpenSSL randomness engine.");
	} else {
		engine = new Snow2RandomEngine (m_logger);
		WRITE_LOG_DEBUG (m_logger, "Defaulting to SNOW 2 randomness engine.");
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
		WRITE_LOG_ERROR (m_logger, "[Random] Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}

	//uint32 methodSectionId = ExecutionProfiler::StartSection (ACTION_RANDOMNESS_GENERATION, end - start);
	uint32 rv = engine->FillVector (vec, start, end);
	//ExecutionProfiler::EndSection (methodSectionId);
	return rv;
}

RandomEngine::~RandomEngine() { }
