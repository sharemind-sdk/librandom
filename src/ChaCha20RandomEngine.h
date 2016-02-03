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

#ifndef SHAREMIND_LIBRANDOM_CHACHA20RANDOMENGINE_H
#define SHAREMIND_LIBRANDOM_CHACHA20RANDOMENGINE_H

#include "RandomEngine.h"
#include <cstdint>


namespace sharemind {


class ChaCha20RandomEngine: public RandomEngine {

private: /* Constants: */

    static constexpr size_t const CHACHA20_KEY_SIZE = 32u;
    static constexpr size_t const CHACHA20_BLOCK_SIZE = 64u;
    static constexpr size_t const CHACHA20_NONCE_SIZE = 8u;


    static constexpr size_t const CHACHA20_PARALLEL_BLOCK_COUNT = 4u;
    static constexpr size_t const CHACHA20_BUFFER_SIZE =
            CHACHA20_PARALLEL_BLOCK_COUNT * CHACHA20_BLOCK_SIZE;

public: /* Constants: */

    static constexpr size_t const SeedSize =
            CHACHA20_KEY_SIZE + CHACHA20_NONCE_SIZE;

public: /* Methods: */

    explicit ChaCha20RandomEngine(void const * seed) noexcept;

    void fillBytes(void * buffer, size_t bufferSize) noexcept override;

private: /* Fields: */

    /// Internal state of the ChaCha20 cipher:
    uint32_t m_state[16u];

    /**
     * \brief Number of bytes that have been consumed from the buffer.
     * \note Initially all bytes have been consumed from the uninitialized
     *  buffer.
     */
    size_t m_consumed_byte_count = CHACHA20_BUFFER_SIZE;

    /**
     * \brief A buffer of generated blocks.
     * \note Currently 4 blocks are generated at a time.
     */
    uint8_t m_block[CHACHA20_BUFFER_SIZE];

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_CHACHA20RANDOMENGINE_H */
