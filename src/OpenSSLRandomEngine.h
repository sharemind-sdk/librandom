/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef OPENSSLRANDOMENGINE_H
#define OPENSSLRANDOMENGINE_H

#include "common/RandomEngine.h"
#include <openssl/rand.h>

namespace sharemind {

/**
 A randomness engine based on OpenSSL RAND_bytes()
*/
class OpenSSLRandomEngine : public RandomEngine {

public:

	/**
	 Constructs the generator and seeds it
	 */
	OpenSSLRandomEngine(Logger& logger);

	/**
	 The destructor of the randomness generator
	*/
	virtual ~OpenSSLRandomEngine();

	/**
	 Seeds the generator
	*/
	void Seed();

	/**
	 Generates a single random value

	 \returns a single 32-bit random integer
	*/
	inline val_t Generate() {
		unsigned char rbuf[4];
		bool RANDbytesOK = (RAND_bytes(rbuf, 4) == 1);
		assert (RANDbytesOK);

		return (uint32)(rbuf[3] << 24 | rbuf[2] << 16 | rbuf[1] << 8 | rbuf[0]);
	}

	/**
	 Fills a vector with random values

	 \param[out] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)

	 \return the number of values generated
	*/
	uint32 FillVector(val_vector_t& vec, uint32 start, uint32 end);

	/**
	 Fills the given byte array

	 \param[out] ptr a pointer to a char array to fill
	 \param[in] num the number of bytes to generate
	*/
	void GenerateBytes(uint8* ptr, uint32 num);
};

} // namespace sharemind

#endif // OPENSSLRANDOMENGINE_H
