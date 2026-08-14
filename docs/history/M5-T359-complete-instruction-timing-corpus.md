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

### S3: Four-profile control and stack timing

S3 is accepted at `a33d9d31`/`c69852cd`, with acceptance closure
`89ec6b81`. Its
[source ledger](../etc/evidence/t359-s3-four-profile-control-stack-source-ledger.md)
adds the private control/stack classifier at the same successful-retirement
publisher. It allocates selected real-mode transfer, stack, loop, HLT,
software-interrupt, and same-level-return rows, while transferring protected
and cross-privilege transitions, task/VM86 delivery, physical timing, and
source conflicts. The retained owner marker, inventory verifier, documentation
governance, and 242/242 current-gate tests passed; the artifact SHA-256 is
recorded in that ledger.

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

### S5: 80386 secondary-integer and prefix/width timing

S5 is accepted at `4c857b37`. Its
[source ledger](../etc/evidence/t359-s5-80386-secondary-source-ledger.md)
adds one private secondary-encoding classifier at the same successful-retirement
publisher. It allocates 80386 near `Jcc`, bit test/modify, double shifts,
two-operand `IMUL`, bit scan, and `MOVZX`/`MOVSX`, while retaining S2's exact
`SETcc` owner and its PRM early-out multiplier mechanism. The classifier uses
the decoder-captured source operand and the existing lexeme preview; no handler
or decoder owns a clock.

The selected PRM `10+3n` bit-scan equation exposes a 106-clock 32-bit all-zero
`BSR` maximum. S5 raises only the global preflight maximum and updates retained
smokes to distinguish that global 105/106 admission boundary from their own
instruction-result assertions. The owner smoke covers branch truth, register
and memory rows, immediate extensions, 66/67/FS/LOCK examples, dynamic `IMUL`,
scan extremes, preflight, and rejected-LOCK zero publication. Privileged
secondary forms, delivery timing, physical timing, and unresolved source
conflicts remain S6/T360 transfers. The inventory verifier, documentation
governance, and 244/244 current-gate tests passed. The rebuilt artifact is
`build/output/nxvm_0_5_0359.exe`, SHA-256
`CB9AC61ABF122C726D493F88D76721B26A88B581538988C496B26B5969E85538`.

### S6: 80286/80386 privileged-form timing reconciliation

S6 is accepted at `43e6b74d`. Its
[privileged-form ledger](../etc/evidence/t359-s6-privileged-source-ledger.md)
adds one private 80386 classifier at the successful-retirement publisher for
fixed source rows: ARPL; selector/table and MSW forms; LAR and CLTS; CR, DR,
and TR moves; FS/GS stack forms; and LSS/LFS/LGS. The classifier uses only
the completed decoder capture and pre-retirement mode/privilege state. It
does not modify handlers, decoding, descriptor state, delivery, or bus
ownership, and assigns no successful time to rejected, faulting, or delivery
paths.

The ledger explicitly transfers every 80286 Appendix-B form, descriptor-
granular LSL, unselected mode/prefix outcomes, task and delivery transitions,
and physical timing to T360 or the later cycle-exact/bus receivers. The
focused smoke proves representative fixed forms plus rejected LOCK zero
publication; retained owner smokes preserve the corresponding semantic
coverage. The inventory verifier, documentation governance, and 245/245
current-gate tests passed. The rebuilt developer artifact is
`build/output/nxvm_0_5_0359.exe`, SHA-256
`65823D830715FC8925DBD810C2BCDEC3AEEB654D3B6B96BB022870ABF9B6783D`.

### S7: Complete instruction-timing corpus closure audit

S7 is accepted at `d5f76911`. The
[closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md)
reconciles the complete S1 primary and secondary inventory to the S2--S6
source ledgers and the sole successful-retirement publisher. It confirms the
retained one-tick result is an explicit transfer sentinel, not an Intel timing
claim, and finds no handler-local or second elapsed-tick publisher.

The task closes only the bounded source-accounted instruction-retirement
corpus. It transfers range-only and conflicting primary material, 80286
Appendix-B normalization, granular LSL, and incomplete selected variants to
T360; delivery and transition paths to the cycle-exact receiver; physical
wait/ownership to bus timing; device service to its own corpus; and x87/VME
breadth to their existing TODOs. The inventory verifier, documentation
governance, diff check, and 245/245 current-gate tests passed. T359 does not
make an L3 or cycle-exact claim.
