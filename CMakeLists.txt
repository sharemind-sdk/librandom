#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(SharemindLibRandom VERSION 0.3.0 LANGUAGES CXX)

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCMakeHelpers REQUIRED)

SharemindSetProjectVersion()

FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(SharemindCHeaders REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)

# Check if we have SSE2 support via emmintrin.h header
INCLUDE(CheckCXXSourceCompiles)
Check_CXX_Source_Compiles(
"
#include <emmintrin.h>
#if !defined(__SSE__)
#error
#endif
#if !defined(__SSE2__)
#error
#endif
int main () {
    __m128i x, y, z;
    x = _mm_set_epi32(0, 1, 2, 3);
    y = z = _mm_set1_epi32(0);
    x = _mm_or_si128(y, z);
    x = _mm_slli_epi32(x, 1);
    x = _mm_srli_epi32(x, 1);
    x = _mm_add_epi32(y, z);
    x = _mm_xor_si128(y, z);
    return 0;
}
"
HAVE_EMMINTRIN_SSE2)

IF(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Release"))
    FIND_PATH(VALGRIND_INCLUDE_DIR "valgrind/memcheck.h"
              PATHS "/usr/include/valgrind" "/usr/local/include/valgrind")
    IF(VALGRIND_INCLUDE_DIR)
        MESSAGE(STATUS "Found valgrind/memcheck.h in ${VALGRIND_INCLUDE_DIR}")
    ELSE()
        MESSAGE(STATUS "valgrind/memcheck.h not found!")
    ENDIF()
ENDIF()

SharemindNewUniqueList(SharemindLibRandom_LINK_LIBRARIES
    ${OPENSSL_LIBRARIES}
    ${SharemindCHeaders_LIBRARIES}
    ${SharemindCxxHeaders_LIBRARIES}
)
SharemindNewUniqueList(SharemindLibRandom_INCLUDE_DIRS
    "${OPENSSL_INCLUDE_DIR}"
    ${SharemindCHeaders_INCLUDE_DIRS}
    ${SharemindCxxHeaders_INCLUDE_DIRS}
)
IF(VALGRIND_INCLUDE_DIR)
    SharemindListAppendUnique(SharemindLibRandom_INCLUDE_DIRS
        "${VALGRIND_INCLUDE_DIR}"
    )
ENDIF()

SharemindNewUniqueList(SharemindLibRandom_DEFINITIONS
    ${SharemindCHeaders_DEFINITIONS}
    ${SharemindCxxHeaders_DEFINITIONS}
)
IF(VALGRIND_INCLUDE_DIR)
    SharemindListAppendUnique(SharemindLibRandom_DEFINITIONS
        "-DSHAREMIND_LIBRANDOM_HAVE_VALGRIND"
    )
ENDIF()
IF(HAVE_EMMINTRIN_SSE2)
    SharemindListAppendUnique(SharemindLibRandom_DEFINITIONS
        "-DSHAREMIND_HAVE_EMMINTRIN_SSE2"
    )
ENDIF()


# Headers:
FILE(GLOB_RECURSE SharemindLibRandom_HEADERS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
INSTALL(FILES ${SharemindLibRandom_HEADERS}
        DESTINATION "include/sharemind/Random"
        COMPONENT "dev")

# The library:
SharemindSetCxx11CompileOptions()

FILE(GLOB_RECURSE SharemindLibRandom_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
SharemindAddSharedLibrary("librandom"
    OUTPUT_NAME "sharemind_random"
    SOURCES
        ${SharemindLibRandom_SOURCES}
        ${SharemindLibRandom_HEADERS}
    INCLUDE_DIRECTORIES
        ${SharemindLibRandom_INCLUDE_DIRS}
    LEGACY_DEFINITIONS
        ${SharemindLibRandom_DEFINITIONS}
    LINK_LIBRARIES
        ${SharemindLibRandom_LINK_LIBRARIES}
)


# Install CMake files:
SharemindCreateCMakeFindFiles(
    INCLUDE_DIRS
        "${CMAKE_INSTALL_PREFIX}/include"
        ${SharemindLibRandom_INCLUDE_DIRS}
    DEFINITIONS
        ${SharemindLibRandom_DEFINITIONS}
    LIBRARIES
        ${SharemindLibRandom_LINK_LIBRARIES}
        "-L${CMAKE_INSTALL_PREFIX}/lib"
        "-lsharemind_random"
)


# Tests:
SharemindAddSimpleTests(
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/Test*.cpp"
    INCLUDE_DIRECTORIES
        ${SharemindLibRandom_INCLUDE_DIRS}
    LEGACY_DEFINITIONS
        ${SharemindLibRandom_INSTALL_DEFINITIONS}
    LINK_LIBRARIES
        ${SharemindLibRandom_LINK_LIBRARIES}
        "librandom"
)


# Packaging:
SharemindSetupPackaging()
SharemindAddComponentPackage("lib"
    NAME "libsharemind-random"
    DESCRIPTION "Sharemind Random library"
    DEB_SECTION "libs"
    DEB_DEPENDS
        "libstdc++6 (>= 4.8.0)"
        "libc6 (>= 2.19)"
        "libssl1.0.0"
        "| libssl1.0.2"
        "| libssl1.1"
)
SharemindAddComponentPackage("dev"
    NAME "libsharemind-random-dev"
    DESCRIPTION "Sharemind Random library development headers"
    DEB_SECTION "libdevel"
    DEB_DEPENDS
        "libsharemind-random (= ${SharemindLibRandom_DEB_lib_PACKAGE_VERSION})"
        "libsharemind-cxxheaders-dev (>= 0.1.0)"
        "libsharemind-cheaders-dev (>= 0.1.0)"
        "libc6-dev (>= 2.19)"
        "libstdc++-dev"
        "linux-libc-dev"
        "libssl-dev"
        "libgcc-4.8-dev"
        "| libgcc-4.9-dev"
        "| libgcc-5-dev"
        "| libgcc-6-dev"
        "| libgcc-7-dev"
)
SharemindAddComponentPackage("debug"
    NAME "libsharemind-random-dbg"
    DESCRIPTION "Sharemind Random library debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS
        "libsharemind-random (= ${SharemindLibRandom_DEB_lib_PACKAGE_VERSION})"
)
SharemindPackagingFinalize()
