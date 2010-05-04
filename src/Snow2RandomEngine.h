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
#ifndef SNOW2RANDOMENGINE_H
#define SNOW2RANDOMENGINE_H

#include "common/RandomEngine.h"
extern "C" {
#include "snow2_fast.h"
}

/**
 A randomness engine based on the SNOW 2 stream cipher.

 Initial seed is provided by the OpenSSL random engine.
*/
class Snow2RandomEngine : public RandomEngine {

public:
	/**
	 Constructs the generator
	 */
	Snow2RandomEngine (Console* console);

	/**
	 The destructor of the randomness generator
	*/
	virtual ~Snow2RandomEngine();

	/**
	 Seeds the generator
	*/
	void Seed();

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	inline val_t Generate() {
		if (keystream_ready == 0) {
			snow_keystream_fast(keystream);
			keystream_ready = 16;
		}
		keystream_ready--;
		return keystream[keystream_ready];
	}
	
	/**
	 Fills a vector with random values
	 
	 \param[out] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)
	 
	 \return the number of values generated
	*/
	uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

private:
	uint8 snowkey[32];
	uint8 keystream_ready;
	uint32 keystream[16];

};

#endif // SNOW2RANDOMENGINE_H
