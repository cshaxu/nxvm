# M5 T320 S2: CPL0 IRET Return To VM86

## Scope and form audit

This packet implements only Intel 80386 protected CPL0, 32-bit-operand-size
`IRET` whose saved EFLAGS has VM set.  The exact stack image is nine dwords in
ascending address order: EIP, CS, EFLAGS, ESP, SS, ES, DS, FS, GS.  The return
uses real-mode-style VM86 segment caches (base `selector << 4`, limit `FFFFh`,
DPL 3) and is not a protected outer return.  `66 IRET` remains the 16-bit form
and cannot encode VM in its popped FLAGS; `67 IRET` has no effective address
and retains the 32-bit return form.  VM86-origin IRET, VME/PVI, NT/task return,
16-bit gates, paging and generic delivery policy remain outside this S.

## Owner proof

`tests/machine/core_machine_vm86_iret_smoke.c` is registered exactly once as
`current.core-machine-vm86-iret-smoke`, uses the target-local GCC strict flags,
and emits `M5:T320:S2:VM86-IRET:OK`.

| Vector | Actual proof |
| --- | --- |
| Valid `CF` | A CPL0 32-bit code frame restores all nine dwords, sets VM and IF, installs CS/SS/ES/DS/FS/GS real-mode-style caches, and executes the next VM86 NOP. |
| `67 CF` | The no-EA address-size form has the same frame/cache result and next VM86 instruction execution. |
| Short CPL0 stack | The 36-byte frame preflight fails before cache/VM/CPL/ESP publication; the complete captured CPU remains unchanged. |

The local implementation now identifies the 32-bit VM frame before consuming
the first three return words.  It preflights and peeks all nine dwords, builds
private VM86 caches, then commits flags, caches, EIP/ESP and CPL together.  This
repairs the prior partial-pop ordering defect without modifying generic
interrupt, PIC, memory transaction, task or paging paths.

## Caller sweep

Reviewed `_e_iret`, `_ser_iret_protected_same`,
`_ser_iret_protected_outer`, `_ksa_load_sreg`, `_s_load_ss`, and the direct
T320 S1 VM86 delivery owner smoke.  Only the protected CPL0 32-bit EFLAGS.VM
branch changes.  The S1 entry test deliberately stops in its handler and is a
retained S2 round-trip fixture dependency; no S1 producer behavior changes.
