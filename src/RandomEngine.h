/*
 * This file is a part of the Sharemind framework.
 *
 * Copyright (C) AS Cybernetica
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner.
 *
 * Main contributors:
 * Dan Bogdanov (dan@cyber.ee)
 */
#ifndef RANDOMENGINE_H
#define RANDOMENGINE_H

/**
 * The randomness engines available in the code.
 */	
enum RandomEngines {
	/*! An engine based on the SNOW2 stream cipher */
	RNG_SNOW2,
	/*! An engine based on the OpenSSL RAND_bytes function */
	RNG_OPENSSL
};

/**
 This class is an abstract randomness generator
 */
class RandomEngine {

public:

	RandomEngine (Console* console) 
	  : m_console (console)
	{ }
	
	/**
	 RNG destructor
	*/
	virtual ~RandomEngine() = 0;

	/**
	 Seeds the generator
	*/
	virtual void Seed() = 0;

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	virtual val_t Generate() = 0;

	/**
	 Fills a vector with random values
	 
	 \param[out] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)
	 
	 \return the number of values generated
	*/
	virtual uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end) = 0;

protected:
	
	Console* m_console;
	
};

#endif // RANDOMENGINE_H