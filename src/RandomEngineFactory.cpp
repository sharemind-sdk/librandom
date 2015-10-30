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

#include "RandomEngineFactory.h"

#include "AESRandomEngine.h"
#include "ChaCha20RandomEngine.h"
#include "NullRandomEngine.h"
#include "OpenSSLRandomEngine.h"
#include "RandomBufferAgent.h"
#include "RandomEngine.h"
#include "SNOW2RandomEngine.h"

using namespace sharemind;

namespace /* anonymous */ {

extern "C" {

SharemindRandomEngineConf RandomEngineFactoryImpl_get_default_configuration(
        const SharemindRandomEngineFactoryFacility* facility);

SharemindRandomEngine* RandomEngineFactoryImpl_get_random_engine(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf);

}

class RandomEngineFactoryImpl: public SharemindRandomEngineFactoryFacility {
public: /* Methods: */

    inline RandomEngineFactoryImpl(SharemindRandomEngineConf conf)
        : SharemindRandomEngineFactoryFacility {
            RandomEngineFactoryImpl_get_default_configuration,
            RandomEngineFactoryImpl_get_random_engine
        }
        , m_conf (conf)
    { }

    inline static const RandomEngineFactoryImpl& fromWrapper(const SharemindRandomEngineFactoryFacility& base) noexcept {
        return static_cast<const RandomEngineFactoryImpl&>(base);
    }

public: /* Fields: */
    const SharemindRandomEngineConf m_conf;
};

extern "C"
SharemindRandomEngineConf RandomEngineFactoryImpl_get_default_configuration(
        const SharemindRandomEngineFactoryFacility* facility)
{
    assert (facility != nullptr);
    return RandomEngineFactoryImpl::fromWrapper(*facility).m_conf;
}

inline
SharemindRandomEngine* getCoreEngine(SharemindCoreRandomEngineKind kind) noexcept {
    try {
        switch (kind) {
        case SHAREMIND_RANDOM_NULL: return make_null_random_engine();
        case SHAREMIND_RANDOM_OPENSSL: return make_OpenSSL_random_engine();
        case SHAREMIND_RANDOM_SNOW2: return make_OpenSSL_random_engine();
        case SHAREMIND_RANDOM_CHACHA20: return make_ChaCha20_random_engine();
        case SHAREMIND_RANDOM_AES: return make_AES_random_engine();
        default:
            break;
        }
    }
    catch (...) {
        return nullptr;
    }

    return nullptr;
}

inline
SharemindRandomEngine* getThreadBufferedEngine(SharemindRandomEngine* coreEngine, size_t bufferSize) noexcept
{
    try {
        return make_thread_buffered_random_engine(RandomEngine {coreEngine}, bufferSize);
    }
    catch (...) {
        return nullptr;
    }
}

extern "C"
SharemindRandomEngine* RandomEngineFactoryImpl_get_random_engine(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf)
{
    assert (facility != nullptr);
    (void) facility;
    const auto coreEngine = getCoreEngine(conf.core_engine);
    switch (conf.buffer_mode) {
    case SHAREMIND_RANDOM_BUFFERING_NONE:
        return coreEngine;
    case SHAREMIND_RANDOM_BUFFERING_THREAD:
        return getThreadBufferedEngine(coreEngine, conf.buffer_size);
    default:
        break;
    }

    return nullptr;
}

} // namespace anonymous

namespace sharemind {

SharemindRandomEngineFactoryFacility* make_default_random_engine_factory(
        SharemindRandomEngineConf defaultConf)
{
    return new RandomEngineFactoryImpl {defaultConf};
}

} // namespace sharemind
