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

bool RandomnessCache::runGeneration = false;
uint32 RandomnessCache::cacheLimit = RANDOMNESS_CACHE_DEFAULT_SIZE;
boost::mutex RandomnessCache::theMutex;
boost::condition RandomnessCache::waitCondition;
std::deque<val_t> RandomnessCache::randomness;
CryptoPP::DefaultAutoSeededRNG RandomnessCache::strongRNG;
	
RandomnessCache::RandomnessCache () { }


RandomnessCache::~RandomnessCache () { }


void RandomnessCache::operator() () {

	RandomnessCache::runGeneration = true;

	WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Starting randomness thread.");

	while (RandomnessCache::runGeneration) {
		if (RandomnessCache::randomness.size () < cacheLimit) {
			boost::mutex::scoped_lock lock (RandomnessCache::theMutex);
			for (uint32 i = 0; i < RANDOMNESS_GENERATION_PIECE; i++) {
				RandomnessCache::randomness.push_back (RandomnessCache::strongRNG.GenerateWord32 ());
			}
			//WRITE_TO_LOG (LOG_FULLDEBUG, "Randomness cache size " << RandomnessCache::randomness.size () << ".");
			waitCondition.notify_all();
			boost::thread::yield ();
		}
	}

	WRITE_TO_LOG_NETNODE (LOG_DEBUG, "Stopped randomness thread.");

}


uint32 RandomnessCache::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	boost::mutex::scoped_lock lock (RandomnessCache::theMutex);
	
	if (!RandomnessCache::runGeneration) {
		WRITE_TO_LOG (LOG_MINIMAL, "ERROR: Generation thread not active, cannot generate randomness!");
		return 0;
	}
	
	//WRITE_TO_LOG (LOG_FULLDEBUG, "RNG: Required: " << end - start << " Available: " << RandomnessCache::randomness.size () << ".");
	
	// Check if we have enough
	if (randomness.size () < (end - start)) {
		// No, we wait
		while (randomness.size () < (end - start)) {
			waitCondition.wait (lock);
		}
	}
	
	uint32 count = 0;
	if (start >= vec.size () || end > vec.size () || start > end) {
		WRITE_TO_LOG (LOG_MINIMAL, "Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}
	
	for (uint32 i = start; i < end; i++) {
		// \todo See if this can be done more efficiently, some iterator magic maybe
		vec[i] = randomness.front ();
		randomness.pop_front ();
		count++;
	}
	
	return count;
}


val_t RandomnessCache::Generate() {
	boost::mutex::scoped_lock lock (RandomnessCache::theMutex);
	
	if (!RandomnessCache::runGeneration) {
		WRITE_TO_LOG (LOG_MINIMAL, "WARNING: Generation thread not active, generating on the go!");
		return RandomnessCache::strongRNG.GenerateWord32 ();
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
