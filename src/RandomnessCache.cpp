/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#include "CommonLibrary.h"

#include <boost/thread.hpp>
using boost::thread;

bool RandomnessCache::useCache = true;
bool RandomnessCache::runGeneration = false;
uint32 RandomnessCache::cacheLimit = RANDOMNESS_CACHE_DEFAULT_SIZE;
boost::mutex RandomnessCache::theMutex;
boost::condition RandomnessCache::waitCondition;
std::deque<val_t> RandomnessCache::randomness;
FastRandom RandomnessCache::rng;

RandomnessCache::RandomnessCache () { }


RandomnessCache::~RandomnessCache () { }


void RandomnessCache::operator() () {

    if (!useCache) {
        WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Randomness cache disabled, generating on the go.");
        return;
    }

	RandomnessCache::runGeneration = true;

	WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Starting randomness thread.");

	while (RandomnessCache::runGeneration) {
		if (RandomnessCache::randomness.size () < cacheLimit) {
			boost::mutex::scoped_lock lock (RandomnessCache::theMutex);
			for (uint32 i = 0; i < RANDOMNESS_GENERATION_PIECE; i++) {
				RandomnessCache::randomness.push_back (RandomnessCache::rng.Generate ());
			}
			//WRITE_TO_LOG (LOG_FULLDEBUG, "Randomness cache size " << RandomnessCache::randomness.size () << ".");
			waitCondition.notify_all();
			Console::Rest (1);
			boost::thread::yield ();
		}
	}

	WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Stopped randomness thread.");

}


uint32 RandomnessCache::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	boost::mutex::scoped_lock lock (RandomnessCache::theMutex);

    // Verify range
	uint32 count = 0;
	if (start >= vec.size () || end > vec.size () || start > end) {
		WRITE_TO_LOG (LOG_MINIMAL, "Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}

    // If the cache is disabled, generate as we go
    if (!useCache || !runGeneration) {

        for (uint32 i = start; i < end; i++) {
            vec[i] = RandomnessCache::rng.Generate ();
            count++;
        }

    } else {

        // Check if we have enough
        //WRITE_TO_LOG (LOG_FULLDEBUG, "RNG: Required: " << end - start << " Available: " << RandomnessCache::randomness.size () << ".");
        if (randomness.size () < (end - start)) {
            // No, we wait
            while (randomness.size () < (end - start)) {
                waitCondition.wait (lock);
            }
        }

        for (uint32 i = start; i < end; i++) {
            // \todo See if this can be done more efficiently, some iterator magic maybe
            vec[i] = randomness.front ();
            randomness.pop_front ();
            count++;
        }
    }

	return count;
}


val_t RandomnessCache::Generate() {
	boost::mutex::scoped_lock lock (RandomnessCache::theMutex);

	if (!useCache || !runGeneration) {
		return RandomnessCache::rng.Generate ();
	}

	// Check if we have enough
	if (randomness.size () < 1) {
		// No, we wait
		while (randomness.size () < 1) {
			waitCondition.wait (lock);
		}
	}

	val_t rv = randomness.front ();
	randomness.pop_front ();
	return rv;
}
