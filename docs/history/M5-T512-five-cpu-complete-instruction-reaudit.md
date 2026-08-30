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
the available 8086/8088, 80186 and 80386DX instruction-time tables. It also
corrects a material provenance error: the previously cited 80286 Appendix B is
an encoding dictionary, not an instruction-time table. No CPU behavior changes
in S1; S2 must carry that timing-source boundary into every 80286 row.
