/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "NullRandomEngine.h"

#include <cstring>


namespace sharemind {

void NullRandomEngine::fillBytes(void * memptr, size_t size) noexcept {
    memset(memptr, 0, size);
}

} // namespace sharemind
