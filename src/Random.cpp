/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <GetTime.h>
#include "common/Logger/Logger.h"
#include "common/Random/Random.h"
#include "common/Random/Snow2RandomEngine.h"
#include "common/Random/OpenSSLRandomEngine.h"

using namespace sharemind;

Random::Random(Logger& logger)
  : m_logger (logger)
{
	engine = new Snow2RandomEngine (m_logger);
	WRITE_LOG_DEBUG (m_logger, "Defaulting to SNOW 2 randomness engine.");
	assert (engine);
	engine->Seed ();
}


Random::Random(Logger& logger, RandomEngineType selectedEngine)
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

RandomEngine::~RandomEngine() { }
