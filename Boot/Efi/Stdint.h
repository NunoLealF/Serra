// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [64-bit Stdint.h - do not use with 16- or 32-bit code.]
// [Built for x64 (AMD64); compatible with both LP64 and LLP64]

#if !defined(__amd64__) && !defined(__x86_64__)
  #error "This code must be compiled with an x86_64-w64-mingw32 cross-compiler"
#endif

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // Many compilers have support for __(U)INT(SIZE)_TYPE__ macros, and if
  // those are defined, we can use those.

  // In the off case that these macros aren't defined (which can happen as
  // they aren't part of the C standard), we use the following:

  // -> char for 8-bit types
  // -> short for 16-bit types
  // -> int for 32-bit types
  // -> long long for 64-bit (and pointer) types

  // Additionally, we also define (u)int* ~ (u)int*_t ~ (U)INT*, so, for
  // example, uint32 = uint32_t = UINT32.

  // [Signed integer types (int8 to int64)]

  #ifdef __INT8_TYPE__
    typedef __INT8_TYPE__ int8;
  #else
    typedef signed char int8;
  #endif

  typedef int8 int8_t;
  typedef int8 INT8;

  #ifdef __INT16_TYPE__
    typedef __INT16_TYPE__ int16;
  #else
    typedef signed short int16;
  #endif

  typedef int16 int16_t;
  typedef int16 INT16;

  #ifdef __INT32_TYPE__
    typedef __INT32_TYPE__ int32;
  #else
    typedef signed int int32;
  #endif

  typedef int32 int32_t;
  typedef int32 INT32;

  #ifdef __INT64_TYPE__
    typedef __INT64_TYPE__ int64;
  #else
    typedef signed long long int64;
  #endif

  typedef int64 int64_t;
  typedef int64 INT64;

  #ifdef __INTPTR__TYPE__
    typedef __INTPTR_TYPE__ intptr;
  #else
    typedef int64 intptr;
  #endif

  typedef intptr intptr_t;
  typedef intptr INTPTR;

  // [Unsigned integer types (uint8 to uint64)]

  #ifdef __UINT8_TYPE__
    typedef __UINT8_TYPE__ uint8;
  #else
    typedef unsigned char uint8;
  #endif

  typedef uint8 uint8_t;
  typedef uint8 UINT8;

  #ifdef __UINT16_TYPE__
    typedef __UINT16_TYPE__ uint16;
  #else
    typedef unsigned short uint16;
  #endif

  typedef uint16 uint16_t;
  typedef uint16 UINT16;

  #ifdef __UINT32_TYPE__
    typedef __UINT32_TYPE__ uint32;
  #else
    typedef unsigned int uint32;
  #endif

  typedef uint32 uint32_t;
  typedef uint32 UINT32;

  #ifdef __UINT64_TYPE__
    typedef __UINT64_TYPE__ uint64;
  #else
    typedef unsigned long long uint64;
  #endif

  typedef uint64 uint64_t;
  typedef uint64 UINT64;

  // [Character and pointer types]

  #ifdef __UINTPTR__TYPE__
    typedef __UINTPTR_TYPE__ uintptr;
  #else
    typedef uint64 uintptr;
  #endif

  typedef uintptr uintptr_t;
  typedef uintptr UINTPTR;

  // [Character types]

  typedef uint8 char8;
  typedef char8 char8_t;
  typedef char8 CHAR8;

  typedef uint16 char16;
  typedef char16 char16_t;
  typedef char16 CHAR16;

  // [Compile-time checks, and static assertions]

  #ifdef __has_attribute

    #if !(__has_attribute(packed))
      #error "Your compiler must support __attribute__((packed))"
    #endif

  #else

    #error "Your compiler must support GCC-style attributes (__attribute__(...))"

  #endif

  static_assert((sizeof(bool) == 1), "`bool` is not 8 bits.");

  static_assert((sizeof(int8) == 1), "`int8` has incorrect size.");
  static_assert((sizeof(int16) == 2), "`int16` has incorrect size.");
  static_assert((sizeof(int32) == 4), "`int32` has incorrect size.");
  static_assert((sizeof(int64) == 8), "`int64` has incorrect size.");

  static_assert((sizeof(uint8) == 1), "`uint8` has incorrect size.");
  static_assert((sizeof(uint16) == 2), "`uint16` has incorrect size.");
  static_assert((sizeof(uint32) == 4), "`uint32` has incorrect size.");
  static_assert((sizeof(uint64) == 8), "`uint64` has incorrect size.");

  static_assert((sizeof(intptr) == 8), "`intptr` has incorrect size - are you on a 64-bit platform?");
  static_assert((sizeof(uintptr) == 8), "`uintptr` has incorrect size - are you on a 64-bit platform?");

  // [Other integer/pointer types]

  #define intmax 0x7FFFFFFFFFFFFFFF
  #define INTMAX intmax

  #define uintmax 0xFFFFFFFFFFFFFFFF
  #define UINTMAX uintmax

  typedef struct _genericUuid {

    uint32 Uuid_A;
    uint16 Uuid_B[2];
    uint8 Uuid_C[8];

  } __attribute__((packed)) genericUuid;

  // [Null types]

  #define null 0
  #define NULL null

  // [Macros/definitions for variadic functions]

  // (We just use <stdarg.h> because this is difficult to implement, and
  // our compiler has it)

  #include <stdarg.h>

#endif
