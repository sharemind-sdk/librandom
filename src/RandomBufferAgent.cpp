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


#include "RandomBufferAgent.h"
#include "RandomEngine.h"

#include <exception>
#include <sharemind/CircBufferSCSP.h>
#include <sharemind/Exception.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#include <sharemind/Stoppable.h>
#include <thread>

using namespace sharemind;

namespace /* anonymous */ {

extern "C" {
SharemindRandomEngineSeedError BufferedRandomEngine_seed_hardware(SharemindRandomEngine* rng_);
SharemindRandomEngineSeedError BufferedRandomEngine_seed(SharemindRandomEngine* rng_, const void * memptr_, size_t size);
void BufferedRandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size);
void BufferedRandomEngine_free(SharemindRandomEngine* rng_);
}

class RandomBufferAgent: public SharemindRandomEngine {
public: /* Methods: */
    SHAREMIND_DEFINE_EXCEPTION_UNUSED(std::exception, Exception);

    inline RandomBufferAgent(RandomEngine randomEngine,
                             const size_t bufferSize)
        : SharemindRandomEngine {
              size_t(0),
              BufferedRandomEngine_seed_hardware,
              BufferedRandomEngine_seed,
              BufferedRandomEngine_fill_bytes,
              BufferedRandomEngine_free
          }
        , m_engine {std::move (randomEngine)}
        , m_buffer (bufferSize)
        , m_thread {&RandomBufferAgent::fillerThread, this}
    { }

    inline ~RandomBufferAgent () {
        m_stoppable.stop ();
        m_thread.join ();
    }

    inline static RandomBufferAgent& fromWrapper(SharemindRandomEngine& base) noexcept {
        return static_cast<RandomBufferAgent&>(base);
    }

    /* Generate with inner generator! */
    inline size_t operator()(void* memptr, size_t size) noexcept {
        m_engine.fillBytes(memptr, size);
        return size;
    }

private: /* Methods: */

    void fillerThread() noexcept {
        struct GracefulStop {};
        try {
            for (;;) {
                m_buffer.write(*this);
                m_buffer.waitSpaceAvailable(
                            Stoppable::TestActor<GracefulStop>{m_stoppable},
                            sharemind::StaticLoopDuration<10>{});
            }
        } catch (const GracefulStop &) {}
    }

public: /* Fields: */
    RandomEngine m_engine;
    CircBufferSCSP<void> m_buffer;
    Stoppable m_stoppable;
    std::thread m_thread;
};

extern "C"
SharemindRandomEngineSeedError BufferedRandomEngine_seed_hardware(SharemindRandomEngine*) {
    return SHAREMIND_RANDOM_SEED_NOT_SUPPORTED;
}

extern "C"
SharemindRandomEngineSeedError BufferedRandomEngine_seed(SharemindRandomEngine*, const void*, size_t) {
    return SHAREMIND_RANDOM_SEED_NOT_SUPPORTED;
}

extern "C"
void BufferedRandomEngine_fill_bytes(SharemindRandomEngine* rng_, void* memptr, size_t size) {
    assert (rng_ != nullptr);
    assert (memptr != nullptr);

    auto& rng = RandomBufferAgent::fromWrapper(*rng_);
    for (;;) {
        const auto read = rng.m_buffer.read(memptr, size);
        assert(read <= size);
        if (read >= size)
            return;
        memptr = PotentiallyVoidTypeInfo<void>::ptrAdd(memptr, read);
        size -= read;
        rng.m_buffer.waitDataAvailable();
    }
}

extern "C"
void BufferedRandomEngine_free(SharemindRandomEngine* rng_) {
    assert (rng_ != nullptr);
    delete &RandomBufferAgent::fromWrapper(*rng_);
}

} // namespace anonymous

namespace sharemind {

SharemindRandomEngine* make_thread_buffered_random_engine(RandomEngine rng,
                                                          size_t bufferSize)
{
    return new RandomBufferAgent {std::move (rng), bufferSize};
}

} /* namespace sharemind { */
