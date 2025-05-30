// Copyright (C) 2025 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

// [32-bit Stdint.h - do not use with 16- or 64-bit code.]
// [Built for i686 (x86, Pentium Pro+); compatible with ILP32]

#ifndef __i686__
  #error "This code must be compiled with an i686-elf cross-compiler"
#endif

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // Many compilers have support for __(U)INT(SIZE)_TYPE__ macros, and if those
  // are defined, we can use those.

  // In the off case that these macros aren't defined (which can happen as they
  // aren't part of the C standard), we use the following:
  // - char for 8-bit types
  // - short for 16-bit types
  // - int for 32-bit (and pointer) types
  // - long long for 64-bit types

  // [Signed integer types (int8 to int64)]

  #ifdef __INT8_TYPE__
    typedef __INT8_TYPE__ int8;
  #else
    typedef signed char int8;
  #endif

  #ifdef __INT16_TYPE__
    typedef __INT16_TYPE__ int16;
  #else
    typedef signed short int16;
  #endif

  #ifdef __INT32_TYPE__
    typedef __INT32_TYPE__ int32;
  #else
    typedef signed int int32;
  #endif

  #ifdef __INT64_TYPE__
    typedef __INT64_TYPE__ int64;
  #else
    typedef signed long long int64;
  #endif

  #ifdef __INTPTR__TYPE__
    typedef __INTPTR_TYPE__ intptr;
  #else
    typedef int32 intptr;
  #endif

  // [Unsigned integer types (uint8 to uint64)]

  #ifdef __UINT8_TYPE__
    typedef __UINT8_TYPE__ uint8;
  #else
    typedef unsigned char uint8;
  #endif

  #ifdef __UINT16_TYPE__
    typedef __UINT16_TYPE__ uint16;
  #else
    typedef unsigned short uint16;
  #endif

  #ifdef __UINT32_TYPE__
    typedef __UINT32_TYPE__ uint32;
  #else
    typedef unsigned int uint32;
  #endif

  #ifdef __UINT64_TYPE__
    typedef __UINT64_TYPE__ uint64;
  #else
    typedef unsigned long long uint64;
  #endif

  #ifdef __UINTPTR__TYPE__
    typedef __UINTPTR_TYPE__ uintptr;
  #else
    typedef uint32 uintptr;
  #endif

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

  static_assert((sizeof(intptr) == 4), "`intptr` has incorrect size - are you on a 32-bit platform?");
  static_assert((sizeof(uintptr) == 4), "`uintptr` has incorrect size - are you on a 32-bit platform?");

  // [Other integer/pointer types]

  #define intmax 0x7FFFFFFF
  #define uintmax 0xFFFFFFFF

  typedef struct {

    uint16 Offset;
    uint16 Segment;

  } __attribute__((packed)) farPtr;

  #define convertFarPtr(Ptr) ((Ptr.Segment << 4) + Ptr.Offset)

  // [Null types]

  #define null 0
  #define NULL null

  // [Macros/definitions for variadic functions]
  // (We just use <stdarg.h> because this is difficult to implement, and our compiler has it)

  #include <stdarg.h>

#endif
