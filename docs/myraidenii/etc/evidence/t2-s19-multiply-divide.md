# T2 S19 Native Multiply And Divide

Baseline `d543b5082`, after accepted S18. V07's batch is F6/F7 unsigned/signed
byte/word multiply and divide, plus signed immediate 69/6B word multiplication.
It includes zero/overflow vector-0 entry and actual IRET return, not general
interrupt/mode/PSW/physical-clock or full V30/Machine/board/game qualification.

Shared P1 `a14736a15` and NXVM receiving-artifact P2 `b60a44c6d` are pushed.
The product-local evidence P retains the complete admission packet before
coordinator actual-change acceptance. No MyNES P is necessary.

## Primary Facts And Reproductions

The T1-pinned NEC V20/V30 October 1986 manual remains the source, SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`.
Printed 12-50--12-60 define the native multiplication/division forms and flags.
Rendered PDF 103/105 (printed 12-58/12-60), not OCR alone, confirms the prose
lower signed quotient bounds -127 and -32767, repeatedly across register and
memory forms. The malformed comparison typography in the operation diagrams
is not used as an executable algorithm; the repeated explicit range prose
owns this implementation. It is documented conformance, not a silicon probe.
Section 3 defines PC as the next instruction address; section 6 classifies
DIV/DIVU as software interrupts and describes saving PC. Source/model grades
and old native bus-tail limits are not upgraded by these synthetic tests.

The old source accepts -128/1 as an ordinary signed byte result. The new native
range witness instead requires vector 0; its before log records success at
0102 instead of entry at 9000:0200. The analogous word -32768 boundary and
surrounding values are in the same matrix. Intel retains the most-negative
quotient as valid; the existing Intel integer_multiply_divide unit explicitly
covers byte/word/dword limits and protected/real, register/memory variants.

Separately, unsigned division by zero enters vector 0 but pushes 0100 instead
of the following PC 0102. The before log records the exact saved word. The
common entry rollback incorrectly chooses the Intel 186 restart-PC rule merely
because V30 supports the 186-compatible integer forms. Instruction membership
and native software-divide return position are different contracts.

## Repair Ownership And Similar-Issue Sweep

The existing _a_idiv byte/word bounds now distinguish V30's documented range
from Intel's. Existing unsigned arithmetic, successful signed result/remainder,
wide overflow protection and 32-bit Intel paths are unchanged. The common
ExecFinal DE branch now uses the following PC for V30 while preserving every
Intel selection and other exception. No helper, new state, public function,
callback, dispatcher, frame mechanism or alternate instruction path is added.
Production is +8/-3/net +5 in one instruction unit, excluding its manifest.

All _a_mul/_a_imul/_a_div/_a_idiv/_a_imul3, F6/F7 and 69/6B callers,
ExecFinal and the Intel integer/legacy and V30 admission receivers were swept:

```text
rg -n '_a_(mul|imul|div|idiv)|IMUL_R32|VCPUINS_EXCEPT_DE' src/x86-core/cpu/cpu_instructions.c
rg -n -i 'idiv|signed division|signed divide' test/x86-core/cpu
```

Multiply results/defined CY,V and immediate sign extension already agree with
the independent matrix. No speculative multiply rewrite is warranted. Intel
32-bit and protected entry retain their existing distinct obligations.
Non-ten AAM is outside NEC's documented form; it still uses the same DE
owner, not a second fallback, but is not certified by this batch. Packed
decimal ambiguities, generalized entry/mode/PSW, prefixes/LOCK and timing
retain the original ledger receivers.

## Finite Result, Transfer And Failure Proof

The single CPU-owned unit uses the existing actual one-MiB storage fixture
and S17 restart, not a modulo provider or copied production interpreter.
Wide arithmetic computes the complete expected CPU before result aliases are
written. A typed transfer plan independently specifies operand reads, native
odd/even words, PSW/CS/next-PC writes and vector reads. It checks every physical
address/width/order/BEGIN/COMMIT/CANCEL and each final touched byte. Successful
and failed execution have separate complete CPU/flag obligations.

The 1,796,404 successful input cases (including expected guest overflow/zero
interrupts) comprise:

- Two explicit native signed-minimum entry/return witnesses.
- 84,480 form/context cases: all relevant ModRM, both widths, five absent/
  segment-prefix selections, eleven value seeds and two flag seeds; both
  immediate forms include every destination/source alias.
- 131,072 byte multiplication cases, every operand pair for signed/unsigned.
- 1,048,576 byte division cases, every sixteen-bit dividend against eight
  zero/sign/limit divisor values for signed/unsigned operations.
- 7,986 word boundary triples across six forms.
- 524,288 immediate cases: every sixteen-bit source against four imm8 sign
  boundaries, and every imm16 against four source boundaries.

Every form/context case also tests pure preview and refuses each applicable
data read/write ordinal. Host refusal must stop and restore complete CPU with
no guest entry/shadow while retaining exactly its accepted stack-write prefix;
no retry or replay is introduced. Overflow success checks the exact vector,
six-byte frame, cleared IE with BRK remaining clear, and real IRET continuation
to next PC. Input BRK is zero; step/BRK ordering is not qualified here.
Arithmetic undefined flags alone are masked according to original tables.
Undefined overflow quotient/remainder retain the old rollback model; their
retention is not claimed as a specified NEC result. General asynchronous
priority and emulation entry are not covered by this vector-0 receiver.

A focused fixture correction initialized both bytes of each wrapped stack
word rather than wrapping every byte offset independently. This honors the
already recorded S8 contiguous word-tail L2 model, including SP=1, and avoids
stale high bytes between independent failure attempts. It changes neither the
CPU nor the shared fixture. The complete focused x64 matrix passes in 3.90
seconds, well inside its pre-admitted 90-second finite-matrix containment.
The initial source failures and fixture diagnosis are retained, not presented
as passing baseline runs or reasons to weaken assertions.

## Qualification And Receiving Releases

Both complete builds and first full unit suites pass: 669/669 per width,
including 182 X86 Core units, in 317.99/132.20 seconds. The new unit takes
3.72185/4.50255 seconds in those suites. Both specialized targets pass all 498
strict rows. Types, source/test manifests, ownership, inward dependencies,
negative verifiers, product governance and whitespace pass; existing warning
exceptions are retained, not claimed as removed. No full-suite retry or
existing timeout adjustment is needed.

Tests/registration are +359/-0/net +359 (357-line unit and two CMake lines),
production +8/-3/net +5: combined +367/-3/net +364 exclude manifests,
documents and artifacts. This increases independent NEC result and failure
proof, not runtime abstraction or duplicate execution. Initial estimates
were source +5..25/-3..15 and tests +330..480/-0.

All eight retained-revision PC EXEs are rebuilt/deployed at 0564. Four profile
groups pass 14 receiving integrations per width (default five, other Apps three
each): actual App INI identity, adjacent INI status and boot matrix, plus default
debug pause and lifecycle. DeskPro boot completes within the unchanged budget,
in complete group wall times 131.94/177.47 seconds. After each width returns to
the default graph, all targets/current EXE are relinked as needed and one final
default identity integration passes. That is 28 profile checks plus two distinct
post-restoration checks, not a claimed fresh complete integration suite.
[Results](t2-s19-results.json) retain names and exact counts.

Fresh [artifact identities](t2-s19-artifacts.json) verify all ten PE/name/width/
size/hash records. Eight PC hashes reflect this frozen CPU instruction source
`6DFE1226FDDB5C8A577D8F93289E312307AB43CE59809788530A8310739BFA8D`;
the two MyNES 0044 hashes match the accepted T564 baseline exactly. Per-profile
build logs show no repeated shared CPU object compilation, and both original
graphs are restored; no per-App build tree or compiler-launcher change exists.

The release-only local helper first
requested a nonexistent build-integration-tests target; Ninja rejected it before
any receiving test ran. It was corrected to the repository's actual all and
current artifact targets. No production/build graph, test, timeout or assertion
was changed to work around this command-setup mistake.

MyNES has no X86 Core dependency: its two accepted
hashes, all user INIs, media, snapshots and firmware remain unchanged.
All builds reuse the original Release Ninja/ccache graphs and persistent cache;
shared CPU libraries are not regenerated in separate per-App build trees.

This S does not create a MyRaidenII EXE or qualify a game path. No new physical
audio, RDP, manual desktop, silicon timing or complete whole-repo integration
claim follows from CPU vectors and selected automated receiving integrations.

## Coordinator Acceptance

Shared `a14736a15`, NXVM `b60a44c6d` and evidence `2c3b2cc3e` are accepted
after single-session actual-change review. The complete original request and
sixteen-field packet, full frozen V07 form/value/failure batch and bounded
V11 receiver, primary prose versus undefined/model facts, sole production
owners, actual complete test/fixture/registration/manifest, full unit and
selected integration logs, source hash, all ten release identities and
explicit residuals agree. Source/Types/dependency, no-new-state/API/path,
target-scoped P and unchanged protected-input invariants hold. Initial
failures and local harness corrections are retained, not broadened into
unverified runtime or silicon claims. The final governance P closes S19;
T2 and the full original MyRaidenII outcome remain open.
