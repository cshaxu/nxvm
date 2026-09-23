# NXVM Repository

This repository hosts two products under one root MIT license: NXVM, the x86
whole-machine emulator, and MyNES, the NES emulator. Shared `lib`, `common`,
and `x86` components remain neutral; each product owns its App/Core, tests,
tools, documentation sequence, and versioned executable artifacts.

Neko's x86 Virtual Machine is a portable x86 PC emulator with an integrated
debugger. NXVM is the repository's current, bootable whole-machine product.
The approved direction is one build-fixed executable per machine: IBM 5160 XT,
IBM 5170 AT, DeskPro 386, default PC/AT, and later IBM Palm Top PC 110.
They share an extensible architecture and one NXVM.ini configuration format.
CPU-family implementations remain reusable. This is the target, not a claim
that the INI/fixed-build cutover is implemented. All implemented machines remain
supported; the former NXVDM plan is withdrawn.

## Introduction

NXVM emulates an Intel 80386 PC with the machine services needed for the
current product: RAM, DMA, PIC, floppy and hard disks, keyboard, display, BIOS
behavior, and a debugger. It is useful both as a bootable virtual machine and
as a controlled environment for examining legacy x86 software.

The screenshots below are historical NXVM product captures retained from the
original project README. They illustrate the product's lineage; they are not
current compatibility claims and include no guest media or executables.

![MS-DOS running in HMA](assets/readme/msdos_hma_w32c.jpg)

![NXVM running on Microsoft Surface RT](assets/readme/nxvm_on_surface_rt_arm.jpg)

## Platform

### Windows

NXVM provides Console and application-window presentation paths. The windowed
path keeps the NXVM control Console separate from the guest display.

![MS-DOS running in the Win32 Console](assets/readme/msdos_w32c.jpg)

![Windowed bitmap-font display](assets/readme/edit_w32a.jpg)

![Windowed display](assets/readme/pv12_w32a.jpg)

### Linux

Linux support uses the terminal presentation path. It remains a development
and verification surface; see the project documentation for current support
boundaries.

![MS-DOS running in a Linux terminal](assets/readme/msdos_linux.jpg)

![MS-DOS Editor in a Linux terminal](assets/readme/edit_linux.jpg)

## Build

The supported development path is 64-bit MinGW-w64 GCC, CMake 3.23 or later,
and Ninja on Windows. Ensure `gcc` and `ninja` are available on `PATH`. From
the repository root, build the selected product:

```powershell
cmake --preset mingw-gcc-x64-release
cmake --build --preset current-gcc

# MyNES
cmake --preset mynes-gcc-x64-release
cmake --build --preset mynes-current-x64
```

Versioned runnable artifacts are retained under `assets/binary-nxvm/` and
`assets/binary-mynes/`. Task closure builds the matching x86 artifact where
the product supports it. Run the selected product documentation gate with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File tools/shared/Verify-DocumentationGovernance.ps1 `
  -RepositoryRoot . -Product nxvm
```

When `ccache` is installed, an optional isolated repeat-build route is:

```powershell
cmake --preset mingw-gcc-x64-ccache
cmake --build --preset current-gcc-ccache
```

Visual Studio is not required for the supported development path. The legacy
manual-project, Autotools, and Makefile instructions are retired.

## Quick Start

1. Build NXVM using the commands above.
2. Start its selected artifact from `assets/binary-nxvm/<profile>/`.
3. In the NXVM Console, use `help` to inspect available commands.
4. Supply only your own legally obtained boot media when experimenting with a
   guest; the repository does not provide or distribute guest media.
5. Use the Console and debugger to inspect machine state, then start, stop,
   and resume the session as supported by the current command surface.

The exact current product and media boundaries are maintained in the
[repository documentation guide](docs/README.md) and its selected product
authorities.

## Components

### Console and machine

NXVM combines a bootable machine composition, firmware/profile behavior,
platform presentation, and the retained Console/debugger experience. The
shared `core` foundation owns reusable guest-machine behavior; NXVM owns the
current product composition and user experience.

![NXVM status information](assets/readme/vminfo_w32c.jpg)

### Debugger

NXVM includes an integrated debugger for examining guest registers, memory,
ports, breakpoints, watchpoints, and instruction execution. Its assembler and
disassembler remain part of that debugging experience.

![NXVM internal debugger](assets/readme/debugger_w32c.jpg)

## Historical application captures

These original NXVM captures are retained for project history and visual
context, not as a claim that every displayed application is currently tested
or supported.

![MS-DOS UMB capture](assets/readme/msdos_uma_w32c.jpg)

![QBasic](assets/readme/qbasic_w32c.jpg)

![Turbo C 2.0](assets/readme/tc2_w32c.jpg)

![MS-DOS Defrag](assets/readme/defrag_w32c.jpg)

![MS-DOS Editor for Windows 95](assets/readme/edit95_w32c.jpg)

![MS-DOS Anti Virus](assets/readme/msav_w32c.jpg)

![Type Tutor start screen](assets/readme/tt_start_w32c.jpg)

![Type Tutor execution](assets/readme/tt_exec_w32c.jpg)

![Tetris start screen](assets/readme/nyet_start_w32c.jpg)

![Tetris execution](assets/readme/nyet_exec_w32c.jpg)

## Start Here

- [Repository Documentation Guide](docs/README.md) is the contributor and
  agent entry point.
- [NXVM documentation](docs/nxvm/README.md) and
  [MyNES documentation](docs/mynes/README.md) own their respective goals,
  architecture, roadmaps, and task queues.
- [Contributing](CONTRIBUTING.md) describes the change and review process.

## Project Boundary

NXVM emulates whole machines, not a DOS/VDM application host. Source, firmware, guest media,
research, and distribution constraints are defined by the local documentation
authorities. Read [AGENTS.md](AGENTS.md) before changing the repository.

## References

The project is informed by Intel 80386 architecture documentation, IBM PC/AT
technical references, and other documented research sources. See the
[repository documentation guide](docs/README.md) for the current source and research
boundaries.
