# M5 T318: SGDT/SIDT History

## Implementation P1

T318 S1 implements and proves only Intel 80286/80386 `SGDT` `0F 01 /0` and
`SIDT` `0F 01 /1` memory pseudo-descriptor stores.  It does not claim a full
processor-control, descriptor-table, protection, interrupt, paging, task,
V86, LOCK-policy, or 80387 capability.

The local `INS_0F_01` correction replaces the prior limit-then-base pair of
writes for these two forms with a complete six-byte write preflight and single
existing logical-memory-route publication.  It prevents partial publication
when a protected destination limit excludes the tail of the pseudo-descriptor.
`LGDT`/`LIDT` `/2,/3`, `SMSW` `/4`, reserved `/5`, `LMSW` `/6`, and reserved
`/7` retain their existing callers and dispositions; no shared decoder or
memory route changed.

`tests/machine/core_machine_sgdt_sidt_smoke.c` is the standalone owner smoke,
registered once in `PROJECT_CURRENT_SMOKE_TARGETS`.  Its marker is
`M5:T318:S1:SGDT-SIDT:OK`.  It proves both forms over 80286/80386 real and
protected state, 80386 operand/address attributes and exact EIP, different
GDTR/IDTR images, DS/SS/ES addressing, rejection boundaries, protected
six-byte atomicity, pending-PIC ordering, unchanged successful CPU state, and
the factual VM86 classification.  The detailed form table and caller sweep
are in [T318 S1 evidence](../etc/evidence/t318-s1-sgdt-sidt-matrix.md).

## Verification And Artifact

- Fresh `mingw-gcc-x64` configuration completed.
- Focused target and marker passed.
- The final implementation P1 reran the current strict/type/gate and
  documentation checks recorded below.
- Current artifact: `build/output/nxvm_0_5_0318.exe`.
- SHA-256: `700DA3C0BB4FF0C1F5443FE2EF8623DA65E1BBD0E4354979FA3245A6CCD4583C`.
- Runtime identity: the current target supplies `PRODUCT_BUILD_VERSION`
  `0.5.0318` and the product identity is `Neko's x86 Virtual Machine`.
- Artifact source commit: `8c3f8761` (the P1 source/build commit before this
  evidence-only record was finalized).

## Retained Boundary

The Intel 80386 PRM table-register entries govern only the admitted stores.
The existing 8086 legacy `0F` POP-CS compatibility behavior is not an SGDT/SIDT
form and is retained outside the 80286-or-later decoder scope.  T318 remains
active pending coordinator review and closure.
