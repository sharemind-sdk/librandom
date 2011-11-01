/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef RANDOMENGINE_H
#define RANDOMENGINE_H

#include "common/SharemindTypes.h"
#include "common/Logger/Logger.h"

namespace sharemind {

/**
 * The randomness engine types available in the code.
 */
enum RandomEngineType {
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

	RandomEngine (Logger& logger)
	  : m_logger (logger)
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
	virtual size_t FillVector(val_vector_t& vec, size_t start, size_t end) = 0;

protected:

	Logger& m_logger;

};

} // namespace sharemind

#endif // RANDOMENGINE_H
