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

struct SharemindRandomEngineFactoryFacility_;
typedef struct SharemindRandomEngineFactoryFacility_ SharemindRandomEngineFactoryFacility;

struct SharemindRandomEngine_;
typedef struct SharemindRandomEngine_ SharemindRandomEngine;

/**
 * \brief The core random number engine types.
 * \note This does not consider wrappers for core engines, such as the RandomBufferAgent,
 * but only the generators that are directly able to generate random numbers.
 * The "wrapper" engines are configured by selecting the appropriate buffering mode.
 */
typedef enum SharemindCoreRandomEngineKind_ {

    /** Random number engine that only outputs zeroes. Useful for debugging. */
    SHAREMIND_RANDOM_NULL = 0,

    /** Random number generator provided by openssl. */
    SHAREMIND_RANDOM_OPENSSL,

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
    SharemindCoreRandomEngineKind      core_engine;

    /** How to buffer the core generator. */
    SharemindRandomEngineBufferingMode buffer_mode;

    /** The buffer size in bytes. Only used if relevant to the buffering mode. */
    size_t                             buffer_size;
} SharemindRandomEngineConf;

/**
 * \brief Indicates if the RNG was constructed (and seeded) properly.
 */
typedef enum SharemindRandomEngineCtorError_ {

    /** Construction and seeding succeeded. */
    SHAREMIND_RANDOM_CTOR_OK = 0,

    /*
     * Construction errors:
     */

    /** The generator is not supported. */
    SHAREMIND_RANDOM_CTOR_GENERATOR_NOT_SUPPORTED,

    /** Construction failed because of a memory error (for example std::bad_alloc). */
    SHAREMIND_RANDOM_CTOR_OUT_OF_MEMORY,

    /** Construction failed due to some other problem (library not working as expected). */
    SHAREMIND_RANDOM_CTOR_OTHER_ERROR,

    /*
     * Seeding errors:
     */

    /** Seeding failed because the user supplied too short seed. */
    SHAREMIND_RANDOM_CTOR_SEED_TOO_SHORT,

    /** Seeding failed because generated a non-deterministic seed failed. */
    SHAREMIND_RANDOM_CTOR_SEED_SELF_GENERATE_ERROR,

    /** Seeding with a fixed seed is not supported by this generator. */
    SHAREMIND_RANDOM_CTOR_SEED_NOT_SUPPORTED,

    /** Seeding failed due to some other problem (library not working as expected). */
    SHAREMIND_RANDOM_CTOR_SEED_OTHER_ERROR,

} SharemindRandomEngineCtorError;

/**
 * \brief Facility for creating random number generation engines.
 */
struct SharemindRandomEngineFactoryFacility_ {

    /**
     * \param[in] facility pointer to this factory facility.
     * \returns the default configuration (set in the configuration file).
     * \note only modify the configuration when you absolutely must use
     * different random number generator than the one specified in the default
     * configuration.
     */
    SharemindRandomEngineConf (* const get_default_configuration)(
            const SharemindRandomEngineFactoryFacility* facility);

    /**
     * \param[in] facility pointer to this factory facility.
     * \param[in] conf the configuration that specified which random engine to use and how it's configured.
     * \param[out] e error flag. Set only on error, not touched otherwise. May be NULL.
     * \returns a new random number generation engine.
     * \brief construct a new random number generator with a fresh seed.
     */
    SharemindRandomEngine* (* const make_random_engine)(
            SharemindRandomEngineFactoryFacility* facility,
            SharemindRandomEngineConf conf,
            SharemindRandomEngineCtorError* e);
    /**
     * \param[in] memptr pointer to the seed.
     * \param[in] size of the seed.
     * \brief construct a new random number generator with a given seed.
     * \see make_random_engine for details and other parameters.
     */
    SharemindRandomEngine* (* const make_random_engine_with_seed)(
            SharemindRandomEngineFactoryFacility* facility,
            SharemindRandomEngineConf conf,
            const void* memptr,
            size_t size,
            SharemindRandomEngineCtorError* e);

    /**
     * \param[in] facility pointer to this factory facility.
     * \post The facility is no longer valid and any subsequent operations on it will have undefined behaviour.
     * \brief frees this factory facility.
     */
    void (* const free)(SharemindRandomEngineFactoryFacility* facility);
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
    void (* const fill_bytes)(SharemindRandomEngine* rng, void* memptr, size_t size);

    /**
     * \param[in] rng pointer to this RNG engine.
     * \post The RNG is no longer valid and any subsequent operations on it will have undefined behaviour.
     * \brief free the RNG.
     */
    void (* const free)(SharemindRandomEngine* rng);
};


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* SHAREMIND_LIBRANDOM_H */
