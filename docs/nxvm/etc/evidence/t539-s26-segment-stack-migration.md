# T539 S26 Segment Stack Migration

## Delivery Boundary

Baseline: 430abfc54. Executor delivery is complete and awaits coordinator
actual-commit review. Current owns admission and acceptance.
Production, Shared corpus, MyNES, owner INIs and deployed EXEs are unchanged.

The two original sources contain 164 contexts:

| Original suite/group | Contexts | Receiving obligation |
| --- | ---: | --- |
| Legacy LOCK rejection | 7 | CPU full rollback and explicit terminal UD |
| Legacy default segment PUSH/POP | 28 | Four CPU profiles, exact selector/cache assertions |
| Legacy attributes | 84 | 21 supported and 63 legacy rejection contexts |
| Legacy protected POP | 3 | Target and non-target cache plus accessed-bit write |
| Legacy null ES/DS | 2 | Invalid cache publication, no other cache change |
| Legacy null SS | 1 | Fault rollback |
| Legacy protected rejection | 9 | Three targets by three rejected selectors |
| Legacy stack limits | 2 | Expand-down/normal limit and stack-memory rollback |
| Legacy IRQ | 4 | Original full caches at CPU owner and actual PIC/frame/SS-shadow board proof |
| Legacy FS/GS matrix | 4 | Preserve despite overlap with the separate FS/GS suite |
| FS/GS real mode | 8 | Four operations by two widths |
| FS/GS 286 rejection | 8 | Four operations by two prefixes; terminal UD |
| FS/GS protected POP | 2 | Original deliberately inconsistent selector/cache preparation |
| FS/GS stack fault | 2 | Original selector/FLAGS/ESP rollback and terminal DF |

## Implemented Receivers

`cpu_fs_gs_stack_smoke.c` retains the separate FS/GS suite's 20 original
contexts, instruction tables and expectations. It uses the existing CPU-local
memory/diagnostic fixture and links only x86-cpu. The original bootstrap's
temporary HLT is replaced by its ten setup instructions and a checked CS:IP
boundary, avoiding a board wait-state dependency. There is no new production
operation or state mirror.

The original board file now tests two complementary stack-fault receivers
through public construction, register patches and copied snapshots. It retains
machine terminal-fault status, DF, unchanged IP/ESP/FLAGS and target selector.
Its valid FS=10h and GS=18h are deliberately different from the original
private, inconsistent FS=1111h/GS=2222h preparation; the latter remains intact
in the CPU test. These are additional board composition contexts, not a claim
that a public API can manufacture the same invalid cache state.

`cpu_legacy_sreg_stack_smoke.c` retains all 144 legacy contexts. Its six GPR,
target and segment-cache comparison functions are byte-for-byte unchanged.
All 20 original static array tables are unchanged. The separate FS/GS receiver
retains seven of eight original tables; its one removed array is the temporary
bootstrap HLT, not an instruction-under-test. Real IRQ handler HLT remains.

The four CPU IRQ contexts use the existing interrupt-pending/acknowledge bus
callbacks and preserve every private cache predicate, frame IP, stack image and
register assertion. The file-local fixture owns just its interrupt input and
acknowledgement count; it does not emulate PIC. Its memory callbacks adapt the
fixture context to the existing bounded CPU memory provider.

The legacy board receiver retains 16 complementary contexts: null SS, nine
rejected selectors, two stack-limit faults and four real PIC IRQ cases. Public
guest GDT preparation and paused register operations replace private mutation.
The protected bootstrap stops after ten instructions, before the tested opcode;
invalid stack-limit preparation occurs at that paused boundary. PIC vector,
IRR/ISR acknowledgement, frame IP (2 for POP SS, 1 for the other forms), stack
image and copied register/cache assertions remain. The complete private caches
remain tested by the CPU receiver; no flagValid/sregtype public accessor is added.

Thus all 164 original contexts remain, with 164 CPU plus 18 board executions.
Both original suites' terminal UD belongs to their CPU targets. The board
targets remain registered rather than retiring machine-fault/PIC obligations.

## Verification

- Complete builds and unit suites pass on both widths: 382/382, x64 30.03s
  and x86 40.24s, with native desktop suites executed sequentially. The earlier
  x64 pass took 212.78s; the final pass includes the bounded IRQ-frame read.
- The two CPU targets link only x86-cpu and compile with strict warnings;
  164 CPU and 18 complementary board contexts pass per width.
- The boundary gate now covers eleven migrated board files and four rejected
  private-access/import mutations each (44 controls). Both S26 files are included.
  Complete unit runs execute these controls, not just build their targets.
- All 66 specialized gates pass; the direct compilation matrix is 381 rows,
  348 retained strict and 33 deferred. T344 has 107 inventoried constructors,
  114 total, 21 shared tails and 86 explicit shapes.
- No source includes either original S26 C file; the receiving CMake entries
  and T337 terminal-UD registration were reviewed. Both suites' terminal UD
  now belongs to their CPU targets, not the board fault receivers.
- Six unchanged manifests pass. Documentation governance and staged diff
  whitespace checks pass. Source/build review confirms no production, asset,
  Shared corpus or MyNES change. Existing 0539 executable inputs remain current.
- The first restricted Ninja attempt could not start compiler children. Only
  its verified S26 CMake/Ninja processes were stopped, then the same tree was
  built outside that restriction. Unrelated sibling build processes were left
  untouched. All owned verification process handles are terminal.

The existing build trees and recovery patch are retained for later CPU packages.
The private-consumer search has 90 hits: 89 original consumers assigned S27-S37
and the boundary-negative verifier's intentional rejected-code strings. Neither
S26 board test remains a private CPU consumer; no C-source includers were found.
This result does not accept the CPU ledger row or complete T539.

## Actual-Diff And Simplicity Review

Counted paths are the four NXVM CMake files, four old/new test sources and the
CPU boundary-negative test: nine source/test/build paths. Staged Git numstat
against 430abfc54 gives +1,107/-783, net +324, excluding documentation and
artifacts. The increase preserves both complete private CPU assertions and
real-board IRQ/fault receivers; it is test ownership separation, not a new
runtime mechanism. Original mixed test paths no longer access private CPU state.

Self-review checked all original loops/constant tables, six unchanged cache
helpers, successful and rejected forms, stack memory, real PIC acknowledgement,
public descriptor construction, all registrations and negative controls. The
two FS/GS public-board initial states are explicitly distinguished above.
There is one unchanged production CPU implementation. CPU-owned test sources
remain temporarily App-located and are assigned physical relocation in S39;
the mixed legacy fixture remains only for the named S27-S37 consumers.

The original owner request is met for this bounded package without claiming
the later opaque lifetime or Shared CPU relocation. Coordinator review and
the governance closure P remain required after the implementation P is pushed.

## Coordinator Acceptance

Actual implementation commit 587a91af9 was reviewed in the coordinator role:
all four test sources, registrations, lifecycle/boundary gates, case inventory
and delivery documents agree with the packet. The two copied snapshot board
receivers do not substitute for the original private-cache CPU assertions.
Latest retained CTest logs each contain 382 passes and zero failures; older
LastTestsFailed logs predate those complete runs and are not current failures.
No production, Shared, MyNES or artifact paths are present in the commit.
The 164-context batch is accepted; the remaining 89 consumers, opaque lifetime
and physical relocation remain assigned to S27-S40. Current records closure.
