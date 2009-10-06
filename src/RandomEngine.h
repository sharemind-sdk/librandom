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

enum RandomEngines {
	RNG_SNOW2,
	RNG_OPENSSL
};

/**
 This class is an abstract randomness generator
 */
class RandomEngine {

public:

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
	 
	 \param[in] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)
	 
	 \return the number of values generated
	*/
	virtual uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end) = 0;

};

#endif // RANDOMENGINE_H
