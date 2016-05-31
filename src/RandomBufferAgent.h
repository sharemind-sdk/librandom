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

#ifndef SHAREMIND_LIBRANDOM_RANDOMBUFFERAGENT_H
#define SHAREMIND_LIBRANDOM_RANDOMBUFFERAGENT_H

#include "RandomEngine.h"

#include <cstddef>
#include <memory>
#include <sharemind/CircBufferSCSP.h>
#include <sharemind/Stoppable.h>
#include <thread>
#include "librandom.h"


namespace sharemind {

class RandomBufferAgent: public RandomEngine {

public: /* Methods: */

    RandomBufferAgent(std::shared_ptr<RandomEngine> randomEngine,
                      size_t const bufferSize);

    ~RandomBufferAgent() noexcept override;

    void fillBytes(void * buffer, size_t bufferSize) noexcept override;

private: /* Methods: */

    void fillerThread() noexcept;

public: /* Fields: */

    std::shared_ptr<RandomEngine> m_engine;
    CircBufferSCSP<void> m_buffer;
    Stoppable m_stoppable;
    std::thread m_thread;

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_RANDOMBUFFERAGENT_H */
