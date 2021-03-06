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

#ifndef SHAREMIND_LIBRANDOM_H
#define SHAREMIND_LIBRANDOM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Forward declarations.
 */

struct SharemindRandomFacility_;
typedef struct SharemindRandomFacility_ SharemindRandomFacility;

struct SharemindRandomEngine_;
typedef struct SharemindRandomEngine_ SharemindRandomEngine;

/**
 * \brief The core random number engine types.
 * \note This does not consider wrappers for core engines, such as the
 *       RandomBufferAgent, but only the generators that are directly able to
 *       generate random numbers. The "wrapper" engines are configured by
 *       selecting the appropriate buffering mode.
 */
typedef enum SharemindCoreRandomEngineKind_ {

    /** Random number engine that only outputs zeroes. Useful for debugging. */
    SHAREMIND_RANDOM_NULL = 0,

    /** Random number generator based on SNOW2 stream cipher. */
    SHAREMIND_RANDOM_SNOW2,

    /** Random number generator based on ChaCha20 stream cipher. */
    SHAREMIND_RANDOM_CHACHA20,

    /** Random number generator based on AES in CTR mode. */
    SHAREMIND_RANDOM_AES

} SharemindCoreRandomEngineKind;

/**
 * \brief Indicate what kind of buffering to use.
 */
typedef enum SharemindRandomEngineBufferingMode_ {
    /** No buffering. */
    SHAREMIND_RANDOM_BUFFERING_NONE,

    /** Threaded buffering. Randomness is collected in a background thread. */
    SHAREMIND_RANDOM_BUFFERING_THREAD,

} SharemindRandomEngineBufferingMode;

/**
 * \brief Random engine configuration.
 * This indicates what core RNG engine to use, how to buffer the generated
 * randomness and, if so, how large buffer (in bytes) to use.
*/
typedef struct SharemindRandomEngineConf_ {
    /** The core random number generator to use. */
    SharemindCoreRandomEngineKind      coreEngine;

    /** How to buffer the core generator. */
    SharemindRandomEngineBufferingMode bufferMode;

    /** The buffer size in bytes. Only used if relevant to the buffering mode.*/
    size_t                             bufferSize;
} SharemindRandomEngineConf;

/**
 * \brief Indicates if the RNG was constructed (and seeded) properly.
 */
typedef enum SharemindRandomEngineCtorError_ {

    SHAREMIND_RANDOM_OK = 0,

    /* General errors: */

    SHAREMIND_RANDOM_UNKNOWN_ERROR,
    SHAREMIND_RANDOM_GENERAL_ERROR,

    SHAREMIND_RANDOM_OUT_OF_MEMORY,

    /* Construction errors: */

    SHAREMIND_RANDOM_GENERATOR_NOT_SUPPORTED,

    /* Seeding errors: */

    SHAREMIND_RANDOM_SEED_TOO_SHORT,

    SHAREMIND_RANDOM_SEED_GENERATION_ERROR,

    SHAREMIND_RANDOM_SEED_NOT_SUPPORTED,

} SharemindRandomEngineCtorError;


/**
 * \brief Cryptographic random number generators.
 */
struct SharemindCryptographicRandom {
    /**
     * \param[out] memptr pointer to the memory region to randomize.
     * \param[in] size of the memory region to randomize.
     */
    void (* const RandomBlocking)(
            SharemindRandomFacility * facility,
            void * memptr,
            size_t size);

    /**
     * \param[out] memptr pointer to the memory region to randomize.
     * \param[in] size of the memory region to randomize.
     */
    void (* const URandomBlocking)(
            SharemindRandomFacility * facility,
            void * memptr,
            size_t size);

    /**
     * \param[out] memptr pointer to the memory region to randomize.
     * \param[in] size of the memory region to randomize.
     * \returns Number of bytes generated.
     */
    size_t (* const RandomNonblocking)(
            SharemindRandomFacility * facility,
            void * memptr,
            size_t size);

    /**
     * \param[out] memptr pointer to the memory region to randomize.
     * \param[in] size of the memory region to randomize.
     * \returns Number of bytes generated.
     */
    size_t (* const URandomNonblocking)(
            SharemindRandomFacility * facility,
            void * memptr,
            size_t size);
};

/**
 * \brief Facility for creating random number generation engines.
 */
struct SharemindRandomFacility_ {

    /**
     * \brief Cryptographic random number generators for entropy generation.
     */
    SharemindCryptographicRandom entropy;

    /**
     * \param[in] facility pointer to this factory facility.
     * \returns the default configuration (set in the configuration file).
     * \note only modify the configuration when you absolutely must use
     *       different random number generator than the one specified in the
     *       default configuration.
     */
    SharemindRandomEngineConf const * (* const defaultFactoryConfiguration)(
            SharemindRandomFacility const * facility);

    /**
     * \param[in] facility pointer to this factory facility.
     * \param[in] conf the configuration that specified which random engine to
     *                 use and how it's configured.
     * \returns the size of the seed required for the random engine specified
     *                 in the configuration.
     */
    size_t (* const getSeedSize)(
            SharemindRandomFacility * facility,
            SharemindRandomEngineConf const * conf);

    /**
     * \param[in] facility pointer to this factory facility.
     * \param[in] conf the configuration that specified which random engine to
     *                 use and how it's configured.
     * \param[in] memptr pointer to the seed.
     * \param[in] size of the seed.
     * \param[out] e error flag. Set only on error, not touched otherwise.
                     May be NULL.
     * \returns a new random number generation engine.
     * \brief construct a new random number generator with a given seed.
     * \see make_random_engine for details and other parameters.
     */
    SharemindRandomEngine * (* const createRandomEngineWithSeed)(
            SharemindRandomFacility * facility,
            SharemindRandomEngineConf const * conf,
            void const * memptr,
            size_t size,
            SharemindRandomEngineCtorError * e);

};

/**
 * \brief Random number generation engine.
 */
struct SharemindRandomEngine_ {

    /**
     * \param[in] rng pointer to this RNG engine.
     * \param[out] memptr memory region to randomize,
     * \param[in] size size of the memory region to randomize.
     */
    void (* const fillBytes)(SharemindRandomEngine * rng,
                              void * memptr,
                              size_t size);

};


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* SHAREMIND_LIBRANDOM_H */
