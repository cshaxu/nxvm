# T539 S22: GPR MOV/MOFFS Migration

Baseline `bf8dd127c`, clean intake. Both original files were read completely;
no source includer depends on either test. Current owns admission/acceptance.
This batch consumes two of the 98 remaining original direct CPU consumers.

## Frozen Original Cases

| File / family | Contexts | Receiver |
| --- | --- | --- |
| GPR default memory, byte/word immediate, register directions | 84: four profiles x (4+8+8), plus four directions | CPU |
| GPR immediate memory and invalid extensions | 64: four profiles x two valid forms; four x two x seven rejections | CPU |
| GPR 386 attributes | 5 | CPU |
| GPR 386 immediate-register attributes | 16: eight byte and eight dword destinations | CPU |
| GPR legacy prefixes and LOCK | 26: three profiles x six prefixes, eight LOCK forms | CPU |
| GPR segment addressing/overrides | 6; despite its old segments_and_irq name, this function has no PIC | CPU |
| GPR protected limit / actual IRQ | 2 + 2 | Board |
| MOFFS defaults | 16: four profiles x four opcodes | CPU |
| MOFFS combined / single attributes | 4 + 12 | CPU |
| MOFFS rejected prefixes / LOCK | 24 + 4 | CPU |
| MOFFS segment reads / writes | 4 + 4 | CPU |
| MOFFS protected limit / actual IRQ | 2 + 2 | Board |

Total: 277 original contexts; 201 GPR and 68 MOFFS chip cases, eight board
cases. Counts describe original loop iterations, not complete instruction-set
coverage. Retain every original instruction byte, expected register/memory
value, negative-form result and flag/nonparticipant assertion.

## Boundary Design

Reuse the CPU-owned instruction fixture from S21, enlarging its bounded backing
array to cover the original MOFFS physical address 10000h without modulo aliases.
Instruction setup and hidden-cache fault controls belong to that owner. No
new public test bridge or product callback is needed.

Board cases retain actual PIC wiring/ISR/IRR/stack frames and protected-mode
terminal DF plus unchanged memory/registers. Use existing public register
patches and copied snapshots. As in S21, stop guest protected setup before HLT
rather than clearing a private halt flag; record this fixture mechanism change.
Do not replace a terminal DF expectation with a simpler chip GP expectation.

The following sections record implementation and verification; Current remains
the acceptance authority.

## Intake Implementation Finding

The public machine memory read accepts stopped/paused, not FAULTED state.
Protected-limit tests must inspect unchanged 3010h after terminal DF. Retain
their original board-owned physical-memory read for that assertion; it calls
the existing memory operation on board storage, not CPU internals. Other board
reads use the public machine operation. Do not add a test-only public interface,
reset the machine or weaken the unchanged-memory assertion to pass migration.

## Completed Implementation

Two CPU-only targets link solely to x86-cpu and reuse the CPU-owned bounded
memory fixture. Its array is now 128 KiB, covering 10000h plus the original
dword, and one instruction-run helper removes identical mechanics from the two
new tests. The original chip-state setup, hidden IDTR terminal-UD cases, GPR
nonparticipant checks, segment selection and memory results remain CPU-owned.
No original instruction table or production CPU implementation is rewritten.

Both board files use public register patches and copied CPU snapshots. They
retain two protected-limit and two real IRQ cases each. Guest GDT/LMSW/far-jump
setup executes its original ten instructions and stops before HLT instead of
executing HLT then privately clearing its flag. The measured instruction still
starts at protected CS:0 and must produce terminal DF, unchanged GPR/FLAGS and
unchanged physical memory. Real IRQ cases retain PIC ISR/IRR and stack-frame
IP=4 (GPR MOV) or IP=3 (MOFFS), plus data-transfer results.

The existing boundary gate now covers four migrated board tests, with sixteen
negative controls (93 CPU/board controls overall). T332 explicitly classifies
four public-operation board setups; T344 retains exact membership with 100
inventoried constructors, 107 including timing/table/INTA constructors, and no
blanket exemption. T337 follows terminal UD coverage to the CPU-owned tests.

The similar-issue sweep covers both original files and their includers, using
executor_cpu, core_machine_cpu_fixture, private CPU includes, executor_memory
and table/loop comparisons. There are no source includers. CPU-private hits
are zero in both board files; the two existing post-DF RAM reads are explicitly
retained for the boundary reason above. All other original direct CPU consumers
remain assigned to S23-S37. A structural comparison of all 17 original test
families confirms identical constant-array contents and loop headers in their
receivers, excluding only the two removed setup-HLT arrays. Direct assertion
review additionally checks expected register/memory values and fault masks.

## Verification

- Both existing full build trees succeed. Complete unit suites pass 375/375 on
  x64 (25.44 seconds) and x86 (25.69 seconds); runs do not overlap desktop use.
- All 66 specialized steps pass, including the 374-row strict-compilation matrix
  (341 strict, 33 unchanged deferred production rows). Initial compile checking
  caught and corrected mechanical bit-mask translations; no expectation was
  relaxed. Initial board runs exposed the FAULTED memory-read admission described
  above, repaired by retaining the original memory-owner read.
- Six unchanged Shared manifests, documentation and staged diff checks pass.
- This is test/build-registration-only work: no production source or EXE link
  input changes. Existing eight 0539 EXEs, owner INIs, Shared and MyNES are
  unchanged. No new external integration or EXE rebuild is claimed.
- Existing three build trees and the S18 recovery patch remain needed for the
  subsequent CPU batches. No new temporary tree or raw trace was created.

Ten test/build paths add 1,040 and remove 1,174 lines (net -134), excluding
four task documents. No production source or shared corpus file changes.

Executor review completes the entire two-file batch; coordinator actual-commit
review and closure remain separate. This does not accept CPU extraction.

## Coordinator Review

Reviewed actual pushed P1 `9e5382872`: receiving CPU tests, retained board
assertions, fixture, registrations, exact gate membership and task records.
All original matrices have receivers; post-DF physical-memory inspection stays
with its board owner and no private CPU access remains in the two board tests.
The 16-field packet, target scope, ordered work plan, remaining-consumer map
and no-EXE-change decision agree with the actual commit. S22 is accepted;
S23 is next. T539 and whole CPU extraction remain open.
