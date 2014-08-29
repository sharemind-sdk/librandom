/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <cassert>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <mutex>
#include <memory>
#include <vector>

namespace /* anonymous */ {

/**
 * Initialize openssl and set locking callback.
 * This is required to make OpenSSLRandomEngine thread safe.
 */
class OpenSSLInit {

private: /* Methods: */

    OpenSSLInit() {
        SSL_library_init();
        if (size_t num_locks = CRYPTO_num_locks()) {
            m_mutexes.reserve(num_locks);
            do {
                m_mutexes.emplace_back(new std::mutex);
            } while (--num_locks);
            CRYPTO_set_locking_callback(&openssl_locking_callback);
        }
    }

    static void openssl_locking_callback(int mode,
                                         int n,
                                         const char *,
                                         int) noexcept
    {
        assert(n >= 0);
        assert(static_cast<unsigned>(n) < m_instance.m_mutexes.size());
        if (const auto & mutex = m_instance.m_mutexes[n]) {
            if (mode & CRYPTO_LOCK) {
                mutex->lock();
            } else {
                mutex->unlock();
            }
        }
    }

private: /* Fields: */

    static OpenSSLInit                       m_instance;
    std::vector<std::unique_ptr<std::mutex>> m_mutexes;

}; /* class OpenSSLInit { */

OpenSSLInit OpenSSLInit::m_instance;

} /* namespace anonymous { */
