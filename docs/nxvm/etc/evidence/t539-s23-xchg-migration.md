# T539 S23: XCHG Migration

Baseline `8dd52d43f`, clean intake. Read all 876 lines of the original
core_machine_xchg_smoke.c; no source includers. Current owns admission and
acceptance. This batch consumes one of 96 original private CPU consumers.

## Original Case Map

| Family | Instruction contexts | Receiver |
| --- | --- | --- |
| Real register/memory widths | 6 | CPU |
| Legacy rejected prefixes | 9: three profiles x three prefixes | CPU |
| Legacy default16 | 6: three profiles x two sequential instructions | CPU |
| Plain/LOCK memory and rejected LOCK register | 3 sequential instructions | CPU |
| Accumulator default | 32: four profiles x eight opcodes | CPU |
| Accumulator 386 width | 8 | CPU |
| Accumulator rejected prefix | 24: three profiles x eight opcodes | CPU |
| Accumulator rejected LOCK | 8 | CPU |
| Protected write/read fault atomicity | 1 + 2 | Board |
| ModR/M and accumulator IRQ | 1 + 1 | Board |

Twelve original test families total 101 instruction contexts: 96 chip and five
board. This counts original instruction executions, not full ISA coverage.
Tables, loops, GPR/FLAGS/memory rollback and nonparticipant checks are retained.

## Boundary Design

Use the existing CPU-owned instruction fixture for chip cases. Keep the
original general/accumulator tables and helpers, terminal-UD setup and sequential
instruction ordering. Remove board construction/reset callbacks from chip tests.

Board protected tests currently mutate DS writable/limit after loading a real
GDT. Instead construct the same read-only or short-limit descriptor before the
guest loads DS/ES; execute the original ten setup instructions and stop before
HLT. Public register patches seed EAX/FLAGS, and copied snapshots observe
rollback. Preserve the original post-fault physical-memory read at its board
owner because the public paused read excludes FAULTED state. IRQ tests retain
actual PIC state and stack frames through public CPU setup/observations.

No production CPU instruction or timing implementation changes, no mutable
CPU accessor, mirrored state or second execution path. S38/S39 still own
opaque lifetime and physical relocation. All other original raw consumers
retain their planned receivers.

## Verification And Review

Both existing full builds succeed. Complete units pass 376/376 on x64
(26.86 seconds) and x86 (29.86 seconds), with desktop runs sequential across
trees. All 66 specialized steps pass, including 375 strict-matrix entries
(342 strict, 33 unchanged deferred), 101 inventoried constructors / 108 total
classified constructors, and exact T332 public-setup membership. The CPU/board
boundary test now covers five migrated board tests / twenty negative controls
(97 controls overall). All six Shared manifests remain unchanged and verify.

Read-only structural comparison confirms identical arrays and loop headers for
all twelve original test families; all four accumulator selection/comparison
helpers are byte-identical. Assertion review retains original values, masks,
widths, rollback, nonparticipants and PIC stack-frame IP=4/1. CPU tests retain
original detailed failure messages and sequential instruction setup. The plain
memory instruction in the three-step LOCK case now explicitly checks successful
execution as well as successful diagnostic capture; no assertion was relaxed.

The board setup now loads the short/read-only descriptor through guest code
before the test instruction instead of patching DS internals after HLT. ES
loads that same descriptor too; these measured instructions use only DS.
Both fault cases retain their original post-fault board-owned physical-memory
inspection. No private CPU token remains in the board source; no source
includer depends on it. The extra broad-search hit is the boundary negative
test's deliberate rejection text, not another CPU consumer. The original
inventory has 95 remaining files, all assigned to S24-S37.

An initial transient CTest expression omitted the unit prefix and selected no
tests; it is not verification evidence. Corrected selection ran both targets,
then the complete suites above ran them again. Self-review caught a mechanical
call rewrite before compilation; the implementation did not require any CPU
behavior repair. Production, Shared, MyNES and owner INIs are unchanged.
No EXE input changes, so the existing eight 0539 EXEs remain current. No new
build tree or trace was created; retained trees/recovery patch serve later S.

Staged Git numstat over seven test/build paths, excluding four task documents:
609 added, 688 removed, net -79 lines. The single CPU production owner and
execution path are unchanged; the new CPU test replaces, not duplicates, the
removed board-owned instruction cases. Documentation and staged diff checks pass.

Executor review is complete; coordinator actual-commit acceptance remains
separate. This batch does not accept the CPU extraction row or close T539.
