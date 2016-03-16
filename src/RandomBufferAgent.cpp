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

#include <sharemind/PotentiallyVoidTypeInfo.h>


namespace sharemind {

RandomBufferAgent::RandomBufferAgent(
        std::unique_ptr<RandomEngine> randomEngine,
        size_t const bufferSize)
   : m_engine{std::move(randomEngine)}
   , m_buffer(bufferSize)
   , m_thread{&RandomBufferAgent::fillerThread, this}
{}

RandomBufferAgent::~RandomBufferAgent() noexcept {
    m_stoppable.stop();
    m_thread.join();
}

void RandomBufferAgent::fillBytes(void * buffer,
                                  size_t bufferSize) noexcept
{
    for (;;) {
        const auto read = m_buffer.read(buffer, bufferSize);
        assert(read <= bufferSize);
        if (read >= bufferSize)
            return;
        buffer = ptrAdd(buffer, read);
        bufferSize -= read;
        m_buffer.waitDataAvailable();
    }
}

namespace {

struct RandomEngineWriter {
    using Exception = int;
    size_t operator()(void * buffer, size_t const bufferSize) noexcept {
        engine.fillBytes(buffer, bufferSize);
        return bufferSize;
    }
    RandomEngine & engine;
};

} // anonymous namespace

void RandomBufferAgent::fillerThread() noexcept {
    struct GracefulStop {};
    try {
        for (;;) {
            m_buffer.write(RandomEngineWriter{*m_engine});
            m_buffer.waitSpaceAvailable(
                        Stoppable::TestActor<GracefulStop>{m_stoppable},
                        sharemind::StaticLoopDuration<10>{});
        }
    } catch (const GracefulStop &) {}
}

} /* namespace sharemind { */
