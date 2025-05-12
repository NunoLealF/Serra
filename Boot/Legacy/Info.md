## Boot/Legacy/ layout

- The first stage bootloader, as well as a reference MBR implementation,
is located at Legacy/Bootsector;

- The second stage bootloader is located at Legacy/Stage2;

- The third stage bootloader is located at Legacy/Stage3;

- Code and data that's shared between multiple stages of the bootloader,
like the real-mode wrapper and basic header files (like Stdint.h), is
located at Legacy/Shared.
