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

/**
 This class is a front end to randomness generation.
 
 The actual generation is done within the RandomnessCache class.
 
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
	static val_t Generate();

	/**
	 Fills a vector with random values
	 
	 \param[in] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)
	 
	 \return the number of values generated
	*/
	static uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

};

#endif // RANDOM_H
