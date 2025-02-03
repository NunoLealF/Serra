# Serra - a boot manager under construction <3

&nbsp;

<p align="center">
  <img src="Branding/Serra-logo.png" alt="The Serra logo"></img>
</p>

&nbsp;

Serra is an open-source x86 boot manager currently under construction.

My goal was to develop a universal and reliable boot manager that works across
different systems, focusing on code quality and user experience.

It's currently a work in progress, but **(TODO - finish this soon)**
**(also, i could probably add better screenshots, etc. etc.)**

<p align="center">
  <img src="Branding/Screenshot.png" alt="A screenshot of Serra"></img>
</p>

&nbsp;

## Roadmap

### Boot/
- Finish working on my legacy/BIOS bootloader (roadmap in Boot/Legacy/);
- - Work on a watermark allocator (it should be okay for page tables);
- - Create a decent paging implementation;
- Create a proper outline for how the bootloader will hand over control to the kernel;
- In the future, work on the UEFI bootloader part (Boot/Efi/).

### Branding/
- - Update screenshots, add more media;

### Common/
- Create a common 'kernel', loaded by the bootloader in Boot/Legacy or Boot/Efi;
- - Work on boot tables, configuration files, etc.;
- - Implement a few common drivers, for things like graphics support, PCI, etc.;
- - Add filesystem support (something involving a VFS maybe?);
- Add the actual boot manager functionality (support for boot protocols);
- Create a simple user interface (GUI *and* text-mode as fallback)

&nbsp;

## Disclaimer
This project has been released under the MIT license. For more information, please
refer to the accompanying license agreement. <3
**(TODO: add a disclaimer that it's not affiliated with MIT)**

*Last updated on February 3rd 2025*
