/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
 * \brief Indicates if the RNG was seeded properly.
 */
typedef enum SharemindRandomEngineSeedError_ {

    /** Seeding succeeded. */
    SHAREMIND_RANDOM_SEED_OK = 0,

    /** Seeding failed because the generator is already seeded and reseeding is not supported. */
    SHAREMIND_RANDOM_SEED_NOT_SUPPORTED,

    /** Seeding failed because user supplied too little entropy. */
    SHAREMIND_RANDOM_SEED_INSUFFICIENT_ENTROPY,

    /** Seeding failed due to some internal problem such as a library not working as expected. */
    SHAREMIND_RANDOM_SEED_INTERNAL_ERROR,

    /** Seeding failed because of a hardware issue. */
    SHAREMIND_RANDOM_SEED_HARDWARE_ERROR
} SharemindRandomEngineSeedError;

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
 * \brief Facility for creating random number generation engines.
 */
struct SharemindRandomEngineFactoryFacility_ {
    /**
     * \param[in] facility pointer to this factory facility.
     * \returns the default configuration (set in the configuration file).
     * \note only modify the configuration when you absolutely must use
     * different random number generator than the one specified in the configuration.
     */
    SharemindRandomEngineConf_ (* const get_default_configuration)(
            const SharemindRandomEngineFactoryFacility* facility);

    /**
     * \param[in] facility pointer to this factory facility.
     * \param[in] conf the configuration that specified which random engine to use and how it's configured.
     * \returns a new random number generation engine.
     */
    SharemindRandomEngine* (* const get_random_engine)(
            const SharemindRandomEngineFactoryFacility* facility,
            SharemindRandomEngineConf conf);
};

/**
 * \param[in] rng pointer to this RNG engine.
 * \brief seed the random number generator using hardware randomness.
 */
typedef SharemindRandomEngineSeedError (* SharemindRandomEngineSeedHardwareCallback)(SharemindRandomEngine* rng);

/**
 * \param[in] rng pointer to this RNG engine.
 * \param[in] memptr pointer to the bytes to see the RNG with.
 * \param[in] size number of bytes to seed the RNG with.
 * \brief seed the random number generator.
 * \note if the supplied key is not sufficiently long it will be padded with zeroes.
 */
typedef SharemindRandomEngineSeedError (* SharemindRandomEngineSeedCallback)(
        SharemindRandomEngine* rng, const void* memptr, size_t size);

/**
 * \param[in] rng pointer to this RNG engine.
 * \param[out] memptr memory region to randomize,
 * \param[in] size size of the memory region to randomize.
 */
typedef void (* SharemindRandomEngineFillBytesCallback)(
        SharemindRandomEngine* rng, void* memptr, size_t size);

/**
 * \param[in] rng pointer to this RNG engine.
 * \post The RNG is no longer valid and any subsequent operations on it will have undefined behaviour.
 * \brief free the RNG.
 */
typedef void (* SharemindRandomEngineFreeCallback)(SharemindRandomEngine* rng);

/**
 * \brief Random number generation engine.
 */
struct SharemindRandomEngine_ {
    const size_t                                     seed_size;
    const SharemindRandomEngineSeedHardwareCallback  seed_hardware;
    const SharemindRandomEngineSeedCallback          seed;
    const SharemindRandomEngineFillBytesCallback     fill_bytes;
    const SharemindRandomEngineFreeCallback          free;
};


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* SHAREMIND_LIBRANDOM_H */
