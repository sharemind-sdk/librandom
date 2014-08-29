/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

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

    OpenSSLInit () {
        SSL_library_init ();

        const size_t num_locks = CRYPTO_num_locks ();
        m_mutexes.reserve (num_locks);
        for (size_t i = 0; i < num_locks; ++ i) {
            m_mutexes.emplace_back (new std::mutex);
        }

        CRYPTO_set_locking_callback (&openssl_locking_callback);
    }

    static void openssl_locking_callback (int mode, int n, const char*, int) {
        if (auto & mut = m_instance.m_mutexes.at (n)) {
            if (mode & CRYPTO_LOCK)
                mut->lock ();
            else
                mut->unlock ();
        }
    }

private: /* Fields: */
    static OpenSSLInit                       m_instance;
    std::vector<std::unique_ptr<std::mutex>> m_mutexes;
}; /* class OpenSSLInit { */

OpenSSLInit OpenSSLInit::m_instance;

} /* namespace anonymous { */
