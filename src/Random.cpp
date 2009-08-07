/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */

#include <GetTime.h>
#include "Sharemind.h"


Random::Random() {
    // We do this here so that it gets randomized in any case
    RandomnessCache::Initialize();
}


Random::~Random() { }


val_t Random::Generate() {
	return RandomnessCache::Generate ();
}


uint32 Random::FillVector(val_vector_t& vec, uint32 start, uint32 end) {
	//uint32 methodSectionId = ExecutionProfiler::StartSection (ACTION_RANDOMNESS_GENERATION, end - start);
	uint32 rv = RandomnessCache::FillVector (vec, start, end);
	//ExecutionProfiler::EndSection (methodSectionId);
	return rv;
}
