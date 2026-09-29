# T539 S27 Far-Pointer Load Migration

## Boundary And Case Map

Baseline: S26 P2 9a4de7adf. Current owns admission and acceptance. This batch
changes NXVM-owned tests/build gates only; no production, public ABI, Shared
corpus, MyNES, firmware, INI or executable inputs change.

The original three files contain 117 executed contexts. Overlapping matrices
remain separate; no deduplication removes original coverage.

| Original suite/group | Contexts | Receiver |
| --- | ---: | --- |
| LES/LDS S41 real mode | 26 | CPU: eight profile forms, twelve segment-prefix forms, six attribute forms |
| LES/LDS S41 rejection | 28 | CPU: eighteen legacy-prefix, eight register-direct, two LOCK |
| LES/LDS S41 protected | 10 | CPU: valid, null, not-present, executable-only and privilege rejection per opcode; six fault contexts also have board receivers |
| LES/LDS S41 source limit | 2 | CPU private-cache rollback plus public-descriptor board fault |
| LES/LDS S41 IRQ | 2 | Board: actual PIC, complete original GPR assertions and frame IP=4 |
| LES/LDS real mode | 8 | CPU: three profiles, with 32-bit operand only on 386 |
| LES/LDS register-direct rejection | 2 | CPU terminal UD |
| LES/LDS legacy operand32 rejection | 4 | CPU terminal UD |
| LES/LDS protected | 4 | CPU: both widths and both operations |
| LES/LDS source fault | 2 | CPU private initial state plus public-descriptor board fault |
| LES/LDS IRQ | 2 | Board: actual PIC and no SS shadow, frame IP=4 |
| LSS/LFS/LGS real mode | 6 | CPU: three operations by two widths |
| LSS/LFS/LGS register-direct rejection | 3 | CPU terminal UD |
| LSS/LFS/LGS 286 memory rejection | 6 | CPU terminal UD |
| LSS/LFS/LGS protected | 6 | CPU: three operations by two widths |
| LSS/LFS/LGS source fault | 3 | CPU private initial state plus public-descriptor board fault |
| LSS/LFS/LGS IRQ | 3 | Board: actual PIC, frame IP=6 for LSS versus 5 for LFS/LGS |

There are 110 CPU executions and 20 board executions; thirteen fault contexts
have complementary receivers. This is 117 original contexts, not 130 distinct
original cases. IRQ assertions require only copied architectural values, so
their sole receiver remains the real board rather than adding a fake PIC.

## Implementation Review

Three CPU targets link only x86-cpu and reuse the existing bounded instruction
fixture. They retain original instruction tables, loops, private selector/cache
preconditions and complete assertions. CPU handlers and timing formulas are
unchanged. Protected bootstrap runs ten instructions (eleven for S41), then
checks CS=8 and IP=0 instead of entering a temporary HLT and clearing it through
a board-private fixture. The guest instructions under test are unchanged.

Static-array comparison retains 43 of 45 original array declarations verbatim
across their receivers. The only omitted arrays are the two temporary bootstrap
HLTs; actual IRQ handlers still execute HLT. S41's scalar bootstrap HLT is also
unneeded; its identical IRQ-handler HLT remains. Source writes, accessed-bit
checks, complete cache comparisons, register rollback and source-byte invariance
stay in the CPU receiver. No new public cache accessor is introduced.

Board receivers construct/freeze/reset through existing operations, execute
real GDT setup and use paused register patches plus copied snapshots. Their
source-limit faults alter the descriptor bytes and reload DS via that public
operation. Deliberately inconsistent selectors/cache pairs remain CPU-local;
complementary board tests use valid selectors (DS/FS=10h, GS=18h where needed).
S41 retains ES=18h for LES and ES=10h for LDS to match its original initial
target selection. These board preparations do not pretend that a public API
can create the same inconsistent private state.

The original LES/LDS real-mode loop allocated a machine before continuing past
unsupported width/profile combinations, leaking four skipped constructions.
The receiving loop now skips before fixture preparation. Other two loops were
checked: neither has the same skipped-construction path. No valid case is lost.

Terminal-UD ownership moves to the CPU targets in T337. Original board targets
remain registered for machine faults and PIC composition. Boundary negatives
cover fourteen migrated board files, four mutations each (56 rejected controls).
The three originals have no C-source includers. Private-consumer search has
87 hits: 86 original consumers assigned S28-S37 plus the deliberate negative
fixture strings. Physical CPU test relocation remains assigned to S39.

## Verification And Delivery

Full x64/x86 builds and all 66 specialized gates pass. The direct compile
matrix has 384 rows: 351 strict, 33 deferred. T344 inventories 110 constructors,
117 total, 21 shared tails and 89 explicit shapes. Six unchanged manifests pass.
Complete unit suites pass 385/385 on x64 (31.92 seconds) and x86 (30.54
seconds). After the x64 full run, two indentation-only lines changed; its
incremental build and all three CPU receivers were rerun successfully. The
x86 full run includes that final source. Native desktop suites ran sequentially.
Documentation governance and diff checks pass. All owned verification handles
are terminal. This executor delivery awaits coordinator actual-commit review;
it does not itself close S27 or the CPU ledger.

Git numstat counts eleven test/build paths, excluding documentation/artifacts:
+1,294/-1,017, net +277. The positive test-only delta retains both private CPU
assertions and board fault/PIC obligations while removing board-private CPU
access. No parallel production state, execution route or helper API is added.
Existing 0539 EXEs remain current because their executable inputs are unchanged.
Retain the existing build trees/recovery patch for later CPU work packages.

## Coordinator Acceptance

Actual-commit review accepts NXVM P1 42d6c86e0. Review compared the three
original matrices with the six receivers, including protected selector/cache
preconditions, original guest bytes, rollback assertions, IRQ acknowledgement
and the LSS versus LFS/LGS shadow distinction. It checked target linkage,
terminal-UD registration, constructor classification and all boundary mutations.
The migration removes private CPU access from these three board files without
changing production behavior or using a new public test interface. Build/unit,
specialized-gate and six-manifest proof meet this packet; documentation and
scope review confirm no Shared, MyNES or artifact-input changes. S27 closes.
S28-S40 and the whole CPU/T539 acceptance remain pending.
