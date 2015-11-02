/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
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
            logger.fullDebug() << "Initializing the OpenSSL randomness engine.";
            engine = new OpenSSLRandomEngine();
            break;
        case SNOW2:
            logger.fullDebug() << "Initializing the SNOW 2 randomness engine.";
            engine = new Snow2RandomEngine();
            break;
        default:
            return nullptr;
    }
    engine->Seed();
    return engine;
}


} /* namespace sharemind { */
