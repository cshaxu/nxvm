# M5 T320 S2: CPL0 IRET Return To VM86

## Implementation P1

T320 S2 adds the bounded Intel 80386 inverse of T320 S1: a CPL0 32-bit `IRET`
can atomically return from a full nine-dword VM86 frame.  The code preflights
and reads the complete frame before publishing VM, segment caches, EIP, ESP or
CPL.  It restores VM86 real-mode-style caches for CS, SS, ES, DS, FS and GS.

The owner smoke records direct `IRET` and address-size-prefixed `IRET`, exact
frame/cache contents, continued VM86 execution, and the short-stack no-partial
publication boundary.  It is registered as one current gate with target-local
GCC `-Wall -Wextra -Wpedantic -Werror`.

The S1 VM86 IRQ0 owner fixture additionally now installs a CPL0 `IRET` handler
and proves the real delivery-to-return round trip: the saved VM86 frame is
restored, VM86 resumes at the interrupted instruction boundary, and IRQ0
remains delivered in ISR rather than being spuriously re-pended.

## Boundaries

This history does not claim VM86-origin IRET, VME/PVI, NT/task returns, task
switches, paging, call gates, 16-bit gate breadth, or a generic interrupt/PIC
redesign.  Those require separate admission.  The Intel SDM is the governing
instruction and system-programming reference for this form.

Detailed matrix and caller sweep: [T320 S2 evidence](../etc/evidence/t320-s2-vm86-iret-matrix.md).

## Artifact

Fresh `mingw-gcc-x64` configuration rebuilt the current
`build/output/nxvm_0_5_0320.exe` artifact from the P2 source tree. Its
SHA-256 is `4E66566B83900E6AABC9ECA54732E1E5B266809846AFD740D7019BE9669A378A`;
the CMake artifact target embeds version `0.5.0320` and runtime identity
`Neko's x86 Virtual Machine`.
