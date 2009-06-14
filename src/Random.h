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
 A pseudorandomness generator. Relies on the ANSI X9.17 Appendix C 
 compliant generator from CryptoPP library.

 \note The generator is to be used with instances created. 
 The global generator is used to reinforce the seeds of the
 individual instances.
 
 \note The generator is not locked, so do not use one instance from several threads.
 
 \note Sometime in the future we might implement something like this:
 \code
 local_thread:
 get_rnd {
	64bit local_index
	64bit thread_index
	return get_AES_block(++local_index,thread_index) 
 }

 rnd_thread

 initialise_rnd()

 get_AES_block(local_index,tread_index) {
	aes_block = combine (local_index,tread_index)
	return AES(key,aes_block)
 }
 \endcode
*/
class Random {

public:

	/**
	 The constructor is private in a singleton
	 */
	Random();
	
	/**
	 The destructor of the randomness generator
	*/
	virtual ~Random();

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	val_t Generate();

	/**
	 Fills a vector with random values
	 
	 \param[in] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)
	 
	 \return the number of values generated
	*/
	uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

private:

	/**
	 An instance of the CryptoPP generator.
	*/
	CryptoPP::DefaultAutoSeededRNG strongRNG;
	
	/**
	 The global instance of this class
	 */
	static Random globalInstance;

	/**
	 The mutex guarding the global instance
	 */
	static boost::mutex globalInstanceMutex;
	
	/**
	 True, if the global instance is initialized
	 */
	static bool globallyInitialized;

};

#endif // RANDOM_H
