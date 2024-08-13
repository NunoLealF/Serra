// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_STDINT_H
#define SERRA_STDINT_H

  // Many compilers have support for __(U)INT(SIZE)_TYPE__ macros, and if those
  // are defined, we can use those.

  // In the off case that these macros aren't defined (which can happen as they
  // aren't part of the C standard), we use the following:
  // - char for 8-bit types
  // - short for 16-bit types
  // - int for 32-bit types
  // - long long for 64-bit types

  // Signed integer types (int8 to int64).

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

  // Unsigned integer types (uint8 to uint64).

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

  // Other integer/pointer types.

  #define intmax  0x0FFFFFFF
  #define uintmax 0xFFFFFFFF

  typedef struct {

    uint16 Segment;
    uint16 Offset;

  } __attribute__((packed)) farPtr;

  #define convertFarPtr(Address) ((Ptr.Segment << 4) + Ptr.Offset)

  // Non-integer types.

  #define false 0
  #define true !(false)

  typedef uint8 bool;

  // Null type

  #define null 0
  #define NULL null

  // Macros/definitions for variadic functions.
  // (We just use <stdarg.h> because this is difficult to implement, and our compiler has it)

  #include <stdarg.h>

#endif
