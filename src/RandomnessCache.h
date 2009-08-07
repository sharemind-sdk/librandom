/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#ifndef RANDOMNESSCACHE_H
#define RANDOMNESSCACHE_H

#include <deque>
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"

#define RANDOMNESS_CACHE_DEFAULT_SIZE 10000000
#define RANDOMNESS_GENERATION_PIECE 100000

/**
 This class manages the randomness cache filled in the background

 The randomness is taken from the FastRandom class.
 */
class RandomnessCache {
public:
	/**
	 * Create the thread
	 */
	RandomnessCache();

	/**
	 * Destructor of the thread wrapper.
	 *
	 * \note This does not destroy the thread.
	 */
	virtual ~RandomnessCache();

	/**
	 * The thread main loop
	 */
	void operator ()();

	/**
	 Initializes the cache by seeding randomness
	*/
	static void Initialize();

	/**
	 Fills a vector with random values

	 \param[in] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)

	 \return the number of values generated
	*/
	static uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	static val_t Generate();

	/**
	 * Set the cache limit to a given number of values.
	 *
	 * The cache will be generated until it contains that much values.
	 */
	static inline void SetCacheLimit (uint32 limit) {
		cacheLimit = limit;
	}

	/**
	 * Stops the generation process
	 */
	static void Stop () {
		WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Stopping the randomness thread.");
		randomness.clear ();
		runGeneration = false;
	}

	/**
	 * Enable or disable the cache
	 */
	static inline void SetUseCache (bool use) {
		useCache = use;
		if (!useCache) {
		    Stop ();
		}
	}

private:

	/**
	 * True, if the cache is ready to generate stuff
	 */
	static bool isInitialized;

	/**
	 * The configuration value that enables and disables the cache.
	 */
	static bool useCache;

	/**
	 * The loop controller
	 */
	static bool runGeneration;

	/**
	 The mutex guarding the global instance
	 */
	static boost::mutex theMutex;

	/**
	 The condition used for waiting for cache refill
	 */
	static boost::condition waitCondition;

	/**
	 The cache itself
	 */
	static std::deque<val_t> randomness;

	/**
	 The upper limit to the cache
	 */
	static uint32 cacheLimit;

	/**
	 The actual generator
	 */
	static FastRandom rng;

};

#endif // RANDOMNESSCACHE_H
