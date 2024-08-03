## Some important information

- The first stage bootloader is located at Legacy/Bootsector

- The second stage bootloader is located at Legacy/Init

- The third stage bootloader is located at Legacy/Core

- Code and data that's shared between multiple stages of the bootloader, like the real-mode
payload and basic header files (like Stdint.h), will be located at Legacy/Shared

.
