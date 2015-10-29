/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
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

    ~OpenSSLInit() {
        EVP_cleanup();
        SSL_COMP_free_compression_methods();
        CRYPTO_cleanup_all_ex_data();
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
