/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <GetTime.h>
#include "common/Logger/Logger.h"
#include "common/Random/OpenSSLRandomEngine.h"

namespace sharemind {

OpenSSLRandomEngine::OpenSSLRandomEngine(Logger& logger)
  : RandomEngine (logger)
{ }


OpenSSLRandomEngine::~OpenSSLRandomEngine() { }

void OpenSSLRandomEngine::Seed () {
	// Possibly incorporate more entropy
	WRITE_LOG_FULLDEBUG (m_logger, "Seeding OpenSSL randomness engine.");
}

void OpenSSLRandomEngine::fillBytes (void* memptr, size_t size) {
    bool RANDbytesOK = (RAND_bytes(static_cast<unsigned char*>(memptr), size) == 1);
    assert (RANDbytesOK);
}

} // namespace sharemind
