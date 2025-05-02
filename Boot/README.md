## Boot/

> [!WARNING]
> *This section is **still** a work in progress, so please don't expect this to be functional or complete*

Despite Serra being a boot manager, it's structured more like an operating
system, with a *bootloader* (located in `Boot/`) and a *kernel*
(located in `Common/`).

This section contains the bootloader, and its job is to prepare the kernel
environment, before eventually transferring control to the kernel itself.

The BIOS bootloader is located in `Boot/Legacy` (or `/Boot/Bootx32.bin` +
the first reserved sectors on a FAT disk), and the EFI bootloader is located
in `Boot/Efi` (or `/Efi/Boot/Bootx64.efi` in the image itself).

<details>

  <summary>Legacy/BIOS boot process</summary>

  - **First stage (`Boot/Legacy/Bootsector`):** Located within the first
  sector of the disk/partition (as the Volume Boot Record), and loads the
  next stage of the bootloader, as well as the real mode wrapper;
  - - Loaded to 7C00h\~7E00h *by the firmware*;
  - - CPU is in 16-bit real mode (with access to BIOS interrupts).

  - **Second stage (`Boot/Legacy/Init`, S2Init() ~ S2Bootloader()) and
  real mode wrapper (`Boot/Legacy/Shared/Rm`):** Located within the 16-40th
  sectors of the disk/partition, and loads the next stage of the bootloader
  from the FAT file system, as well as preparing a few important things;
  - - Loaded to 7E00h\~9E00h (wrapper between 9E00\~AE00h) *by the bootsector (using int 13h)*;
  - - CPU is in 32-bit protected mode (with access to BIOS interrupts *via*
  the 16-bit real mode wrapper, which temporarily switches back).

  - **Third stage (`Boot/Legacy/Core`, S3Bootloader()):** Located in the
  `/Boot/Bootx32.bin` file on a FAT partition, and prepares the kernel
  environment - enabling the A20 line, querying the system memory map, setting
  up identity paging, etc. - before loading the kernel.
  - - Loaded to 20000h *by the second stage (using int 13h)*;
  - - CPU is in 32-bit protected mode (with access to BIOS interrupts via
  the same mechanism for earlier), until the switch to 64-bit long mode;
  - - Transfers control to the kernel via LongmodeStub (`Stub.asm`), after
  filling out the kernel info table, and enabling paging/VESA modes/etc.

</details>

<details>

  <summary>EFI boot process</summary>

  - **EFI application (`Boot/Efi/`):** Located in the
  `/Efi/Boot/Bootx64.efi` file on an EFI System Partition, and prepares
  the kernel environment;
  - - Loaded to any memory location *by the firmware*, so must be
  position-independent (compiled with `-fpie`, linked with `-static-pie`);
  - - CPU is in 64-bit long mode (without access to BIOS interrupts, but
  *with access to EFI functions*);
  - - Transfers control to the kernel via **TODO (still not here yet)**,
  after filling out the kernel info table, and enabling **TODO (features)**.

  - **EFI subsystem/headers (`Boot/Efi/Efi`):** This is linked together with
  the EFI application, and it provides a minimal interface with EFI tables,
  functions and protocols.
  - - This is *not* a complete implementation of the UEFI Specification, it
  just provides a way to access some necessary functions from the EFI
  application itself.

</details>

For more information, refer to the [roadmap](#roadmap) below:

&nbsp;


### Roadmap

- Finish implementing EFI support, in `Boot/Efi`;
- - Process the system memory map;
- - Locate protocols for things like ACPI, USB, SATA, disk, etc.;
- Review the build system (and add a better way to control things like debug messages);
- - A configuration file would be ideal for this.

&nbsp;


## Disclaimer

This project has been released under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the accompanying license agreement. <3

*(last updated on May 2nd 2025)*
