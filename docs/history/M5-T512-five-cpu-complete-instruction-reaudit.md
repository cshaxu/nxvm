# M5 T512: Five-CPU Complete Instruction Re-audit And Repair

## Active Contract

T512 re-establishes a complete source-backed conformance result for the
project's supported 8086, 8088, 80186, 80286 and 80386DX CPU profiles. Intel
original manuals are normative. Existing ledgers are crosswalks, and 86Box,
MAME, PCjs, Bochs and QEMU are read-only corroboration only; they cannot
override an Intel-manual conflict.

Core remains the sole owner of decode, architectural state, execution, memory
access, delivery, retirement and guest-time publication. A profile selects a
frozen CPU identity only. Any repair consolidates the shared mechanism at its
Core owner and removes the superseded route.

## S1: Source Admission And Cross-validation

Locate and quality-check original manual material for all five CPU profiles.
Record edition/order locator, original-versus-OCR status, and the bounded
86Box/MAME/PCjs/Bochs/QEMU corroboration or conflict for each instruction,
state, delivery and timing source family. This S creates no CPU implementation
change and does not assert a value for reserved or architecturally undefined
state.

S1 closes only after its source record is complete, documentation governance
passes, and the complete repository-only unit suite passes. S2--S3 must accept
the complete List 1 and List 2 before any repair S begins.

## S1 Result

S1 accepts the five-profile source corpus in
[its evidence](../etc/evidence/t512-s1-five-cpu-source-cross-validation.md).
It confirms original manual coverage for every supported semantic universe and
the available five-CPU instruction-time tables. S3's rendered correction makes
the 80286 locator precise: Appendix B is an instruction dictionary whose
printed B-6 onward also defines its per-form `Clocks` column and qualifiers.
No CPU behavior changes in S1; S3--S7 must map those source rows to current
selectors before any value is called conforming.

## S2: Complete List 1

Freeze a finite List 1 that maps every decoder-admitted instruction form and
applicable state/delivery context across all five profiles to its Intel manual
locator. The ledger must separately classify opcode/prefix legality, operands,
defined and undefined FLAGS, control/system state, success/fault/trap/abort/
interrupt/NMI delivery, and instruction-time fact or explicit absence. It must
not replace a missing 80286 instruction-time source with the previous
Appendix-B claim or an external emulator literal. S2 changes no production code.

## S2 Result

S2 accepts the finite five-CPU List 1 in
[its evidence](../etc/evidence/t512-s2-five-cpu-function-state-timing-list-1.md).
The decoder-inventory producers are the exact opcode/ModR/M expansion, while
the ledger assigns every expansion a manual form, prefix, state/delivery and
timing-source category. S3 corrects the initial 80286 timing classification:
the Intel Appendix-B `Clocks` column is a Manual-L3 candidate, while each
current value remains unverified until S3--S7 map its exact source qualifier.
S3 must make every List-1 expansion's current Core disposition explicit before
repair begins.

## S3: Complete List 2

Trace every List-1 inventory expansion through the current Core decoder,
state construction, commit/rollback, delivery, retirement and regression
owner. Classify it as conforming, missing, conflicting, unsupported or
source-unavailable, including a precise receiver for every nonconforming
batch. This is an audit only: do not modify CPU execution or create a
profile-side behavior path. It closes only after the full repository-only unit
suite and documentation governance pass.

## S3 Result

S3 accepts the complete current-owner partition in
[its evidence](../etc/evidence/t512-s3-five-cpu-current-code-gap-list-2.md).
It corrects the 80286 Appendix-B `Clocks` locator, confirms one Core-only
decode/state/delivery/retirement path, and finds one bounded shared FLAGS
conflict: 80386 real/V86 stack images must clear bit 15, while the current
16-bit helper retains it. Full repository-only unit is 313/313 and
documentation governance passes. S4 is the sole shared-mechanism receiver;
S5--S8 remain the profile-specific form and timing sweeps.

## S4: Shared FLAGS Image, Load And Frame Repair

Repair the one shared Core 16-bit FLAGS **image** mechanism found by S3. Intel 386 DX
Programmer's Reference Manual 230985-003, printed page 14-7, states that
real-address `PUSHF`, interrupt and exception stack images keep bit 15 clear;
the virtual-8086 rule states the same result. The current 80386 branch instead
includes bit 15 in its shared `FFD5h` image mask. S4 changes this one Core
owner and its finite image/test-oracle callers. The shared helper's firmware
load is examined but no 32-bit EFLAGS load or protected-frame contract is
changed. It does not alter decoder admission, timing, VM profile or any value
the manual leaves undefined. Full repository-only unit and documentation
governance are required before its implementation P may close.

## S4 Result

S4 accepts the one-owner repair recorded in
[its evidence](../etc/evidence/t512-s4-shared-flags-image-repair.md): the
shared 80386 real/V86 16-bit FLAGS-image mask is now `7FD5h`, so bit 15 is
clear in PUSHF and real-mode interrupt/exception frames. The production change
is one replacement; the finite six-owner test sweep adds 23 and removes 15
lines, with no 32-bit EFLAGS, protected-frame, decoder, VM-profile or artifact
path changed. The complete repository-only unit suite passes 313/313 in 14.64
seconds, documentation governance passes, and independent actual-diff review
accepts `3b514e50`. S5 is now the active 8086/8088 full sweep.
