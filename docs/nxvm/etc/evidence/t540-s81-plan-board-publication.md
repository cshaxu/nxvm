# T540 S81 Frozen-Plan Board Handle Publication

## Scope And Confirmation

Baseline is accepted S80 P2 `0c168e618`. The executor confirms CURRENT's
S81 packet. This NXVM-only receiver consumes the complete frozen-plan
construction/publication class, not public board operations or physical
Shared relocation. No Shared/MyNES source, test, build, INI or media master
is changed. The project-owned implementation retains its existing provenance
and the owner-approved embedded product-artifact boundary.

## Actual Construction Boundary

The public board header declares the existing board allocation as an opaque
handle. `core_machine_create_from_plan` now requires two output locations,
clears both supplied locations before validation, constructs into local
candidates and publishes both only after topology and timing declarations
succeed. On failure it destroys the unpublished candidate through the sole
Core destructor. No getter, registry, second allocation or board destructor
is introduced. The board remains the callback context accepted in S80.

The existing candidate factory becomes callable from the same board module's
plan source and accepts an optional private output. It is still the unique
allocation implementation used by the public configuration constructor and
both existing allocation-failure seams. Those Core-only fixture interfaces
are retained unchanged, with their public-board caller migration explicitly
assigned to the subsequent owner receiver. This is not a new compatibility
factory or a public private-state accessor.

The NXVM driver retains a borrowed board handle beside its Core owner. Its
one destruction site clears both handles before releasing display/media/plan
resources. Core alone releases the attachment and chips. Board APIs still
take Core in this baseline; their next complete migration can now use the
actual construction output rather than retrieve Core's private association.

## Complete Caller And Similar-Issue Sweep

Search all tracked NXVM source/test files using
`rg -n 'core_machine_create_from_plan\(' src/app-nxvm test/app-nxvm` and
`rg -n 'core_machine_create_internal\(' src/app-nxvm test/app-nxvm`.
The public declaration, sole definition and every call are accounted for:
one production driver and four test files (plan, XT topology, timing
qualification and project-owned floppy firmware). All existing calls supply
the new output. The current inventory has 26 calls plus one declaration and
one definition, including five new publication test calls.

All board field assertions in those plan-created fixtures now use their
actual board output instead of `machine->board`. Core-owned execution, port,
time and reset checks keep the Core handle. These tests remain NXVM-local:
this receiver does not misclassify product topology or firmware as Shared.
Ordinary configuration-only tests, allocation seams, remaining board methods
and chip-wiring callbacks retain their documented next receiver; no new
private association route is created. The same-named MyNES constructor is a
different product implementation and is excluded, not edited or built.

## Verification And Size

The existing plan test adds success/context/Core identity, null plan,
missing output, post-allocation topology rollback and three injected port
allocation failures. Supplied outputs must be null after each rejection.
It emits `M5:T540:S81:BOARD-HANDLE-PUBLICATION:OK`. Existing test entries and
all other timing/boot behavior remain unchanged.

The existing NXVM controller-owner gate requires the dual publication and
driver invalidation and rejects a getter or the old private publication
bridge. An ignored copied-source probe verifies its positive case, the
previous seven owner violations and both new publication/invalidation
negative cases; all nine are rejected for their intended reason. The probe
validates its location below build and removes its synthetic source copy.

Counted tracked source/test paths: ten, using `git diff --numstat` over
`src/app-nxvm` and `test/app-nxvm`. Added 139, removed 52, net +87 lines.
The principal increase is the 52-line owned failure/publication regression;
production adds 38/removes 16 lines. The existing single factory is shared
directly, not wrapped; the only new retained runtime state is a borrowed
handle, not a copy of board state. The NXVM gate adds 18/removes one line.
The existing negative-test registration changes three lines in/three out:
its copied-input comment and containment budget, not its checks.

Final complete units, gates and artifact identities are recorded below.
All eight product builds, independent Core executions and one-shot boots pass.
The complete implementation self-review passes; pushed-diff coordinator
acceptance follows P1 rather than being inferred from these checks.

The first x64 full run completes 469 tests but times out the existing CPU-bus
negative contract at 180.14 seconds (full run 437.31 seconds). It completed
72 CPU and five board controls before timeout; this is not accepted as a pass
or used to omit its remaining checks. Read-only process inspection shows
concurrent sibling builds, which are not terminated. The coordinator revises
the packet before changing this NXVM-only test containment to 600 seconds,
preserving every assertion and RUN_SERIAL ordering. Its stale comment is
corrected: the test mutates owned copies, not tracked production headers.
Final complete units must run again under the registered budget, not a
selected passing rerun. The initial failure log remains retained.

The final x64 complete unit run passes 470/470 in 167.05 seconds, including
the complete CPU negative contract in 147.78 seconds. Its full specialized
gate target passes. The final x86 run passes 470/470 in 85.13 seconds,
including the complete CPU negative contract in 54.38 seconds; its full
specialized gate target also passes. Both gate targets include the 402-row
strict-compilation inventory (377 retained strict, 25 explicitly deferred).
Documentation governance, 466 changed-document local references and diff
whitespace checks pass. Every profile/width row runs exactly once with the
existing real-INI matrix and 180000 ms containment: default x64/x86 reach
`dos-prompt`; XT, AT and Model40 x64/x86 reach `installer-running`.
All eight independent Core executions emit `M5:T540:S69:NEUTRAL-LINK:OK`.
No timeout or repetition is used as a boot success. These finite checkpoints
do not prove indefinite absence of intermittent faults or close T540.

No unbounded trace is produced. Incremental trees and compact S81 logs are
needed by the immediate public-board-operation receiver; unrelated products
and owned external assets are not cleanup targets.

## Product Artifact Identity

The eight existing optimized Release products are rebuilt with unchanged
approved embedded BYOB inputs. PE architecture, absence of `.debug` sections,
`0.5.0540` banner and freshness against the changed board inputs pass.
No protected original or new generated firmware source is imported. MyNES
artifacts and owner INIs have no tracked delta. Compact S81 build/unit/gate,
neutral/boot and artifact logs remain under the ignored build directory.

| Product path | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336056 | `866379897EFEA762DA42496F0565FCCF48FE8BE7E3E8A1FB48B65C73BC0D1DF7` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1506888 | `E8037F419C47B998D96E866E21049C71BEE7768CDE65197968703278FA5160E0` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352373 | `2186C7D3648CF96373541A40F2444148F7088711B45D4F0579EE14AD54C2071C` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1523203 | `9CEF00C7B896B5186A041C0DF08F5FDFD3160D8AA73ECBA679DCFEEAF7DF2686` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352341 | `33E043B417DA9E88678BC124A2A6BD3D776FDE6B1415AD1DDFD3A704D3A02586` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1523170 | `63A74224316A83FF4B064A3D69BBDAF5A39FE1A40993A2D96F80559247E4B281` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352407 | `613D6D8305C9044745100B97AC1A3D7181F85035289F7667D88598903D7B57EC` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1523238 | `3F284553515FB62EAD7DAF8FEE14B08351ED74F1CD336DDFE30A8EAF92E1B4A2` |

## Coordinator Acceptance

P1 `c01ee5edeb6c66a7c12ab56451c069e12d0e8e07` was immediately pushed to
origin/master. After switching roles, the coordinator reviews the actual
committed hunks in all ten source/test, two build/gate and six documentation
paths, then verifies all eight artifact blobs against the checked files.
All 26 paths belong to NXVM. Publication, caller completeness, failure/lifetime
ownership, unchanged algorithms/timing grades, Types vocabulary, linear task
identifiers and packet-to-proof mapping pass. No corrective implementation P
is required. Governance P2 accepts S81 only, removes its active packet and
leaves T540's public board operations, configuration-only fixtures, direct-test
classification and physical relocation open. Incremental trees remain needed
by that receiver; Shared/MyNES and asset masters are not cleanup targets.
