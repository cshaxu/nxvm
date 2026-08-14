# M5 T361: Legacy Dynamic Arithmetic Timing Authority

## Task Record

T361 consumes the T360 S3 transfer for range-only 8086 and 80186 arithmetic
timing and the retained 80286 `NOP` source conflict. It establishes the
approved profile-local timing authority or model-derived disposition for the
legacy arithmetic forms, then performs the bounded Appendix-B-versus-prose
precedence and consumer review for 80286 `NOP`. It does not borrow 80386
behavior, choose a range endpoint, or turn host measurement or another
emulator into primary authority.

## Accepted Progress

### S1: Primary-source formula audit

S1 is accepted at `e8c662f7`. Its [authority audit](../etc/evidence/t361-s1-legacy-dynamic-arithmetic-authority-audit.md) confirms that all transferred 8086 and 80186 Group-3 and immediate-IMUL forms remain table ranges without a primary operand-to-clock formula. The profile-local source routes therefore retain their explicit unallocated successful-retirement transfer, with no endpoint, later-profile formula, emulator, benchmark, or hidden timing allocation.

The only lawful next path is an owner decision: retain the transfer pending new
primary material, or approve the bounded real-hardware observation contract
specified by S1. Focused legacy timing smokes and 245/245 current-gate tests
passed; S1 makes no runtime, ABI, artifact, L3, or cycle-exact change.

The owner subsequently excluded unavailable physical-hardware measurement and
approved the source-labelled reference-model path. After the dynamic arithmetic
S units, the next T361 S is the narrow 80286 `NOP` Appendix-B/prose source and
consumer review; it is no longer a separate Queue candidate.

### S2: Owner-approved external cross-reference research

S2 is accepted at `3ef7f1d7`. The owner authorized a bounded non-importing
review of Bochs, PCjs, PC110-EMU, and public technical material. Its result
and exact source boundary are recorded in the
[cross-reference evidence](../etc/evidence/t361-s2-legacy-dynamic-arithmetic-cross-reference.md).

### S3: Reference-model selection correction

The owner clarified that PCjs is not to be treated as the universal early-x86
timing reference. PCjs declares CPU models through 80386, but its timing data
does not establish an independent 80186 table and some of its 8086 arithmetic
constants are explicitly benchmark-adjusted. S3 therefore ranks candidate
inspection by relevance: Intel documentation first; 86Box for early-PC timing;
MAME i86/i186 for 80186; PCjs only as a verified 8086 candidate; then Bochs
and PC110-EMU for their narrower functional/machine roles. QEMU `icount` is
retained solely as a deterministic event/instruction-count architecture
reference, not a CPU-cycle source. Every resulting allocation remains
source-labelled and range-checked where Intel specifies a range.

S3 is accepted at `914c6afc`. Its
[audit](../etc/evidence/t361-s3-synchronous-fault-retirement-audit.md)
records the Intel-range ledger, selected MAME i86/i186 constants, explicit
`reference-exhausted` immediate forms, and the no-retirement synchronous-fault
execution boundary. The result is a source-labelled model-L3 allocation only;
it makes no physical or cycle-exact claim. The next T361 S is the separately
bounded 80286 `NOP` Appendix-B/prose source and consumer review.
