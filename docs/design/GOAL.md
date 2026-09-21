# Project Goals

NXVM is a whole-machine x86 emulator, not a DOS implementation or VDM host.
The direction is two fixed-machine executables over one multi-machine
architecture, not two independently maintained emulators.

1. **Standard machine.** Deliver one reproducible Intel 386DX machine selected
   for documented hardware, conventional Intel controller mechanisms and a
   complete, lawful external firmware set. The board, revision and expansion
   hardware require evidence before selection is final.
2. **IBM Palm Top PC 110.** Deliver a separate executable for documented PC110
   hardware, including its required 486-class CPU and machine-specific devices.
   Generic PC support is not a claim of PC110 compatibility.
3. **Fixed products, reusable CPUs.** Each executable has one build-selected
   machine profile. Preserve CPU implementations, profiles and selection tables
   across 8086/8088, 186/188, 286, 386 and 486, even when unused by either product.
   Preservation does not imply every listed model is implemented or qualified.
4. **Smaller, coherent implementation.** Retain standard Intel device mechanisms
   and PC110-required components; retire unrelated legacy-machine special cases
   without duplicating CPU, memory, device or time owners. Non-Intel support
   hardware needed by a proposed Standard board must be identified and accepted
   at selection, not silently added.
5. **Usable, evidenced machines.** Preserve Console/Window interaction,
   debugging and assembly/disassembly. Qualify boot, DOS and selected
   compatibility workloads with reproducible evidence and honest timing grades.

The former NXVDM, mantle and owned-DOS direction is withdrawn. Host portability
remains valuable; new browser or DOS-host products are not implicit obligations.

[System Architecture](ARCHITECTURE.md), [Product UX](UI.md) and
[Roadmap](ROADMAP.md) own the corresponding design and delivery decisions.
