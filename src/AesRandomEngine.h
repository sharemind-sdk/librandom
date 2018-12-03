/*
 * Copyright (C) Cybernetica
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

#ifndef SHAREMIND_LIBRANDOM_AESRANDOMENGINE_H
#define SHAREMIND_LIBRANDOM_AESRANDOMENGINE_H

#include "RandomEngine.h"


namespace sharemind {

/** A random engine based on AES in CTR mode. */
class AesRandomEngine: public RandomEngine {

public: /* Methods: */

    AesRandomEngine(const void * seed);

    ~AesRandomEngine() noexcept override;

    void fillBytes(void * buffer, size_t size) noexcept override;

    static bool supported() noexcept;

    static size_t seedSize() noexcept;

private: /* Fields: */

    void * m_inner;

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_AESRANDOMENGINE_H */
