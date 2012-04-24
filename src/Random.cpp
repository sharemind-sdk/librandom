/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Random.h"

#include "../Logger/Logger.h"
#include "OpenSSLRandomEngine.h"
#include "Snow2RandomEngine.h"

namespace /* anonymous */ {

using namespace sharemind;

RandomEngine* newEngine (Logger& logger, RandomEngineType engineType) {
    RandomEngine* engine = 0;
    switch (engineType) {
    case RNG_SNOW2:
        engine = new Snow2RandomEngine (logger);
        WRITE_LOG_DEBUG (logger, "Using SNOW 2 randomness engine.");
        break;
    case RNG_OPENSSL:
        engine = new OpenSSLRandomEngine (logger);
        WRITE_LOG_DEBUG (logger, "Using OpenSSL randomness engine.");
        break;
    default:
        engine = new Snow2RandomEngine (logger);
        WRITE_LOG_DEBUG (logger, "Defaulting to SNOW 2 randomness engine.");
        break;
    }

    assert (engine);
    engine->Seed ();
    return engine;
}

} // namespace anonymous

namespace sharemind {

Random::Random(Logger& logger, RandomEngineType selectedEngine)
    : m_engine (newEngine (logger, selectedEngine))
{ }


Random::~Random() {
    delete m_engine;
}

RandomEngine::~RandomEngine() { }

} // namespace sharemind
