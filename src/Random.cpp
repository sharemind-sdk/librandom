/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (db@math.ut.ee)
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
