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

It requires an x64 environment, with identity paging set up, as well as a
bootloader-provided *info table* (see `InfoTable.h`) that contains some
information about the system.

&nbsp;

### Roadmap

- Start working on the kernel, in `Common/`.
- - Review the kernel info tables;
- - Implement a few common drivers, for things like graphics support, PCI, etc.;
- - Add filesystem support (something involving a VFS maybe?);
- Add the actual boot manager functionality (support for boot protocols);
- Create a simple user interface (GUI *and* text-mode as fallback)

&nbsp;

## Disclaimer

This project has been released under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the accompanying license agreement. <3

*(last updated on April 28th 2025)*
