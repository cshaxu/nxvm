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

## Corrective P2

Coordinator review found that P1's owner smoke had not provided a complete
per-opcode proof for several `/1` boundaries. P2 changes only that smoke and
its evidence: every requirement that depends on the table-store opcode now
loops over both SGDT `/0` and SIDT `/1`. It proves, for each form, register
ModRM and F0 LOCK rejection, protected six-byte destination atomicity,
pending-PIC delivery after the actual instruction, VM86 image/state, and
DS/SS-default/ES-overridden effective addresses. The matrix now states that
coverage precisely; no production, decoder, table-load, control-register,
paging, or delivery implementation changes are introduced.

P2 reruns fresh GCC configuration, the focused marker, exact current-gate
registration discovery, documentation governance, diff check, full current
gates, and the current `0.5.0318` artifact hash/runtime identity before its
single pushed corrective commit. The rebuilt artifact remained SHA-256
`700DA3C0BB4FF0C1F5443FE2EF8623DA65E1BBD0E4354979FA3245A6CCD4583C`.

## Corrective P3

P3 completes the low-profile proof by running the 80186 native-`#UD` and
no-publication vector independently for both SGDT `/0` and SIDT `/1`. It also
records the artifact-cleanup correction: the executor's earlier interactive
runtime-identity launch left the exact `vm-0-5-0318.exe` process alive. P3
verified its PID/path ownership, terminated only that process, confirmed no
such artifact process remained, then freshly configured and rebuilt the
current artifact before its final gate run. No production or behavior change
is part of P3. The freshly relinked `0.5.0318` artifact, identified as
`Neko's x86 Virtual Machine` with `PRODUCT_BUILD_VERSION` `0.5.0318`, has
SHA-256 `60B3CDF0BA63C6303E9809146CEBF2B7CDC278D20A79D05174197E05089030D9`.

## Retained Boundary

The Intel 80386 PRM table-register entries govern only the admitted stores.
The existing 8086 legacy `0F` POP-CS compatibility behavior is not an SGDT/SIDT
form and is retained outside the 80286-or-later decoder scope.  T318 is closed.

## Coordinator Acceptance P4

The coordinator independently reviewed all P1--P3 changes, including the local
six-byte write preflight/publication helper, every owner-smoke vector, CMake
registration, packet, matrix, and retained boundaries. It rebuilt the current
artifact from the pushed P3 source, reran the focused owner marker and exact
current-gate discovery, documentation governance, `git diff --check`, and the
full 195-test current gate. All passed. The independent rebuild recorded
SHA-256 `013F9BF11AC4A5600AB7587DBAE8E63CE2E86FB534BC861787F095DCC4E192EB`.

The P3 executor rebuild hash
`60B3CDF0BA63C6303E9809146CEBF2B7CDC278D20A79D05174197E05089030D9` and this
coordinator rebuild hash are separate build observations of the same admitted
source; this P4 record is the task-closure acceptance evidence. T318 closes
only SGDT/SIDT. LGDT/LIDT and remaining processor-control work remain in the
Queue for separately approved matrix slices.
