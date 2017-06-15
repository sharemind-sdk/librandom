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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(SharemindLibRandom CXX)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(SHAREMIND_LIBRANDOM_VERSION
    "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)
INCLUDE(CheckCXXSourceCompiles)

FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(SharemindCHeaders REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders REQUIRED)

# Check if we have SSE2 support via emmintrin.h header
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

SET(SHAREMIND_LIBRANDOM_EXTERNAL_LINK_LIBRARIES "")
LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_LINK_LIBRARIES
    ${OPENSSL_LIBRARIES}
    ${SharemindCHeaders_LIBRARIES}
    ${SharemindCxxHeaders_LIBRARIES}
)
LIST(REMOVE_DUPLICATES SHAREMIND_LIBRANDOM_EXTERNAL_LINK_LIBRARIES)

SET(SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS "")
LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS
    "${OPENSSL_INCLUDE_DIR}"
    ${SharemindCHeaders_INCLUDE_DIRS}
    ${SharemindCxxHeaders_INCLUDE_DIRS}
)
IF(VALGRIND_INCLUDE_DIR)
    LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS
         "${VALGRIND_INCLUDE_DIR}")
ENDIF()
LIST(REMOVE_DUPLICATES SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS)

SET(SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS "")
LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS
    ${SharemindCHeaders_DEFINITIONS}
    ${SharemindCxxHeaders_DEFINITIONS}
)
IF(VALGRIND_INCLUDE_DIR)
    LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS
         "-DSHAREMIND_LIBRANDOM_HAVE_VALGRIND")
ENDIF()
IF(HAVE_EMMINTRIN_SSE2)
    LIST(APPEND SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS
         "-DSHAREMIND_HAVE_EMMINTRIN_SSE2")
ENDIF()
LIST(REMOVE_DUPLICATES SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS)


SET(SHAREMIND_LIBRANDOM_INSTALL_LINK_LIBRARIES
    ${SHAREMIND_LIBRANDOM_EXTERNAL_LINK_LIBRARIES}
    "-L${CMAKE_INSTALL_PREFIX}/lib" "-lsharemind_random")
SET(SHAREMIND_LIBRANDOM_INSTALL_INCLUDE_DIRS
    "${CMAKE_INSTALL_PREFIX}/include"
    ${SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS})
SET(SHAREMIND_LIBRANDOM_INSTALL_DEFINITIONS
    ${SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS})
LIST(REMOVE_DUPLICATES SHAREMIND_LIBRANDOM_INSTALL_INCLUDE_DIRS)

INCLUDE_DIRECTORIES(${SHAREMIND_LIBRANDOM_EXTERNAL_INCLUDE_DIRS})
ADD_DEFINITIONS(${SHAREMIND_LIBRANDOM_EXTERNAL_DEFINITIONS})
ADD_COMPILE_OPTIONS(
  "-std=c++11" "-Wall" "-Wextra" "-O2" "-fPIC"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-ggdb>"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-fno-omit-frame-pointer>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-DNDEBUG>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-fomit-frame-pointer>"
)

FILE(GLOB_RECURSE SHAREMIND_LIBRANDOM_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
FILE(GLOB_RECURSE SHAREMIND_LIBRANDOM_HEADERS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
ADD_LIBRARY("librandom" SHARED
    ${SHAREMIND_LIBRANDOM_SOURCES}
    ${SHAREMIND_LIBRANDOM_HEADERS})
SET_TARGET_PROPERTIES("librandom" PROPERTIES
                      OUTPUT_NAME "sharemind_random"
                      VERSION "${SHAREMIND_LIBRANDOM_VERSION}"
                      SOVERSION "0.2")
TARGET_LINK_LIBRARIES("librandom"
                      ${SHAREMIND_LIBRANDOM_EXTERNAL_LINK_LIBRARIES})
INSTALL(TARGETS "librandom" LIBRARY DESTINATION "lib" COMPONENT lib)

INSTALL(FILES ${SHAREMIND_LIBRANDOM_HEADERS}
        DESTINATION "include/sharemind/Random"
        COMPONENT dev)

FILE(GLOB_RECURSE SHAREMIND_LIBRANDOM_INFILES
     "${CMAKE_CURRENT_SOURCE_DIR}/*.in")
ADD_CUSTOM_TARGET("include_SHAREMIND_LIBRANDOM_INFILES_in_IDE"
                  SOURCES ${SHAREMIND_LIBRANDOM_INFILES})

# Tests:
ENABLE_TESTING()
ADD_CUSTOM_TARGET(check COMMAND "${CMAKE_CTEST_COMMAND}")
FILE(GLOB_RECURSE SHAREMIND_LIBRANDOM_TESTS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/Test*.cpp")
ADD_CUSTOM_TARGET("include_SHAREMIND_LIBRANDOM_TESTS_in_IDE"
    SOURCES ${SHAREMIND_LIBRANDOM_TESTS})
FOREACH(test IN LISTS SHAREMIND_LIBRANDOM_TESTS)
    GET_FILENAME_COMPONENT(testName "${test}" NAME_WE)
    ADD_EXECUTABLE("testImpl_${testName}" EXCLUDE_FROM_ALL "${test}")
    TARGET_LINK_LIBRARIES("testImpl_${testName}" librandom)
    ADD_DEPENDENCIES(check "testImpl_${testName}")
    ADD_TEST(NAME "test_${testName}" COMMAND "testImpl_${testName}")
ENDFOREACH()

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/SharemindLibRandomConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SharemindLibRandomConfig.cmake"
  @ONLY)
INSTALL(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/SharemindLibRandomConfig.cmake"
  DESTINATION "lib/SharemindLibRandom"
  COMPONENT dev)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/SharemindLibRandomConfigVersion.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SharemindLibRandomConfigVersion.cmake"
  @ONLY)
INSTALL(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/SharemindLibRandomConfigVersion.cmake"
  DESTINATION "lib/SharemindLibRandom"
  COMPONENT dev)

# Packaging:
SET(CPACK_PACKAGE_VENDOR "Cybernetica AS")
SET(CPACK_PACKAGE_CONTACT "Sharemind packaging <sharemind-packaging@cyber.ee>")
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME "libsharemind-random")
SET(CPACK_DEBIAN_PACKAGE_RELEASE "cyber1")
SET(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
SET(CPACK_DEB_COMPONENT_INSTALL "ON")
SET(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

SET(CPACK_DEBIAN_LIB_PACKAGE_NAME "libsharemind-random")
SET(CPACK_COMPONENT_DEV_DESCRIPTION "Sharemind Random library development headers")
SET(CPACK_COMPONENT_LIB_DESCRIPTION "Sharemind Random library")
SET(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS
    "libsharemind-random (= ${SHAREMIND_LIBRANDOM_VERSION}), sharemind-cxxheaders (>= 0.1.0), sharemind-cheaders (>= 0.1.0), libc6-dev (>= 2.19), libstdc++-dev, linux-libc-dev, libssl-dev, libgcc-4.8-dev | libgcc-4.9-dev | libgcc-5-dev | libgcc-6-dev | libgcc-7-dev")
SET(CPACK_DEBIAN_LIB_PACKAGE_DEPENDS
    "libstdc++6 (>= 4.8.0), libc6 (>= 2.19), libssl1.0.0")
SET(CPACK_DEBIAN_DEV_PACKAGE_SECTION "libdevel")
SET(CPACK_DEBIAN_LIB_PACKAGE_SECTION "libs")
SET(CPACK_DEBIAN_DEV_PACKAGE_ARCHITECTURE "any")
INCLUDE(CPack)
