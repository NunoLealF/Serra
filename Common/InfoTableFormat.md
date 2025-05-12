# Common/InfoTable.h structure

> [!WARNING]
> **(May 12 2025)** This does not reflect the current infotable structure, there *are* missing parts, this is just a basic outline.

## [N/A]

Signature `uint64 (equals 7577757E7577757Eh)`

Version `uint16`

Size `uint16`

&nbsp;


## [Disk | Int13,EfiFs]

- Disk.AccessMethod `enum:uint16 ("UnknownMethod", "EfiFsMethod", "Int13Method")`

- - Disk\~Partition.Bpb `ptr` *optional for EfiFsMethod*

- - Disk\~Partition.BytesPerSector `uint16`

- - Disk\~Partition.LbaOffset `uint64`

- - Disk->Int13.DriveNumber `uint8`

- - - Disk->Int13.Edd.IsEnabled `bool`

- - - Disk->Int13.Edd.Table `ptr`

- - Disk->EfiFs.**???**

&nbsp;


## [Display : Text,Graphics]

- Display.Type `enum:uint16 ("UnknownDisplayType", "VgaDisplayType", "VbeDisplayType", "EfiTextDisplayType", "GopDisplayType")`

- - Display\~Edid.IsSupported `bool`

- - Display\~Edid.Table `ptr`

- - Display\~Graphics.Framebuffer `ptr`

- - Display\~Graphics.Pitch `uint32`

- - Display\~Graphics.LimitX `uint16`

- - Display\~Graphics.LimitY `uint16`

- - - Display\~Graphics\~Bits.PerPixel `uint8`

- - - Display\~Graphics\~Bits.RedMask `uint64`

- - - Display\~Graphics\~Bits.GreenMask `uint64`

- - - Display\~Graphics\~Bits.BlueMask `uint64`

- - - Display\~Graphics\~Bits.ReservedMask `uint64`

- - Display\~Text.Format `enum:uint8 ("AsciiFormat", "Utf8Format", "Utf16Format")`

- - Display\~Text.XPos `uint16` *optional for EfiTextDisplayType*

- - Display\~Text.YPos `uint16` *optional for EfiTextDisplayType*

- - Display\~Text.LimitX `uint16`

- - Display\~Text.LimitY `uint16`

&nbsp;


[Firmware : Bios,Efi]

- Firmware.Type `enum:uint16 ("Unknown_Firmware", "Bios_Firmware", "Efi_Firmware")`

- - Firmware->Bios.**???**

- - - Firmware->Bios\~A20.EnableMethod `enum:uint8 ("A20_ByUnknown", "A20_ByDefault", "A20_ByKeyboard", "A20_ByFast")`

- - - Firmware->Bios\~Mmap.NumEntries `uint16`

- - - Firmware->Bios\~Mmap.EntrySize `uint8`

- - - Firmware->Bios\~Mmap.List `ptr`

- - - Firmware->Bios\~Pat.IsSupported `bool`

- - - Firmware->Bios\~Pat.Value `uint64`

- - - Firmware->Bios\~PciBios.IsSupported `bool`

- - - Firmware->Bios\~PciBios.Table `ptr`

- - - Firmware->Bios\~Vbe.IsSupported `bool`

- - - Firmware->Bios\~Vbe.InfoBlock `ptr`

- - - Firmware->Bios\~Vbe.ModeInfo `ptr`

- - - Firmware->Bios\~Vbe.ModeNumber `uint16`

- - Firmware->Efi.ImageHandle `ptr`

- - Firmware->Efi.Mmap.NumEntries `uint16`

- - Firmware->Efi.Mmap.EntrySize `uint8`

- - Firmware->Efi.Mmap.List `ptr`

- - Firmware->Efi.SupportsConIn `ptr`

- - Firmware->Efi.SupportsConOut `ptr`

- - - Firmware->Efi\~Gop.IsSupported `bool`

- - - Firmware->Efi\~Gop.Mode `uint32`

- - - Firmware->Efi\~Gop.Protocol `ptr`

- - - Firmware->Efi\~Tables.BootServices `ptr`

- - - Firmware->Efi\~Tables.SystemTable `ptr`

- - - Firmware->Efi\~Tables.RuntimeServices `ptr`

&nbsp;


## [Image]

- Image.Stack `ptr`

- Image.Type `enum:uint8 ("Raw_ImageType", "Elf_ImageType")`

- - Image\~Executable.Header `ptr`

- - Image\~Executable.Entrypoint `ptr`

&nbsp;


## [Memory]

- Memory.NumEntries `uint16`

- Memory.Table `ptr`

&nbsp;


## [System : Acpi,Cpu,Smbios]

- System.Architecture `enum:uint16 ("Unknown_Architecture", "x86_Architecture", "x64_Architecture")`

- - System\~Acpi.IsSupported `bool`

- - System\~Acpi.Table `ptr`

- - System\~Cpu.Cr0 `uint64` *optional if .ProtectionLevel != 0*

- - System\~Cpu.Cr3 `uint64` *optional if .ProtectionLevel != 0*

- - System\~Cpu.Cr4 `uint64` *optional if .ProtectionLevel != 0*

- - System\~Cpu.Efer `uint64` *optional if .ProtectionLevel != 0*

- - System\~Cpu.Pml4 `uint64` *optional if .ProtectionLevel != 0*

- - System\~Cpu.ProtectionLevel `uint8`

- - System\~Smbios.IsSupported `bool`

- - System\~Smbios.Table `ptr`

&nbsp;


## [N/A, again]

Checksum `uint16 (sum all 16-bit words)`
