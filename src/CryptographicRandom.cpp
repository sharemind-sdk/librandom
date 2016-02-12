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

/*
  See also:
    * drivers/char/random.c in the Linux kernel
    * `man 4 urandom`
    * `man 2 getrandom`
    * http://www.2uo.de/myths-about-urandom/
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
    #warning No getrandom(2) detected!
    #warning Falling back to slow alternative of reading from /dev/u?random.
    #include <fcntl.h>
    #include <linux/random.h>
    #include <mutex>
    #include <sharemind/abort.h>
    #include <sys/ioctl.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

#ifndef SHAREMIND_HAVE_LINUX_GETRANDOM
namespace {

/* Always wait for at least 256 bits of entropy which is required for the Linux
   kernel to initialize its entropy pool for the CSPRNG used for both
   /dev/random and /dev/urandom. */
void waitForEntropyInitialization(char const * filename, int const fd) {
    // Ensure this is executed only once:
    static bool entropyPoolInitialized = false;
    if (!entropyPoolInitialized) {
        bool globalFlagSet = false;
        key_t const key = ::ftok(filename, 's');
        if (key == -1) \
            SHAREMIND_ABORT("ftok() failed with errno = %d!\n",
                            errno);
        int estimatedEntropyBits = 0;
        for (;;) {
            int const shmid = ::shmget(key, 1, 0444);
            if (shmid == -1) {
                /* ENOENT No segment exists for the given key. */
                if (errno != ENOENT)
                    SHAREMIND_ABORT("shmget() failed with errno = %d!\n",
                                    errno);
            } else {
                /* Segment already exists. */
                globalFlagSet = true;
                break;
            }
            /* According to `man 4 urandom` the size returned is the same as
               from /proc/sys/kernel/random/entropy_avail which is the number of
               estimated BITS (!!!) of entropy. */
            if (ioctl(fd, RNDGETENTCNT, &estimatedEntropyBits) != 0)
                SHAREMIND_ABORT("ioctl(RNDGETENTCNT) failed with errno = %d!\n",
                                errno);
            if (estimatedEntropyBits >= 256)
                break;
            sleep(1u);
        }
        if (!globalFlagSet) {
            int const shmid = ::shmget(key, 1, 0644 | IPC_CREAT | IPC_EXCL);
            if (shmid == -1) {
                /* EEXIST Segment already exists for key. */
                if (errno != EEXIST)
                    SHAREMIND_ABORT("shmget() failed with errno = %d!\n",
                                    errno);
            }
        }
        entropyPoolInitialized = true;
    }
}

#define SHAREMIND_LIBRANDOM_CRF(classname,filename) \
    template <int extraFlags = 0> \
    struct classname { \
        int const m_fd; \
        std::mutex m_mutex; \
        inline classname() noexcept \
            : m_fd{::open((filename), O_RDONLY | O_CLOEXEC | extraFlags)} \
        { \
            if (m_fd == -1) \
                SHAREMIND_ABORT("Unable to open " filename "!"); \
            waitForEntropyInitialization(filename, m_fd); \
        } \
        ssize_t read(void * const buf, size_t const bufSize) noexcept { \
            std::lock_guard<std::mutex> const guard{m_mutex}; \
            return ::read(m_fd, buf, bufSize); \
        } \
        inline ~classname() noexcept { ::close(m_fd); } \
    }
SHAREMIND_LIBRANDOM_CRF(CryptoRandomFile,"/dev/random");
static CryptoRandomFile<> blockingCryptoRandomFile;
static CryptoRandomFile<O_NONBLOCK> nonblockingCryptoRandomFile;

SHAREMIND_LIBRANDOM_CRF(CryptoURandomFile,"/dev/urandom");
static CryptoURandomFile<> blockingCryptoURandomFile;
static CryptoURandomFile<O_NONBLOCK> nonblockingCryptoURandomFile;

} // anonymous namespace
#endif

SHAREMIND_EXTERN_C_BEGIN

#ifdef SHAREMIND_HAVE_LINUX_GETRANDOM
    #define SHAREMIND_LIBRANDOM_GETRANDOM(flags,randomFile) \
        ::syscall(SYS_getrandom, buf, bufSize, (flags));
#else
    #define SHAREMIND_LIBRANDOM_GETRANDOM(flags,randomFile) \
        randomFile.read(buf, bufSize);
#endif

#define SHAREMIND_LIBRANDOM_NBR(fName,extraFlags,randomFile) \
    size_t fName(void * buf, size_t bufSize) noexcept { \
        assert(buf); \
        if (bufSize <= 0u) \
            return 0u; \
        auto const r = \
            SHAREMIND_LIBRANDOM_GETRANDOM(GRND_NONBLOCK | extraFlags, \
                                          randomFile); \
        return (r >= 0) ? static_cast<size_t>(r) : 0u; \
    }

SHAREMIND_LIBRANDOM_NBR(sharemindCryptographicRandomNonblocking,
                        GRND_RANDOM,
                        nonblockingCryptoRandomFile)
SHAREMIND_LIBRANDOM_NBR(sharemindCryptographicURandomNonblocking,
                        0,
                        nonblockingCryptoURandomFile)

#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
#define SHAREMIND_LIBRANDOM_RETRYASSERT \
    assert(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
#else
#define SHAREMIND_LIBRANDOM_RETRYASSERT \
    assert(errno == EAGAIN || errno == EINTR)
#endif

#define SHAREMIND_LIBRANDOM_BR(fName,extraFlags,randomFile) \
    void fName(void * buf, size_t bufSize) noexcept { \
        assert(buf); \
        if (bufSize <= 0u) \
            return; \
        for (;;) { \
            auto const r = \
                    SHAREMIND_LIBRANDOM_GETRANDOM(extraFlags, randomFile); \
            if (r < 0) { \
                SHAREMIND_LIBRANDOM_RETRYASSERT; \
                continue; \
            } \
            size_t const randomRetrieved = static_cast<size_t>(r); \
            assert(randomRetrieved <= bufSize); \
            bufSize -= randomRetrieved; \
            if (bufSize == 0u) \
                return; \
            buf = sharemind::ptrAdd(buf, randomRetrieved); \
        } \
    }

SHAREMIND_LIBRANDOM_BR(sharemindCyptographicRandom,
                       GRND_RANDOM,
                       blockingCryptoRandomFile)
SHAREMIND_LIBRANDOM_BR(sharemindCyptographicURandom,
                       0,
                       blockingCryptoURandomFile)

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
    sharemind::cryptographicURandom(v.data(), ARRAY_SIZE);
    printVector();
    std::cout << "------------------------" << std::endl;
    auto const blockingRandom = [&](
            decltype(sharemind::cryptographicRandomNonblocking) fn)
    {
        ElemType * buf = v.data();
        size_t sizeLeft = ARRAY_SIZE;
        for (;;) {
            size_t const r = fn(buf, sizeLeft);
            assert(r <= sizeLeft);
            sizeLeft -= r;
            if (sizeLeft <= 0u)
                break;
            buf += r;
        }
    };
    blockingRandom(&sharemind::cryptographicRandomNonblocking);
    printVector();
    blockingRandom(&sharemind::cryptographicURandomNonblocking);
    printVector();
}

#endif // SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_TEST
