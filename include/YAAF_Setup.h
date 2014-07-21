/*
 * YAAF - Yet Another Archive Format
 * Copyright (C) 2014, Leander Beernaert
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You can contact the author at :
 * - YAAF source repository : http://www.github.com/LeanderBB/YAAF
 */
#ifndef __YAAF_SETUP_H__
#define __YAAF_SETUP_H__

#include "YAAF_Config.h"

/* --- Version ------------------------------------------------------------- */

#define YAAF_VERSION_MAJOR 0
#define YAAF_VERSION_MINOR 5
#define YAAF_VERSION_PATCH 0

#define YAAF_VERSION (YAAF_VERSION_MAJOR * 1000) + (YAAF_VERSION_MINOR * 10) + \
  YAAF_VERSION_PATCH


/* --- Host OS ------------------------------------------------------------- */

#if defined(__ANDROID__) || defined(ANDROID)
  #define YAAF_OS_UNIX
  #define YAAF_OS_ANDROID
  #define YAAF_TARGET_ANDROID
#elif defined(__linux) || defined(__linux__)
  #define YAAF_OS_UNIX
  #define YAAF_OS_LINUX
  #define YAAF_TARGET_LINUX
#elif defined(__APPLE__)
  #define YAAF_OS_UNIX
  #define YAAF_OS_APPLE
  #include <AvailabilityMacros.h>
  #include <TargetConditionals.h>
  #if defined(__IPHONE_OS__)
    #define YAAF_TARGET_IOS
    #if defined(TARGET_OS_IPHONE)
      #define YAAF_IOS_DEVICE
    #else
      #define YAAF_IOS_SIMULATOR
    #endif
  #else
    #define YAAF_TARGET_MAC
  #endif
#elif defined(WIN32) || defined(_WIN32)
  #define YAAF_OS_WIN
  #define YAAF_TARGET_WIN
#else
  #error Unknown Operating System
#endif

/* --- Compiler ------------------------------------------------------------ */

#if defined(__clang__)
  #define YAAF_COMPILER_CLANG
  #if defined(YAAF_OS_APPLE)
    #define YAAF_COMPILER_NAME "Apple Clang/LLVM"
  #else
    #define YAAF_COMPILER_NAME "Clang/LLVM"
  #endif
  #define YAAF_COMPILER_VERSION (((__clang_major__)*100) + \
(__clang_minor__*10))
  #define YAAF_COMPILER_VERSION_MAJOR __clang_major__
  #define YAAF_COMPILER_VERSION_MINOR __clang_minor__
#elif defined(__GNUC__)
  #define YAAF_COMPILER_GNUC
  #define YAAF_COMPILER_NAME "GNUC"
  #define YAAF_COMPILER_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
  #define YAAF_COMPILER_VERSION_MAJOR __GNUC__
  #define YAAF_COMPILER_VERSION_MINOR __GNUC_MINOR__
#elif defined(_MSC_VER)
  #define YAAF_COMPILER_MSC
  #define YAAF_COMPILER_NAME "MSC"
  #define YAAF_COMPILER_VERSION _MSC_VER
  #define YAAF_COMPILER_VERSION_MAJOR (_MSC_VER / 100)
  #define YAAF_COMPILER_VERSION_MINOR (_MSC_VER % 100)
#else
  #error "Unknown compiler detected!"
#endif

/* Check if compiler has C99 standard active */

#if (__STDC_VERSION__ >= 199901L)
#define YAAF_COMPILER_C99
#endif

/* --- Calling Conventions-------------------------------------------------- */

#if defined(YAAF_COMPILER_CLANG) || defined(YAAF_COMPILER_GNUC)
  #define YAAF_CALL
  #define YAAF_EXPORT __attribute__ ((visibility("default")))
  #define YAAF_INLINE __inline__
  #define YAAF_FORCE_INLINE __attribute__((always_inline)) static __inline__
#elif defined(YAAF_COMPILER_MSC)
  #define YAAF_CALL __cdecl
  #define YAAF_EXPORT __declspec(dllexport)
  #define YAAF_INLINE __inline
  #define YAAF_FORCE_INLINE __forceinline
#else
  #error Missing calling convetions for current compiler
#endif

/* --- CPU ARCH ------------------------------------------------------------ */

#if !defined(YAAF_CPU_BE)
#define YAAF_CPU_LE
#endif

#if defined(YAAF_CPU_BE)
#define YAAF_REQUIRE_BSWAP
#endif

/* use builtin swapping functions when possilbe */
#if __GNUC__ >=4 && __GNUC_MINOR__ >= 1
  #define YAAF_BSWAP16 __builtin_bswap16
  #define YAAF_BSWAP32 __builtin_bswap32
  #define YAAF_BSWAP64 __builtin_bswap64
#elif defined(_MSC_VER)
  #define YAAF_BSWAP16 _byteswap_ushort
  #define YAAF_BSWAP32 _byteswap_ulong
  #define YAAF_BSWAP64 _byteswap_uint64
#else
static YAAF_INLINE YAAF_BSWAP16(uint16_t x)
{
  return (( v & 0x00ff) << 8) | ((v & 0xff00) >> 8);
}

static YAAF_INLINE YAAF_BSWAP32(uint32_t x)
{
  return ((x << 24) & 0xff000000 ) |
      ((x <<  8) & 0x00ff0000 ) |
      ((x >>  8) & 0x0000ff00 ) |
      ((x >> 24) & 0x000000ff );
}

static YAAF_INLINE YAAF_BSWAP64(uint64_t v)
{
  return (((v & 0xff00000000000000ull) >> 56) |
          ((v & 0x00ff000000000000ull) >> 40) |
          ((v & 0x0000ff0000000000ull) >> 24) |
          ((v & 0x000000ff00000000ull) >> 8) |
          ((v & 0x00000000ff000000ull) << 8) |
          ((v & 0x0000000000ff0000ull) << 24) |
          ((v & 0x000000000000ff00ull) << 40) |
          ((v & 0x00000000000000ffull) << 56));
}
#endif

#if defined(YAAF_REQUIRE_BSWAP)
#define YAAF_LITTLE_E16(x) YAAF_BSWAP16(x)
#define YAAF_LITTLE_E32(x) YAAF_BSWAP32(x)
#define YAAF_LITTLE_E64(x) YAAF_BSWAP64(x)
#else
#define YAAF_LITTLE_E16(x) x
#define YAAF_LITTLE_E32(x) x
#define YAAF_LITTLE_E64(x) x
#endif

#if defined(__x86_64) || defined(__amd64) || defined (__LP64__) || defined(_WIN64)
#define YAAF_CPU_X86
#define YAAF_CPU_64BIT
#elif defined(__i386__) || (__intel__) || defined(_WIN32)
#define YAAF_CPU_X86
#define YAAF_CPU_32BIT
#else
#error "Unknown Processor."
#endif

/* --- Debug --------------------------------------------------------------- */

#if defined(YAAF_DEBUG)
#define YAAF_ENABLE_ASSERT
#endif

#if defined(YAAF_ENABLE_ASSERT)
#include <assert.h>
#define YAAF_ASSERT(x) assert(x)
#else
#define YAAF_ASSERT(x)
#endif

/* --- Includes ------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>
#if defined(YAAF_HAVE_MALLOC_H)
#include <malloc.h>
#endif
#include <string.h>
#include <limits.h>
#if defined(YAAF_HAVE_STRINGS_H)
#include <strings.h>
#endif

#if defined(YAAF_COMPILER_C99) || defined(YAAF_HAVE_INTTYPES_H)
#include <inttypes.h>
#else
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

/* --- Other Defines-------------------------------------------------------- */

#if defined(YAAF_CPU_64BIT)
typedef int64_t YAAF_signed_size_t;
#else
typedef int32_t YAAF_signed_size_t;
#endif

#endif /* __YAAF_SETUP_H__ */
