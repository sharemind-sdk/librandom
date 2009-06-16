/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */

#include <cstdlib>
#include <ctime>
#include <GetTime.h>
#include <osrng.h>
#include "Sharemind.h"

// Static member initializations
Random Random::globalInstance;
boost::mutex Random::globalInstanceMutex;
bool Random::globallyInitialized;

Random::Random() { 

	// If the global instance exists, use it to help seed local instances.
	if (Random::globallyInitialized) {
		boost::mutex::scoped_lock lock (Random::globalInstanceMutex);
		if (strongRNG.CanIncorporateEntropy ()) {
			byte additionalEntropy[128];
			Random::globalInstance.strongRNG.GenerateBlock (additionalEntropy, 128);
			strongRNG.IncorporateEntropy (additionalEntropy, 128);
		}
	} else {
		Random::globallyInitialized = true;
	}
}


Random::~Random() { }


val_t Random::Generate() {
	return strongRNG.GenerateWord32 ();
}


uint32 Random::FillVector(val_vector_t& vec, uint32 start, uint32 end) {	
	uint32 methodSectionId = ExecutionProfiler::StartSection (ACTION_RANDOMNESS_GENERATION, end - start);

	uint32 count = 0;
	if (start >= vec.size () || end > vec.size () || start > end) {
		WRITE_TO_LOG (LOG_MINIMAL, "Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}
	
	for (uint32 i = start; i < end; i++) {
		vec[i] = strongRNG.GenerateWord32 ();
		count++;
	}
	
	ExecutionProfiler::EndSection (methodSectionId);
	return count;
}
