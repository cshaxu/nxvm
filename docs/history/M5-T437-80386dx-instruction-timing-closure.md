# M5 T437: 80386DX Instruction Timing Closure

## Task Record

T437 owns the complete 80386DX successful-retirement instruction function and
timing closure transferred from the owner-approved requeue of the former
four-profile program. Its final universe is the 450 all-L3 base forms and 960
finite legal context keys in the retained 80386DX manifest: 1,410 canonical
keys. It implements every scalar CPU key through one Core-private
timing-selection and retirement-publication route, with the Chapter-17
value/formula, actual normalized inputs, origin and
`source_timing_unallocated=false`. The sole ESC key is a verified CPU/FPU
handoff in the separate MCP clock domain, as required by the manual.

The task preserves accepted instruction semantics unless focused execution
proves a concrete 80386DX semantic defect required for a successful manifest
key. It does not take ownership of external waits/arbitration, prefetch,
physical board cycles, device response, exception/event delivery, public ABI
work, source import or a new profile.

## Subtask Decomposition

| S | owned closure boundary |
| --- | --- |
| S1 | freeze an executable, disjoint 1,411-key partition and proof plan |
| S2 | result-contract producer and sole 80386DX timing publisher |
| S3 | ordinary arithmetic/data, r/m, size, segment, LOCK, multiplier and XLAT forms |
| S4 | direct I/O, strings, port strings, repeat phases and overrides |
| S5 | ordinary stack, conditional control and real-mode transfer paths |
| S6 | protected/VM86 privilege, gate, task and interrupt transfer paths |
| S7 | protected segment, descriptor, special-register and system forms |
| S8 | full generated-key closure, regression, cleanup and task audit |

The retained [candidate proposal](M5-T437-80386dx-instruction-timing-closure-proposal.md)
defines the exact exit standard for each S. S8 cannot absorb unimplemented
families: each remains assigned to its originating implementation S.

## Initial Subtask

S1 derives the partition from the retained manual ledger, manifest, decoder
inventory, implementation tracker and result contract before runtime changes.
It must prove exactly one S2--S7 owner for each canonical key, preserve the
strict nonconforming baseline and record the focused evidence that later S
units must produce. It creates no timing result or implementation claim.

## Accepted Progress

### S1: Canonical-Key Partition

S1 added an executable partition verifier and indexed evidence. The verifier
assigns the complete 1,411-key manifest exactly once: S3 owns 217 base and 808
canonical keys, S4 60/162, S5 80/232, S6 40/94 and S7 53/115. It keeps the
existing strict baseline visible: zero conforming keys, 192 missing inputs, 257
missing tests and one unallocated XLAT form. No runtime source, CMake product,
developer artifact, public ABI or timing result changed.

## Coordinator Acceptance

The single-session coordinator reviewed P1 `467e8299` against the S1 packet.
The partition verifier, manifest verifier, decoder-ledger verifier, existing
80386 decoder-inventory runner and documentation governance gate passed. The
current GCC subprocess fails before source diagnostics in `audit-current-head`;
the retained current-source inventory runner replayed successfully and produced
no inventory diff. The code and evidence changes are confined to partition
proof and task governance; the review found no duplicate, orphaned or silently
reassigned canonical key.

### S2: Result Producer

S2 adds the private 80386DX manifest runner, its generated-catalog dependency
and a guarded final JSON writer. It captures the first real retirement per
canonical record, preserves origin/ticks/form/formula/disposition state, and
checks the full 1,411-record denominator before opening an output file. The
initial real `NOP` capture proves one record; final emission is refused and no
result artifact is created until S3--S7 supply every remaining recipe.

## Coordinator Acceptance

The single-session coordinator reviewed P1 `4658ce26` against the S2 packet.
The isolated WinLibs GCC C11 build compiled the new runner with warnings as
errors; it captured one classified 80386DX retirement, emitted both S2 markers
and verified no partial final-result file exists. Manifest, decoder-ledger,
S1 partition and documentation-governance gates passed. The change adds one
Core-private test consumer only, leaves runtime ownership and public ABI intact,
and transfers the 1,410 unobserved keys to their existing S3--S7 partitions.

### S3--S7: Complete Execution Coverage

S3 (`c83e5016`) accepted 809 ordinary scalar/formula keys and the distinct
CPU/FPU handoff contract. S4 (`4a9e78b3`) accepted all 162 direct-I/O,
string and REP keys. S5 (`ea439517`) accepted 234 ordinary-control keys.
S6 (`443f1b1a`) removed the manual-invalid 16-bit protected-to-VM86 IRET
context, establishing the final 1,410-key universe, and accepted all 89
protected-control keys. S7 (`58bc69b8`) accepted the remaining 115 protected
segment, descriptor, special-register and system keys. Their retained S3--S7
evidence records the actual protected fixtures, formula inputs and manual-first
corrections; no accepted subtask introduces board, host or public-ABI timing.

### S8: Result Publication, Regression And Task Audit

S8 P1 (`9f030a02`) enables the final writer. It emits 1,409 classified scalar
CPU retirements plus `I386-ESC`, a separately verified `CPU_FPU_COMMAND`
handoff with `timing_domain: mcp`, `ticks: null`, selected 80387 profile and
the data-sheet FADD interval 12--26 MCP clocks. It does not invent a scalar
CPU clock for ESC. The shared verifier selects the canonical catalog by
profile and retains validation of the 8086, 80186 and 80286 artifacts.

The S8 current-gate sweep found only stale gate metadata or stale timing-test
expectations. It corrected the T337 #UD target inventory, made T344 enumerate
its two registered auxiliary current tests explicitly, aligned Model-40 and
prefixed/BSF/BSR timing smoke expectations with the retirement publisher, and
made the T359 S6 privileged rows execute in a real 80386 CPL0 protected
fixture. It also aligns LSL's byte-granular register/memory cases with the
published 21/22-clock selector. These changes alter no production CPU path,
public ABI, board policy, artifact identity, source import or x87 arithmetic.

## Final Closure Audit

| Required outcome | Current evidence |
| --- | --- |
| Complete legal 80386DX universe | `Verify-80386TaskPartition.ps1` reports 1,410 keys with S3/S4/S5/S6/S7 ownership 810/162/234/89/115 and no overlap. |
| Correct scalar timing/function publication | The final manifest run reports 1,409 classified CPU observations, all S3--S7 coverage markers, and final result closure 1,410 = 1,409 CPU + 1 MCP. |
| Manual ESC boundary | `M5:T437:S3:I386-ESC-HANDOFF:PASS`; the retained 80386DX/80387 manual evidence and S8 result artifact keep the MCP interval separate from CPU retirement ticks. |
| Result-contract and cross-profile compatibility | `Verify-CpuTimingResults.ps1` passes the 1410/1053/616/771 80386/8086/80186/80286 artifacts. |
| Decoder and source-model audit | `Verify-80386DecoderLedger.ps1` reports zero canonical difference for all 1,410 keys; the fresh Debug configuration and decoder inventory runner pass. |
| Current regression and governance | Full `current-gate` passes 292/292; T337/T344/T382 pass; documentation governance and manifest-contract gates pass. |
| Architecture, coding and source boundaries | Actual-change review finds only private runner/verifier/result/evidence, current-gate registration and existing smoke fixture changes; no public ABI, production board/timing path, external source, firmware or guest media change. |

The single-session coordinator independently inspected the final diff and the
full verification evidence above. The S8 result artifact is complete and the
manual remains the authority where an inherited test expectation conflicted
with the selector. T437 is closed; deferred physical timing and x87 arithmetic
remain only the already-recorded separate debt boundaries.
