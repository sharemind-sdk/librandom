/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#ifndef RANDOM_H
#define RANDOM_H

// CryptoPP RNG
#include <osrng.h>
#include "boost/thread/mutex.hpp"

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

	/**
	 Fills a vector with random values
	*/
	uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

private:

	/**
	 An instance of the CryptoPP generator.
	*/
	CryptoPP::DefaultAutoSeededRNG strongRNG;

	boost::mutex theMutex;

};

#endif // RANDOM_H
