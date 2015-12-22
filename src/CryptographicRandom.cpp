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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "CryptographicRandom.h"

#include <cassert>
#include <cerrno>
#include <sharemind/abort.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>

#ifdef SHAREMIND_HAVE_LINUX_GETRANDOM
#error SHAREMIND_HAVE_LINUX_GETRANDOM should not be defined!
#endif

#ifndef __linux__
#error Platform not supported!
#endif

#include <sys/syscall.h>
#if defined(SYS_getrandom)
    #define SHAREMIND_HAVE_LINUX_GETRANDOM 1
    #include <linux/random.h>
    #include <unistd.h>
#else
    #include <fcntl.h>
    #include <mutex>
    #include <sharemind/abort.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

#ifndef SHAREMIND_HAVE_LINUX_GETRANDOM
namespace {

template <int extraFlags = 0>
struct CryptoRandomFile {

    int const m_fd;
    std::mutex m_mutex;

    inline CryptoRandomFile() noexcept
        : m_fd{::open("/dev/random", O_RDONLY | O_CLOEXEC | extraFlags)}
    {
        if (m_fd == -1)
            SHAREMIND_ABORT("Unable to open /dev/random!");
    }

    ssize_t read(void * const buf, size_t const bufSize) noexcept {
        std::lock_guard<std::mutex> const guard{m_mutex};
        return ::read(m_fd, buf, bufSize);
    }

    inline ~CryptoRandomFile() noexcept { ::close(m_fd); }

};

static CryptoRandomFile<> blockingCryptoRandomFile;
static CryptoRandomFile<O_NONBLOCK> nonblockingCryptoRandomFile;

} // anonymous namespace
#endif

SHAREMIND_EXTERN_C_BEGIN

size_t sharemindCryptographicRandomNonblocking(void * buf,
                                               size_t bufSize) noexcept
{
    assert(buf);
    if (bufSize <= 0u)
        return 0u;
    auto const r =
        #ifdef SHAREMIND_HAVE_LINUX_GETRANDOM
            ::syscall(SYS_getrandom, buf, bufSize, GRND_RANDOM | GRND_NONBLOCK);
        #else
            nonblockingCryptoRandomFile.read(buf, bufSize);
        #endif
    return (r >= 0) ? static_cast<size_t>(r) : 0u;
}

void sharemindCyptographicRandom(void * buf, size_t bufSize) noexcept {
    assert(buf);
    if (bufSize <= 0u)
        return;
    for (;;) {
        auto const r =
            #ifdef SHAREMIND_HAVE_LINUX_GETRANDOM
                ::syscall(SYS_getrandom, buf, bufSize, GRND_RANDOM);
            #else
                blockingCryptoRandomFile.read(buf, bufSize);
            #endif
        if (r < 0) {
            #if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
            assert(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK);
            #else
            assert(errno == EAGAIN || errno == EINTR);
            #endif
            continue;
        }
        size_t const randomRetrieved = static_cast<size_t>(r);
        assert(randomRetrieved <= bufSize);
        bufSize -= randomRetrieved;
        if (bufSize == 0u)
            return;
        buf = sharemind::ptrAdd(buf, randomRetrieved);
    }
}

SHAREMIND_EXTERN_C_END

#ifdef SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_TEST

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    using ElemType = uint8_t;
    static_assert(sizeof(ElemType) == 1u, "");
    static constexpr size_t const ARRAY_SIZE = 10u;
    static constexpr ElemType const ARRAY_FILL = 42;
    std::vector<ElemType> v(ARRAY_SIZE, ARRAY_FILL);
    auto const printVector = [&v]{
        for (auto const i : v)
            std::cout << static_cast<uintmax_t>(i) << ' ';
        std::cout << std::endl;
    };
    sharemind::cryptographicRandom(v.data(), ARRAY_SIZE);
    printVector();
    v.clear();
    std::cout << "------------------------" << std::endl;
    v.resize(ARRAY_SIZE, ARRAY_FILL);
    ElemType * buf = v.data();
    size_t sizeLeft = ARRAY_SIZE;
    for (;;) {
        size_t const r =
                sharemind::cryptographicRandomNonblocking(buf, sizeLeft);
        assert(r <= sizeLeft);
        sizeLeft -= r;
        if (sizeLeft <= 0u)
            break;
        buf += r;
    }
    printVector();
}

#endif // SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_TEST
