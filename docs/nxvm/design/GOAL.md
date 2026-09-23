# Project Goals

NXVM is a whole-machine x86 emulator, not a DOS implementation or VDM host.
The direction is extensible, build-fixed machine executables over one
multi-machine architecture, not independently maintained emulators.

1. **Retain every implemented machine.** Preserve IBM 5160 XT, IBM 5170 AT,
   DeskPro 386 and default PC/AT, including their implemented variants and
   required hardware. Structural simplification does not select a replacement
   Standard board or retire any of these machines.
2. **IBM Palm Top PC 110.** Deliver a later executable for documented PC110
   hardware, including its required 486-class CPU and machine-specific devices.
   Generic PC support is not a claim of PC110 compatibility.
3. **Fixed products, reusable CPUs.** Each executable has one build-selected
   machine profile. Preserve CPU implementations, profiles and selection tables
   across 8086/8088, 186/188, 286, 386 and 486, even when unused by a product.
   Preservation does not imply every listed model is implemented or qualified.
4. **Smaller, coherent implementation.** Remove duplicate mechanisms, not
   supported machines. Preserve Intel and machine-specific hardware required
   by the retained profiles, with one CPU, memory, device and time owner.
5. **Usable, evidenced machines.** Preserve Console/Window interaction,
   debugging and assembly/disassembly. Qualify boot, DOS and selected
   compatibility workloads with reproducible evidence and honest timing grades.

The former NXVDM, mantle and owned-DOS direction is withdrawn. Host portability
remains valuable; new browser or DOS-host products are not implicit obligations.

[System Architecture](ARCHITECTURE.md), [Product UX](UI.md) and
[Roadmap](ROADMAP.md) own the corresponding design and delivery decisions.
