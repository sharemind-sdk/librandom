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

#include "NullRandomEngine.h"

#include "RandomEngine.h"

#include <cstring>

namespace /* anonymous */ {

extern "C"
void NullRandomEngine_fill_bytes(SharemindRandomEngine*, void* memptr, size_t size) {
    bzero(memptr, size);
}

extern "C"
void NullRandomEngine_free(SharemindRandomEngine*) { }

static SharemindRandomEngine null_random_engine = SharemindRandomEngine {
        NullRandomEngine_fill_bytes,
        NullRandomEngine_free
};

} // namespace anonymous

namespace sharemind {

SharemindRandomEngine* make_null_random_engine() noexcept {
    return &null_random_engine;
}

} // namespace sharemind
