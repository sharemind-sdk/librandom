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


Random::Random() { }


Random::~Random() { }


val_t Random::Generate() {
	// We lock it to preserve the AES state's consistency
	//boost::mutex::scoped_lock lock (theMutex);
	return strongRNG.GenerateWord32 ();
}

uint32 Random::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	uint32 count = 0;
	if (start >= vec.size () || end >= vec.size () || start > end) {
		WRITE_TO_LOG (LOG_MINIMAL, "Cannot fill range (" << start << "-" << end << ") with randomness in vector of size " << vec.size () << ".");
		return 0;
	}
	for (uint32 i = start; i < end; i++) {
		vec[i] = strongRNG.GenerateWord32 ();
		count++;
	}
	return count;
}
