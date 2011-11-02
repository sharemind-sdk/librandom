/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_RANDOM_H
#define SHAREMINDCOMMON_RANDOM_H

#include "common/Random/RandomEngine.h"
#include "common/Logger/Logger.h"

namespace sharemind {


/**
 This class is a front end to randomness generation.
 */
class Random {

public:

	/**
	 The constructor
	 */
	Random(Logger& logger);

	/**
	 The constructor
	 */
	Random(Logger& logger, RandomEngineType engine);

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

	 \param[out] vec the vector of values to fill
	 \param[in] start the index to start filling from
	 \param[in] end the index to fill to (not included)

	 \return the number of values generated
	*/
	size_t FillVector(val_vector_t& vec, size_t start, size_t end);

private:

	/**
	 The actual engine used to generate randomness
	 */
	RandomEngine* engine;

	Logger& m_logger;

};

} // namespace sharemind

#endif // SHAREMINDCOMMON_RANDOM_H
