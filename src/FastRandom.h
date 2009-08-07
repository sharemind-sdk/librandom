/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica, 2006-2009
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#ifndef FASTRANDOM_H
#define FASTRANDOM_H

/**
 A fast pseudorandom generator for use in Sharemind protocols

 The current implementation is based on the SNOW stream cipher.
 Initial seed is provided by the AutoSeededRandom class.
*/
class FastRandom {

public:
	/**
	 Constructs the generator and seeds it from AutoSeededRandom
	 */
	FastRandom ();	

	/**
	 The destructor of the randomness generator
	*/
	virtual ~FastRandom();

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	val_t Generate();

private:
	uint8 snowkey[32];
	uint8 keystream_ready;
	uint32 keystream[16];

};

#endif // FASTRANDOM_H
