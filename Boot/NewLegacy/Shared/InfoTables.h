// Copyright (C) 2024 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_INFOTABLES_H
#define SERRA_INFOTABLES_H

  // ...

  typedef struct {

    uint32 Signature; // This is basically a magic number; set it to 65363231h
    uint16 Version; // For now, 0001h

    // TODO - Create a basic outline of what this is supposed to look like
    // This is just data that the 2nd stage bootloader needs to pass onto the 3rd stage,
    // so..

  } __attribute__((packed)) bootloaderInfoTable;

#endif
