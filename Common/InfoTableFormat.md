# commonInfoTable{} / InfoTable.h structure

> [!WARNING]
> *This section is **still** a work in progress, so please don't expect this to be functional or complete*

## [N/A]

Signature `uint64 (equals 7577757E7577757Eh)`

Version `uint16`

Size `uint16`

&nbsp;


## [Disk | Int13(Edd), EfiFs()]

- Disk.AccessMethod `enum:uint16 ("UnknownMethod", "EfiFsMethod", "Int13Method")`

- - Disk->EfiFs.FileInfo `uptr`

- - Disk->EfiFs.HandleList `uptr`

- - Disk->EfiFs.Handle `uptr`

- - Disk->EfiFs.Protocol `uptr`

- - Disk->Int13.DriveNumber `uint8`

- - - Disk->Int13.Edd.IsEnabled `bool`

- - - Disk->Int13.Edd.Table `uptr`

&nbsp;


## [Display : Edid(), Graphics(Bits), Text()]

- Display.Type `enum:uint16 ("UnknownDisplay", "VgaDisplay", "VbeDisplay", "EfiTextDisplay", "GopDisplay")`

- - Display\~Edid.IsSupported `bool`

- - Display\~Edid.PreferredResolution[] `uint16[2]`

- - Display\~Edid.Table `uptr`

- - Display\~Graphics.Framebuffer `uptr`

- - Display\~Graphics.Pitch `uint32 (bytes per scanline)`

- - Display\~Graphics.LimitX `uint16 (horizontal resolution)`

- - Display\~Graphics.LimitY `uint16 (vertical resolution)`

- - - Display\~Graphics\~Bits.PerPixel `uint8`

- - - Display\~Graphics\~Bits.RedMask `uint64`

- - - Display\~Graphics\~Bits.GreenMask `uint64`

- - - Display\~Graphics\~Bits.BlueMask `uint64`

- - Display\~Text.Format `enum:uint8 ("AsciiFormat", "Utf16Format")`

- - Display\~Text.XPos `uint16` *optional for EfiTextDisplay*

- - Display\~Text.YPos `uint16` *optional for EfiTextDisplay*

- - Display\~Text.LimitX `uint16`

- - Display\~Text.LimitY `uint16`

&nbsp;


## [Firmware : Bios(A20,Mmap,Pat,PciBios,Vbe), Efi(Gop,Mmap,Tables)]

- Firmware.Type `enum:uint16 ("UnknownFirmware", "BiosFirmware", "EfiFirmware")`

- - Firmware->Bios.A20 `enum:uint8 ("EnabledByUnknown", "EnabledByDefault", "EnabledByKeyboard", "EnabledByFast")`

- - - Firmware->Bios\~Mmap.NumEntries `uint16`

- - - Firmware->Bios\~Mmap.EntrySize `uint32`

- - - Firmware->Bios\~Mmap.List `uptr` *pointer to a list of e820MmapEntry{}*

- - - Firmware->Bios\~Pat.IsSupported `bool`

- - - Firmware->Bios\~Pat.Value `uint64`

- - - Firmware->Bios\~PciBios.IsSupported `bool`

- - - Firmware->Bios\~PciBios.Table `uptr`

- - - Firmware->Bios\~Vbe.IsSupported `bool`

- - - Firmware->Bios\~Vbe.InfoBlock `uptr`

- - - Firmware->Bios\~Vbe.ModeInfo `uptr`

- - - Firmware->Bios\~Vbe.ModeNumber `uint16`

- - Firmware->Efi.ImageHandle `uptr`

- - Firmware->Efi.SupportsConIn `bool`

- - Firmware->Efi.SupportsConOut `bool`

- - - Firmware->Efi\~Gop.IsSupported `bool`

- - - Firmware->Efi\~Gop.Protocol `uptr`

- - - Firmware->Efi\~Mmap.NumEntries `uint16`

- - - Firmware->Efi\~Mmap.EntrySize `uint64`

- - - Firmware->Efi\~Mmap.List `uptr` *pointer to a list of efiMmapEntry{}*

- - - Firmware->Efi\~Tables.BootServices `uptr`

- - - Firmware->Efi\~Tables.SystemTable `uptr`

- - - Firmware->Efi\~Tables.RuntimeServices `uptr`

&nbsp;


## [Image : Executable()]

- Image.DebugFlag `bool`

- Image.StackTop `uptr` *StackStart + StackSize*

- Image.StackSize `uint64` *in bytes*

- Image.Type `enum:uint8 ("RawImageType", "ElfImageType", "PeImageType")`

- - Image\~Executable.Entrypoint `uptr`

- - Image\~Executable.Header `uptr` *optional for RawImageType*

&nbsp;


## [Memory]

- Memory.NumEntries `uint16`

- Memory.List `uptr` *pointer to a list of usableMmapEntry{}*

- Memory.PreserveUntilOffset `uint64` *tell the kernel to keep everything below this untouched*

&nbsp;


## [System : Acpi(), Cpu(), Smbios()]

- System.Architecture `enum:uint16 ("UnknownArchitecture", "x64Architecture")`

- - System\~Acpi.IsSupported `bool`

- - System\~Acpi.Table `uptr`

- - - System\~Cpu\~x64.ProtectionLevel `uint8`

- - - System\~Cpu\~x64.Cr0 `uint64` *optional if .ProtectionLevel != 0*

- - - System\~Cpu\~x64.Cr3 `uint64` *optional if .ProtectionLevel != 0*

- - - System\~Cpu\~x64.Cr4 `uint64` *optional if .ProtectionLevel != 0*

- - - System\~Cpu\~x64.Efer `uint64` *optional if .ProtectionLevel != 0*

- - - System\~Cpu\~x64.Pml4 `uint64` *optional if .ProtectionLevel != 0*

- - System\~Smbios.IsSupported `bool`

- - System\~Smbios.Table `uptr`

&nbsp;


## [N/A, again]

Checksum `uint16` *sum all bytes in the table, up to Checksum*

&nbsp;


## Disclaimer

This project has been released under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the accompanying license agreement. <3

*(last updated on May 13th 2025)*
