# M5 T359: Complete Instruction-Timing Corpus

## Task Record

T359 extends T357's deliberately finite instruction-cost corpus into a
complete, source-accounted four-profile program.  It retains one successful-
retirement elapsed-tick publisher and never conflates Intel instruction clocks
with physical bus, device, or host timing.

## Accepted Progress

### S1: Four-profile instruction-timing inventory

S1 is accepted at `a6a52001`.  Its
[inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md)
mechanically binds the 256-entry primary and 256-entry `0F` dispatch tables to
the metadata and the sole successful-retirement cost publisher.  It records
every implemented timing form as selected, a named T359 mechanism receiver,
rejected, or external; it preserves the existing one-tick unallocated form
result strictly as a transfer policy.

The task artifact is `build/output/nxvm_0_5_0359.exe`, SHA-256
`A9F0D342753C7F3BDC2C0492E08E103F3A0D8C7BB651ACE5ABAD5D75F0C39134`.
The inventory gate, retained five timing-ledger markers, documentation
governance, and 240/240 current-gate tests passed.  S2 next owns the combined
arithmetic, FLAGS, data, and ModRM/EA source matrix; physical timing remains
explicitly outside this task's form-cost owner.

### S2: Four-profile arithmetic, FLAGS, data, and ModRM/EA timing

S2 is accepted at `5e685e52`.  Its
[source ledger](../etc/evidence/t359-s2-four-profile-arithmetic-data-source-ledger.md)
adds one private normalized-encoding classifier before the retained profile
fallback owners and keeps dynamic 80386 multiplication in its only required
peer.  It allocates the admitted primary ALU, TEST/XCHG, adjustment/conversion,
INC/DEC, Group 3, immediate-IMUL, selected MOV/LEA, and SETcc rows with
four-profile source provenance.  The classifier distinguishes a word read
from a word read-modify-write, and its maximum preflight is the documented
80386 memory-IDIV 46-clock row.

The source-only 8086/80186 range rows remain transferred to T360 rather than
being approximated.  The owner smoke, retained timing-ledger smoke, strengthened
inventory verifier, documentation governance, and 241/241 current-gate tests
passed.  The rebuilt developer artifact is `build/output/nxvm_0_5_0359.exe`,
SHA-256 `2D91DCD23488FD86E64B1EB5126C27DE293183619F7841C96E447FBA63E15904`.

### S4: Four-profile string, repeat, and ordinary-I/O timing

S4 is accepted at `5b78ec92`. Its
[source ledger](../etc/evidence/t359-s4-four-profile-string-io-source-ledger.md)
replaces the former MOVSB-only special case and handler-local ordinary-I/O
rows with one private classifier at the sole successful-retirement publisher.
It allocates the cited primitive and defined REP rows for `MOVS`, `CMPS`,
`STOS`, `LODS`, `SCAS`, `INS`, `OUTS`, immediate-port `IN`/`OUT`, and DX-port
`IN`/`OUT`, including 80386 real/protected/permission-map/ordinary-VM86 I/O
outcomes. Its repeat signature carries the opcode, repeat prefix,
operand/address-size state, and `CS:EIP`, so a reset or changed instruction
cannot inherit a continuation charge.

The owner proof covers primitive rows, zero/first/continuation repeat paths,
comparison stop, provider-fault zero publication, the 80186 27-clock
preflight boundary, reset, and the 80386 I/O mode matrix. The selected 80286
Appendix-B source has no REP LODS formula, so that one successful form remains
an explicit T360 transfer; physical/device service, waits, exception delivery,
secondary, and privileged timing remain later receivers. The inventory
verifier, documentation governance, and 243/243 current-gate tests passed.
The rebuilt developer artifact is `build/output/nxvm_0_5_0359.exe`, SHA-256
`D8422AE25FF8608282FAD66E057F76F5BC56143377AAAF4210BF4396A7B9FD89`.
