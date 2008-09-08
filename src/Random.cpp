/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) Dan Bogdanov, 2006-2008
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner.
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
	return strongRNG.GenerateWord32 ();
}
