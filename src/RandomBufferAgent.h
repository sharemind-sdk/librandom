/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_RANDOMBUFFERAGENT_H
#define SHAREMINDCOMMON_RANDOMBUFFERAGENT_H

#include "IRandom.h"

#include <exception>
#include <sharemind/CircBufferSCSP.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#include <sharemind/Stoppable.h>
#include <thread>


namespace sharemind {

/** This class is a header-only front-end for randomness engines. */
class RandomBufferAgent: public IRandom {

public: /* Methods: */

    inline RandomBufferAgent(IRandom & randomEngine,
                             const size_t bufferSize)
        : m_engine(randomEngine)
        , m_buffer(bufferSize)
        , m_thread(&RandomBufferAgent::fillerThread, this) {}

    inline ~RandomBufferAgent() noexcept override {
        m_stoppable.stop();
        m_thread.join();
    }

    inline void Seed() noexcept override {
        m_engine.Seed();
    }

    virtual void Seed(const void * memptr, size_t size) noexcept override {
        m_engine.Seed(memptr, size);
    }

    /**
     * \brief Fills memory with randomly generated bytes.
     * \param[in] memptr pointer to memory to fill
     * \param[in] size number of bytes to fill
     */
    void fillBytes(void * memptr, size_t size) noexcept override {
        for (;;) {
            const size_t read = m_buffer.read(memptr, size);
            assert(read <= size);
            if (read >= size)
                return;
            memptr = PotentiallyVoidTypeInfo<void>::ptrAdd(memptr, read);
            size -= read;
            m_buffer.waitDataAvailable();
        }
    }

private: /* Methods: */

    void fillerThread() noexcept {
        struct GracefulStop {};
        typedef Stoppable::TestActor<GracefulStop> STA;
        struct POE: std::exception { inline POE(size_t const) noexcept {}; };
        try {
            try {
                for (;;) {
                    m_buffer.write<POE>(m_engine);
                    m_buffer.waitSpaceAvailable(STA(m_stoppable));
                }
            } catch (POE const &) {}
        } catch (const GracefulStop &) {}
    }

private: /* Fields: */

    IRandom & m_engine;
    CircBufferSCSP<void> m_buffer;
    Stoppable m_stoppable;
    std::thread m_thread;

}; /* class IRandom { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_RANDOMBUFFERAGENT_H */
