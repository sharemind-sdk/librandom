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
#ifndef RANDOM_H
#define RANDOM_H

// CryptoPP RNG
#include <osrng.h>

/**
 A random generator. Relies on the ANSI X9.17 Appendix C compliant generator from CryptoPP library.

 \note The methods are not static so you must make an instance of it to use it.
*/
class Random {

public:
	/**
	 Initializes the random generator
	*/
	Random();


	/**
	 Cleans up after the generatorm if necessary
	*/
	virtual ~Random();

	/**
	 Generates a random value

	 \returns a random value
	*/
	val_t Generate();

private:

	/**
	 An instance of the CryptoPP generator.
	*/
	CryptoPP::DefaultAutoSeededRNG strongRNG;

};

#endif // RANDOM_H
