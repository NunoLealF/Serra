// Copyright (C) 2023 NunoLealF
// This file is part of the Serra project, which is released under the MIT license.
// For more information, please refer to the accompanying license agreement. <3

#ifndef SERRA_RM_H_
#define SERRA_RM_H

  // Real mode functions

  void realMode(void);
  void loadRegisters(uint32 Eax, uint32 Ebx, uint32 Ecx, uint32 Edx, uint16 Si, uint16 Di, uint16 Bp);


#endif
