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

#ifndef SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_H
#define SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_H

#include <sharemind/extern_c.h>

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */


SHAREMIND_EXTERN_C_BEGIN

size_t sharemindCryptographicRandomNonblocking(void * buf,
                                               size_t bufSize) noexcept;
size_t sharemindCryptographicURandomNonblocking(void * buf,
                                                size_t bufSize) noexcept;

void sharemindCyptographicRandom(void * buf, size_t bufSize) noexcept;
void sharemindCyptographicURandom(void * buf, size_t bufSize) noexcept;

SHAREMIND_EXTERN_C_END


#ifdef __cplusplus
namespace sharemind {

inline size_t cryptographicRandomNonblocking(void * buf,
                                             size_t bufSize) noexcept
{ return ::sharemindCryptographicRandomNonblocking(buf, bufSize); }
inline size_t cryptographicURandomNonblocking(void * buf,
                                              size_t bufSize) noexcept
{ return ::sharemindCryptographicURandomNonblocking(buf, bufSize); }

inline void cryptographicRandom(void * buf, size_t bufSize) noexcept
{ return ::sharemindCyptographicRandom(buf, bufSize); }
inline void cryptographicURandom(void * buf, size_t bufSize) noexcept
{ return ::sharemindCyptographicURandom(buf, bufSize); }

} /* namespace sharemind { */
#endif /* __cplusplus */

#endif /* SHAREMIND_LIBRANDOM_CRYPTOGRAPHICRANDOM_H */
