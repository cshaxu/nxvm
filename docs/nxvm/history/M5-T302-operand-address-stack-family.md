# M5 T302: Operand, Address, And Stack Family

## Implementation Record

T302 completes the admitted 80386 `66h`/`67h` operand, address, and stack
family in the single core CPU executor: prefix/fetch/effective-address and
general-data forms; `PUSH`/`POP`/`PUSHA`/`POPA`, `PUSHF`/`POPF`, and
`ENTER`/`LEAVE`; and memory/I-O strings with `REP` forms. It does not add a
second executor or state owner, alter public interfaces, or change Console,
debugger, boot, or media behavior. The coordinator independently accepted the
record and closed T302.

## Reference Conclusion And Matrix

Intel's *80386 Programmer's Reference Manual* (1986), Sections 3.6, 8.2,
16.1, 17.1, and 17.2 plus the named instruction entries, is the semantic
authority. Read-only comparison used Bochs 2.6 `cpu/fetchdecode.cc`,
`cpu/resolver.cc`, `cpu/stack16.cc`, `cpu/stack32.cc`, and `cpu/string.cc`,
and PCjs 2.00.0 `machines/pcx86/modules/v2/cpux86.js`, `x86mods.js`,
`x86ops.js`, and `x86help.js`. No reference source was copied.

The references agree with the frozen matrix in the
[T302 admission record](../etc/evidence/t302-operand-address-stack-admission.md):
operand size, address size, and SS stack-address size are independent;
32-bit SIB/default-segment rules and per-iteration string state follow the
80386 rules; and a failing current REP iteration does not commit that
iteration's index, count, or destination. Control transfer, descriptor/system,
exception delivery, privilege, paging, and virtual-8086 remain outside T302.

## Corrections And Sweep

S2 corrected 16-bit CS instruction-byte wrapping in `_kdf_skip`. S3 corrected
the zero-byte full-limit stack-cache check and made `ENTER`'s saved-frame
pointer use operand size while retaining SS address-size behavior for the
post-push stack pointer. The S5 retained-gate sweep found that the initial
zero-byte correction skipped its start-offset limit check and regressed the
task-switch stack-limit probe. `_kma_linear_logical` now checks the start
offset first and uses a non-underflowing remaining-range test for nonzero
accesses; the task-switch probe asserts the restored `#SS(0)`, IP, and SP
boundary. S4 corrected all byte `OUTSB` paths to use `_p_outs` rather than
`_p_ins`.

The focused synthetic probe covers the S1-S4 matrix: 16/32 prefix and fetch
forms; ModRM/SIB/moffs/default segment and access-limit behavior; crossed stack
widths and frames; memory strings, typed-provider `INS`/`OUTS`, DF, source and
fixed-ES destination selection, address wrap, REP termination, and failing
current-iteration preservation. The retained address and REP probes cover the
existing real-mode address route and REPE/REPNE comparison/scan behavior.

The similar-issue sweep searched the decoder/fetch/effective-address, stack,
string/REP, segment-limit, and commit helpers in production and focused tests.
Production hits were limited to those shared-helper corrections. Return/branch
helpers are deferred to the next unnumbered Queue candidate, the 32-bit
control-transfer family; no Queue candidate was removed or admitted by T302
closure preparation.

## Verification

- `cmake --build --preset current-gcc` rebuilt
  `build/output/nxvm_0_5_0302.exe`.
- SHA-256:
  `6AF7F8B16BAEB2B74F803027D32E005CDC66D429DF2181C46B9FE8EE4734ABA0`.
- `core-machine-operand-address-smoke` emitted
  `M5:T302:OPERAND-ADDRESS-STACK:OK`.
- Retained probes emitted `M5:T287:S24:REAL-MODE-386-ADDR32:OK` and
  `M5:T292:S1:REP-STRING:OK`.
- The retained task-switch probe emitted `M5:T261:S2:TASK-SWITCH:OK`,
  `M5:T261:S3:TASK-SWITCH:CORPUS:OK`, and `M5:T261:S5:SS-CACHE:OK`.
- `cmake --build --preset current-gates-gcc` passed 51 static/governance
  targets and 131/131 CTests, including documentation governance.
- `git diff --check` passed with the S5 closure-preparation tree.

## Setup Observation

One bounded owner-supplied product observation started the T302 artifact with
the external HDD and window-display route. During its 45-second host budget,
the process remained alive but exposed no discoverable host window handle.
No guest keyboard command was sent, no guest Setup checkpoint, stdout/stderr,
or guest diagnostic was obtained, and the executor stopped the process with no
residue. This is an observation limitation, not Setup progress or a product
regression; it is not retried here.

The only deferred verification is an owner-controlled manual Setup observation
using the retained product route. It does not alter the T302 CPU conclusion,
create a repair task, or introduce media, local paths, or traces into the
repository.
