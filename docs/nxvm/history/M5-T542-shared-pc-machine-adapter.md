# T542: Remaining Shared PC Machine Logic

## Admission And Baseline

The owner requests a new implementation T for remaining shared logic and its
S breakdown on 2026-10-04. Allocate M5 T542 from closed T541 at
55f9fb1920af9316c6102b7aff5550fe1d7b6dfc. Initial worktree: clean.
The [proposal](../proposals/m5-shared-pc-machine-adapter.md) defines scope;
[Current](../states/CURRENT.md) alone owns the active S packet.

The preceding read-only audit found one existing Machine implementation:
29 Machine C/H files and seven media C/H files. PowerShell Measure-Object
-Line counted 2,294 and 802 nonempty lines respectively; these are inventory
sizes, not predicted savings. Model40-specific state and all-profile plan
construction prevent a blind directory move.

## S1: Inventory And Contract Design

Design delivery prepared; coordinator acceptance follows the complete P.
The receiver ledger and migration contracts are frozen below. No source,
test, executable or INI changes have been made by this S.

## Convergence Ledger

The S1 inventory below freezes receiving owners, not delivered source. All
implementation batches remain pending until their code and regressions prove
the move. Accept a batch only when every member has a proven receiver or
justified App retention.

| Batch | Initial capability/source universe | Proposed owner | Disposition |
| --- | --- | --- | --- |
| Helpers | profiles/device/floppy; profiles/byob/blob; profile_contract and callers | x86/ibmpc-common | Receiver frozen; S2 implementation pending |
| Media | machine/media FDD/HDD providers, geometry, marks, leases and callers | x86/product/machine/media | Receiver frozen; S3 pending |
| Input/display | keyboard_mapper, mouse_mapper, machine ingress, display/frame carriers | x86/product/machine | Receiver frozen; S4 pending |
| Execution/debug | runner/waiting/control/executor_state/lifecycle/fault/debug/debug_adapter and driver | x86/product/machine | Receiver frozen; S5 pending |
| Construction | machine create/destroy/storage; profiles/machine_plan; product/config; fixed build bindings | Shared mechanism plus fixed App composition | Contract frozen below; S5/S6 pending |
| Retained differences | XT/AT/default/Model40 values, ROM layout, D4, CMOS, HDC geometry and firmware source | Concrete App profile/firmware owner | Retention justified below; S6 verification pending |
| Verification/build | tests, source lists, static gates, tools, manifests and eight artifacts | Behavior owner; external integration remains NXVM | Routing frozen below; all code batches and S7/S8 pending |

Closure resolves every pending entry, proves each shared migration and
reconciles full affected batches rather than only a successful local replay.

## S1 Receiving-Owner Inventory

Inventory scope is all 36 Machine and 33 Profile files, the four Product files,
firmware source and the matching source/test/build/tool references. Search:
`rg --files src/app-nxvm/machine src/app-nxvm/profiles` and
`rg -l 'app-nxvm/(machine|profiles)' src test cmake tools CMakeLists.txt`.
At the reference baseline the latter returns 50 source, 107 test, 35 CMake
and two tool files. This is include-reference coverage, not proof of runtime
coverage; symbol searches and source-list review supplement it. No Shared C/H
file currently includes these App paths.

The rows partition every C/H file below those source roots. Paired C/H names
mean both files; private/interface variants are explicitly listed.

| Source below src/app-nxvm | Receiver/disposition | Batch |
| --- | --- | --- |
| profiles/device/floppy.c,h | ibmpc-common/floppy.c and floppy_interface.h; retain all four geometry/rate/channel behaviors | S2 |
| profiles/byob/blob.c,h | ibmpc-common/rom_validation.c and rom_validation_interface.h; 55AA/length/checksum validator only | S2 |
| profiles/profile_contract.c,profile_contract_interface.h | ibmpc-common/profile_contract.c and profile_contract_interface.h; caller-supplied catalog and constraints, no model registry | S2 |
| machine/media/fdd.c,fdd.h,fdd_private.h,hdd.c,hdd.h,hdd_private.h,media.h | product/machine/media; private per-medium state and one Lib lease | S3 |
| profiles/default_profile/keyboard_mapper.c,h; mouse_mapper.c,h | product/machine input conversion; scan set from board capability, not machine name | S4 |
| machine/frame.c,h; display.c,h; guest_display_interface.h | product/machine display; direct video snapshot to Common frame where equivalent; eliminate redundant copied carriers | S4 |
| machine/event_interface.h; guest_input_interface.h | retire redundant transport types after caller migration; only necessary copied input remains at Product Machine boundary | S4/S6 |
| machine/debug.c,h; debug_adapter.c,h | product/machine execution plan/paused request adaptation, not Debug CLI parsing | S5 |
| machine/runner.c,h; waiting.c,h; control.c,h; executor_state.c,h | product/machine bounded execution/cancel/reset signals; Common owns authoritative lifecycle and queue | S5 |
| machine/fault.c,h; lifecycle.c,h; machine_devices.c,h; machine_info.c | product/machine failure/reset/media bindings/copied observations | S5 |
| machine/machine.c,machine_interface.h,machine_private.h | product/machine opaque owner and one create/destroy path; profile-specific fields/dispatch excluded | S5/S6 |
| profiles/selection_interface.h | split copied runtime/asset values to Shared construction contract; fixed machine identity and selection remain App-owned | S6 |
| profiles/machine_plan.c,machine_plan_interface.h | common preparation to Shared adapter; fixed profile branches to corresponding App composition; remove union/dispatcher and obsolete getters | S6 |
| profiles/default_profile/pc_at_profile.c,h,pc_at_profile_private.h | retain actual default/5170 descriptors and constraints; common parameter-to-board materialization belongs ibmpc-common, not duplicated in two Apps | S6 |
| profiles/default_profile/external_pc_at_rom.c,h | retain fixed BIOS/video role and physical layout declarations; only proven common byte preparation is shared | S6 |
| profiles/xt/xt_5160_268.c,h; xt/rom/xt_5160_268_rom.c | retain XT topology, DIP/8088/Xebec configuration, ROM windows and reset semantics | S6 retention |
| profiles/model40/model40.c,h,model40_private.h; composition.c,composition_interface.h; rom/model40_rom.c | retain Compaq constraints, dual drive topology, selected WD geometry, ROM interleave/mirrors and CECG values | S6 retention |
| profiles/model40/d4_memory.c,h,d4_memory_interface.h; d4_platform.c,h,d4_platform_interface.h | retain genuine D4 memory/parity/refresh/PortB owner and callbacks | S5/S6 retention |
| product/config.c,h; main.c; version.h | retain identity/build-selected composition; eliminate common factory/value-conversion duplication; no copied Product entry | S6 |
| firmware/build.c,README.md and asm files | retain project guest BIOS source and offline tool; no host firmware service or move into runtime Shared adapter | Retained |

## Frozen Machine Construction Contract

The shared receiver is a real Product subresponsibility, not a new top-level
framework: x86/product/machine with media beneath it. Existing Product command,
composition and process entry continue to use the one frozen factory.

The final dependency direction is App fixed composition -> PC Product adapter
-> Common public machine protocol, Core public operations, IBM-PC public board
contracts and Lib services. IBM-PC helpers depend only on declared chip/Core
value contracts and Types, never Common, Product or App. Core stays neutral.

Construction has two phases, not two owners of live hardware:

1. App resolves its fixed profile and linked assets into a copied construction
   record: Core config/timing/topology, floppy drive/media kinds and count,
   HDC presence/optional fixed geometry, memory-change permission, firmware
   provider/context, glyph/CMOS configuration and one bounded board-plan
   configuration operation if genuine profile wiring requires it.
2. Shared Machine takes that prepared record, opens its media candidates,
   configures the existing opaque Core plan/registry/providers, constructs one
   Core and board, describes the existing Common driver, and publishes only
   when all preparation succeeds. No Shared caller interprets a profile enum.

A dynamically prepared profile context uses an explicit release operation in
that same construction record. On acceptance the Shared machine owns that
context's lifetime, not its internal state: Core is destroyed first, then
registries/plans/media, then profile context. Before acceptance App cleans its
own failed candidate. The interface must specify exactly where ownership
transfers so neither failure path double-frees or leaks. Compile-time immutable
firmware bytes need no new allocation/ownership facade.

The profile configuration operation consumes the existing opaque Core plan;
it installs real board wiring/firmware declarations, not lifecycle policy.
It is frozen before first reset, executes only during construction and may not
retain or mutate a running machine. Do not add callback-per-field parsing,
profile inheritance, model-name dispatch or a generic device registration API.

Shared Machine exposes an opaque handle and only the existing needed operations:
construction/destruction, Common driver description and binding, copied INFO,
speed and permitted memory change. Paused debug remains the existing bounded
Common request/response path into Core APIs. Media/input/lifecycle use Common
production entry points; owner-local deterministic fixtures can drive the same
implementation without inventing another runtime queue.

The generic adapter does not retain model40_board or Model40-named terminal
state. The fixed Compaq construction context receives the existing D4 factory
output and FDC observation sink; Core board attachment still destroys D4.
Profile observations remain copied bounded values. Tests needing these facts
hold the explicitly constructed profile context, never a new side registry or
getter exposing the generic adapter's private layout. Reset invalidates the
observer at its real owner. DOS/Model40 probe acceptance predicates survive.

## Test, Build And Tool Receivers

Code-owned, independent helper/media/input/frame/executor/debug-adapter tests
move to test/x86/{ibmpc-common,product/machine} with their source owners.
In particular profile-contract, keyboard-set1-mapper, machine-frame,
executor-state and media-provider behaviors are shared. Tests that construct
an actual selected NXVM profile stay under test/app-nxvm until the later App
split: ibm_5170, xt_5160, model40, pcat/default composition and D4/ROM fixtures.
Mixed tests are split only if their assertions really belong to two owners;
preserve assertions, do not turn profile fixtures into Shared dependencies.
Host-cancellation, pause/resume, debug-authority, initialization atomicity,
fault/runner propagation, media lifecycle, display cadence, speed and timing
tests require both independent adapter proof and retained profile integration.

All test/app-nxvm/integration subtrees (dos, hdd, model40, product, windows and
support/session_ini) remain NXVM. There are 58 existing profile/width contexts:
default 22 per width, XT one, AT three, Model40 three. Keep the CTest predicate,
external input and timeout contract, with one execution per context. Rebind
private-state observations to the actual owner; never replace them with weaker
installer/boot predicates or synthetic inputs.

Affected CMake ownership is concrete: VM_PROFILE_SOURCES, VM_MEDIA_SOURCES,
VM_MACHINE_RUNTIME_SOURCES/VM_MACHINE_SOURCES and VM_APP_SOURCES in
cmake/nxvm/NxvmProduct.cmake lose migrated members; src/x86/CMakeLists.txt owns
the one new source set and test/x86/CMakeLists.txt the independent registrations.
profile_binding.h.in and embed_firmware.cmake remain fixed App build bindings.
No source is compiled twice or forwarded through an alias implementation.
Every matching verify_*.cmake gate retains its invariant but follows its real
source owner; Shared receiver/dependency assertions belong test/x86, while D4,
ROM/profile/INI gates stay NXVM. Six manifests are rehashed only where changed.

tools/nxvm/dependency-dag-allowlist.txt follows the declared graph without hiding
a reverse edge. Verify-Model339CallbackClockLedger.ps1 remains profile-owned;
it consumes migrated public contracts rather than historical file positions.
App release/tool recipes remain four fixed targets sharing one implementation;
T542 does not introduce new App/product scope names or move deployment paths.

S2-S6 update all direct callers/build/test registrations and gates in their
own complete deliveries; no broken linking or compatibility shell is left
until S7. S7 independently audits the aggregate result. Full repository units
run on both widths for each source batch; standalone x86 tools-on/off and
all external integration contexts are task-level closure proof.

## S1 Review And Verification

Actual-source review confirmed the original risk: the Machine owner currently
depends on App plan, keyboard/mouse mappers and Model40 state. Product's
existing factory already separates control composition, so no Lib/Common API
change or additional queue is required. Common owns the worker; the runner is
bounded Core adaptation, not a parallel host executor.

The source partition above covers the enumerated 36 + 33 files and Product
and firmware boundaries. Include/symbol sweeps identify all matching test,
CMake and tool consumers; integration predicate/width inventory is retained
from the unchanged T541 baseline, not reported as a fresh runtime pass.
S1 changes only task and design documents. Runtime inputs and all deployed
artifacts remain unchanged; no compile, unit or integration execution is
claimed for this design-only delivery. Documentation gate, changed links,
whitespace and actual-diff review are required before its P delivery.

Delivery checks: NXVM documentation governance passes; all changed relative
Markdown links resolve; git diff --check passes. Actual-change review finds
only NXVM documents and no Shared/MyNES/configuration/artifact changes.
Git allocation follows closed T541; the Queue remains unnumbered and the
successor explicitly depends on T542. No rule exception is requested.

### S1 Coordinator Acceptance

Review pushed P1 e3027c982 against the owner's extraction request and S1 packet.
The complete source partition, caller/build/test routing, construction transfer
and rollback, Model40 retention and successor dependency satisfy the design
exit. Re-run documentation governance and diff checks successfully; review
all eight changed documents and verify the working tree is clean. Only NXVM
documents changed; runtime and artifact inputs remain untouched. Accept S1,
remove its active packet and retain open T542 progress for the next admission.

## S2: Shared Construction Helpers

Admitted from accepted S1 d7b92e35a. Migrate the complete Helpers batch while
preserving symbols and behaviors; independent contract assertions follow their
Shared owner. File moves use git mv, direct caller/build repairs are in this S,
and no source/test or EXE acceptance is claimed until actual verification.

### S2 Verification Commands And Owned Trees

Configure Debug root trees `build/t542-s2-unit-x64` and `build/t542-s2-unit-x86`
with Ninja, the corresponding WinLibs GCC compiler,
`REPOSITORY_BUILD_MYNES=OFF`, and `PROJECT_ARTIFACT_ARCHITECTURE=x64|x86`.
Build their default targets with `cmake --build <tree> -j 12`, then execute
`ctest --test-dir <tree> -L unit -j 12 --output-on-failure`.
Standalone helper/build proof uses `cmake -S test/x86 -B
build/t542-s2-shared-x64 -G Ninja -DCMAKE_BUILD_TYPE=Debug`, builds
`vm-profile-contract-smoke` and `x86-test-construction_helpers`, and executes
their registered tests. Static proof runs `src/x86/verify_corpus.cmake` and
the Common manifest verifier with each of the six corpus roots.

Release trees are `build/t542-s2-release-<default|xt|at|model40>-<x64|x86>`;
configure each with the fixed profile and corresponding compiler, MyNES OFF,
Release, then build only `vm-0-5-0542` (`-j 12` for default, `-j 8` for the
remaining six builds). Existing gates check Release and PE architecture;
separate name/embedded-identity/objdump checks prove stripped products. Verify hashes and
unchanged owner INIs before replacing each admitted pair. These trees remain
needed through S2 review and the next migration batch; no other build tree
or process is claimed or cleaned by this S. All commands are containment and
verification, not a runtime hardware-qualification claim.

### S2 Implementation And Verification

The complete Helpers batch is migrated: six source/header files and the
original contract test move to their Shared owner. App callers consume the
three public interfaces; only x86-ibmpc-common compiles the three production
implementations. Old helper paths have no remaining source/test/build/tool
references. Independent linkage uses no App source. Compare each original C
file against its receiver after only the declared include substitution:
all three are identical. Symbols, enum values, four media geometries/rates,
double stepping, ROM signature/length/checksum and board constraints are
unchanged. The original contract test retains all assertions and drops one
unused stdio include. A 64-line code-owned matrix test adds direct proof of
the complete helper contract; it reads no asset or INI.

Full root units pass 496/496 on each width: x64 257.42s and x86 59.55s.
Independent helper tests pass 2/2; the standalone x86 corpus, manifest,
test-manifest and negative verifier tests pass 4/4. All six corpus manifests
validate. Documentation governance and whitespace checks pass. The initial
sandboxed compiler probes remained live without compiler children; only those
three owned roots/children were terminated, and elevated configurations/builds
completed. Two ad-hoc static invocations lacked their required script/path
arguments; the registered standalone gates above were then run successfully.
These were harness invocation errors, not product/test regressions.

Actual-diff review confirms no Lib/Common, MyNES, owner INI, firmware payload
or external-master change. All original unit registrations survive; the
contract test keeps its name and adds the Shared label/independent linkage.
Rename-aware Git numstat over changed C/H source/test paths against d7b92e35a,
excluding docs/build/manifests/binaries, records 84 added, 21 removed, net +63.
The increase is the 64-line independent contract matrix minus the unused
include, not another production abstraction; production algorithms add no
lines. File movement is not counted as code deletion.

Eight Release products built with the sole vm-0-5-0542 recipe verify their PE
architecture, 0.5.0542 identity and absence of .debug_* sections. Runtime Debug
remains linked through the unchanged Product/Common driver. The old eight
0541 EXEs are removed only after replacement verification and remain recoverable
in Git; owner INIs are unchanged. This S does not claim fresh external boot
qualification; the preserved 58 integration contexts remain T-level S8 proof.

| Machine | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 197D0C56DB6DD003022EA205A0F49B6820A35D1BD87B1AB457CB69C8C6C116D7 |
| default | x86 | 7078EF9D3797D419CF1A471325540B699D2E2E29AA990AABF4237DDCE66A3963 |
| XT | x64 | 45EE28E42B65D5883429D490C9DDF6BC998D4CAC5F02409A6634D808BA913E51 |
| XT | x86 | EC7CFAAA869207688F9A9A870AAFE550D259CDC0F05D6ED90C5AA6F2B959F1DD |
| AT | x64 | 178502455F2FC28345A2A035AC89E76D160AFE30C91A697A0D3246DEA3923CA8 |
| AT | x86 | 5A25C82F5E6A2E0E5B05A0F0CB2BD904A925D020AF953138319F22AB33706916 |
| Model40 | x64 | 3EAF397F23402D0A6762039AD8076BCB80D59C75F9FEF44FECA7D0AB8DC02CEC |
| Model40 | x86 | F9E44C7FD40DE63E93EA443903684782C44F6801858A2ABE1CEE37A48DB3D262 |

Each hash names the corresponding assets/nxvm fixed-profile 0542 file; source
baseline is d7b92e35a plus the complete reviewed S2 Shared/NXVM deliveries.
The separate target-scoped P sequence publishes Shared helpers first, then
App caller/source retirement and artifacts, before coordinator acceptance.

Shared P1 ebe5172d5 is pushed. It contains only src/test x86 ownership,
registration, README and manifest changes; the receiving App changes remain
the separately verified NXVM P2 delivery. There is no Lib/Common change.

### S2 Coordinator Acceptance

Review Shared P1 ebe5172d5 and NXVM P2 e0bc4f14c against the complete Helpers
batch and every packet exit. All three production source bodies match their
originals after include substitution; all six moved headers and the original
contract assertions retain their boundaries/values. Review the new 4-by-4
drive/media matrix, rejection cases, both CMake registries, direct App callers,
removed originals, corpus metadata, document changes and all eight artifacts.
No parallel production compilation, Shared-to-App dependency, new runtime
state or hardware change is introduced. Every requested outcome has direct
build/test/source evidence above; no helper member is unresolved or deferred.

Recheck all six manifests, x86 corpus, documentation governance, whitespace,
unchanged INIs and excluded Lib/Common/MyNES paths successfully. Reconcile
Git/origin at e0bc4f14c and confirm clean worktree before this governance update.
Accept and close S2; retain T542 open for S3 onward. Warm S2 trees remain
explicitly needed by the immediately next media batch. No new task or
implementation scope is admitted by this acceptance commit.
