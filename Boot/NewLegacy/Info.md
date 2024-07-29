### As it stands right now, the bootloader is going to be restructured:

- The first stage (between 7C00h and 7E00h) will remain the same way as it did, loading the
second stage bootloader.

- The second stage (between 7E00h and 10000h) will change - it'll be much more cut down,
and it will only serve to initialize a few important things, and to jump to the third stage
bootloader.

- Finally, the third stage (above 20000h) will initialize almost everything else. (Keep in
mind that the stack is between 10000h and 20000h)

&nbsp;

At the moment, most of the current code for the third-stage bootloader will be transplanted
from the second-stage bootloader in the old Legacy folder.

The purpose of the second-stage bootloader isn't to initialize anything that isn't crucial,
but just to load the third-stage bootloader while maintaining compatibility with different
BIOS functions and filesystems.

&nbsp;

### Additionally:

- The first stage bootloader will be located at Legacy/Bootsector

- The second stage bootloader will be located at Legacy/Init

- The third stage bootloader will be located at Legacy/Core

- Code and data that's shared between multiple stages of the bootloader, like the real-mode
payload and basic header files (like Stdint.h), will be located at Legacy/Shared.
