/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "IRandom.h"

#include "OpenSSLRandomEngine.h"
#include "Snow2RandomEngine.h"


namespace sharemind {

IRandom::Type IRandom::stringToRandomEngineType(const std::string & value)
        noexcept
{
    if (value.compare("SNOW2") == 0)
        return SNOW2;
    if (value.compare("OPENSSL") == 0)
        return OPENSSL;
    return INVALID;
}

IRandom * IRandom::newRandomEngine(const LogHard::Logger & logger,
                                   const IRandom::Type type)
{
    IRandom * engine;
    switch (type) {
        case OPENSSL:
            logger.debug() << "Initializing the OpenSSL randomness engine.";
            engine = new OpenSSLRandomEngine();
            break;
        case SNOW2:
            logger.debug() << "Initializing the SNOW 2 randomness engine.";
            engine = new Snow2RandomEngine();
            break;
        default:
            return nullptr;
    }
    engine->Seed();
    return engine;
}


} /* namespace sharemind { */
