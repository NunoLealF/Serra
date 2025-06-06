## Common/

> [!WARNING]
> *This section is **still** a work in progress, so please don't expect this to be functional or complete*

Despite Serra being a boot manager, it's structured more like an operating
system, with a *bootloader* (located in `Boot/`) and a *kernel*
(located in `Common/`).

This section contains the kernel, and its job is to provide the *functionality*
of a boot manager - reading files, loading operating systems, etc.

It's compiled as a position-independent executable - meaning it can be loaded
pretty much anywhere - and can be booted from both BIOS (`Boot/Legacy`) and
UEFI (`Boot/Efi`) systems.

It requires an x64\* environment, with identity paging set up, as well as a
bootloader-provided *info table* (see `InfoTable.h`) that contains some
information about the system.

<sup>\*The kernel is platform-independent, but only x64 targets are supported right now; however, it shouldn't be too difficult to port it to AArch64 or RISC-V</sup>

&nbsp;

### Roadmap

- Finish implementing the disk subsystem, in `Kernel/Disk`;
- - Implement handlers for the EFI Block and Disk IO protocols (`EFI_BLOCK_IO_PROTOCOL` and `EFI_DISK_IO_PROTOCOL` respectively);
- - Unify both the BIOS and EFI handlers into one unified disk read function;
- - - This should also have the capability to add other methods in the future; keep in mind the disk subsystem is in the last stage, so it can rely on things like ACPI/PCI.
- - Work on filesystem drivers, and create a VFS.

- Add the actual boot manager functionality (support for boot protocols);

- Create simple human interface drivers (keyboard, mouse..);
- - This is pretty simple on UEFI, but not so much on BIOS, sadly.
- - - I *could* make a PS/2 driver, but that requires emulation; you could use ACPI to check for that though.

- Create a simple console user interface / menu.
- - This could be graphical in the future, but the console subsystem is already ready.
- - - (I was thinking - emulated text mode by default, then do it like Linux which lets you use graphics mode in some situations? 2 different menus)

&nbsp;

## Disclaimer

This project has been released under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the accompanying license agreement. <3

*(last updated on June 6th 2025)*
