/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#ifndef AUTOSEEDEDRANDOM_H
#define AUTOSEEDEDRANDOM_H

/**
 An automatically seeded randomness generator used to seed
 the fast generator in Sharemind
 
 The current implementation is based on OpenSSL
*/
class AutoSeededRandom {

public:

	/**
	 Constructs the generator and seeds it
	 */
	AutoSeededRandom();
	
	/**
	 The destructor of the randomness generator
	*/
	virtual ~AutoSeededRandom();

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	val_t Generate();

	/**
	 Fills the given byte array
	 
	 \param[in] ptr a pointer to a char array to fill
	 \param[in] num the number of bytes to generate
	*/
	void GenerateBytes(uint8* ptr, uint32 num);
};

#endif // AUTOSEEDEDRANDOM_H
