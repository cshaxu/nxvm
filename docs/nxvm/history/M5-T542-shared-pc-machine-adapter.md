# T542: Remaining Shared PC Machine Logic

## Admission And Baseline

The owner requests a new implementation T for remaining shared logic and its
S breakdown on 2026-10-04. Allocate M5 T542 from closed T541 at
55f9fb1920af9316c6102b7aff5550fe1d7b6dfc. Initial worktree: clean.
The [archived proposal](M5-T542-shared-pc-machine-adapter-proposal.md) defines scope;
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

The S1 inventory below freezes receiving owners. The disposition column is
reconciled as batches deliver proof; the original inventory and contract remain
historical evidence. Accept a batch only when every member has a proven receiver
or justified App retention.

| Batch | Initial capability/source universe | Proposed owner | Disposition |
| --- | --- | --- | --- |
| Helpers | profiles/device/floppy; profiles/byob/blob; profile_contract and callers | x86/ibmpc-common | S2 accepted; proof below |
| Media | machine/media FDD/HDD providers, geometry, marks, leases and callers | x86/product/machine/media | S3 accepted; proof below |
| Input/display | keyboard_mapper, mouse_mapper, machine ingress, display/frame carriers | x86/product/machine | S4/S6 accepted; one conversion and orchestration owner |
| Execution/debug | runner/waiting/control/executor_state/lifecycle/fault/debug/debug_adapter and driver | x86/product/machine | S5/S6 accepted; Profile facts isolated from cohesive adapter |
| Construction | machine create/destroy/storage; profiles/machine_plan; product/config; fixed build bindings | Shared mechanism plus fixed App composition | S6/S7 accepted; one transfer with build-selected constructors |
| Retained differences | XT/AT/default/Model40 values, ROM layout, D4, CMOS, HDC geometry and firmware source | Concrete App profile/firmware owner | S5/S7 accepted; real Profile contexts and fixed composition |
| Verification/build | tests, source lists, static gates, tools, manifests and eight artifacts | Behavior owner; external integration remains NXVM | S8/S9 accepted; all 58 external contexts and full units passed |

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

## S3: Shared Media Provider And Resource Lifetime

Admitted from accepted S2 2c4962430. Seven media C/H files and the two original
owner-local tests move with git mv to x86/product/machine/media and its matching
test receiver. The sole production target is x86-product-media; standalone
linkage requires no App source. Existing test names and assertions survive.
Lib remains the only file/lock/direct/readonly/overlay backend. Four new public
operations allocate/destroy opaque FDD/HDD objects; failed allocation does not
publish a handle, destruction closes resources and clears the caller slot.
App no longer embeds or reads Shared-private media layout. Construction binds
stable borrowed provider contexts; Core routes/registry/plan are destroyed
before the media objects. Cleanup also handles partial construction without a
Core, using the same finalizer rather than another rollback path.

The original FDD/HDD algorithm bodies match exactly after removing the new
allocation/destruction blocks and mechanical include/comment substitutions.
Geometry, protection, address marks, generations, replacement/eject and provider
result semantics are preserved. App tests read copied media observations through
the existing provider; the FDC protection test opens a real generated readonly
fixture instead of changing the private flag. Independent tests retain private
access only within their actual media owner. Partial-construction and empty-slot
destruction tests cover the new resource boundary. This is extraction proof,
not a new hardware/timing qualification or a claim that all legacy algorithms
have been redesigned.

### S3 Verification And Artifacts

Reuse the owned S2 unit, standalone and eight Release trees with the same
configure/compiler/profile settings recorded above. Build full root targets,
then run the complete unit label with -j 12: x64 496/496 in 66.16s and x86
496/496 in 62.28s. The final logs are t542-s3-final-unit-<width>.log under build;
these results include partial-media cleanup and the final opaque-handle tests.
Standalone media tests pass 2/2; corpus, source/test manifests and negative
boundary gates pass 4/4. All six corpus manifests validate. NXVM media sole-route
gate, documentation governance and git diff --check pass. Shared metadata
reconciles canonical LF hashes where prior working-file line endings differed;
those hash-only rows do not change source tokens. No original test registration
or assertion is dropped. Initial CMake child-registration/source-list and test
fixture include errors were corrected before these final runs.

Actual source/build/test review confirms one source owner, opaque App handles,
no Shared-to-App include, no private media consumer outside the owner, and no
Lib/Common/MyNES/owner INI/firmware-input change. Rename-aware C/H numstat
against accepted S2 counts 27 paths: 245 added, 115 removed, net +130, excluding
docs/build/manifests/binaries. The increase is opaque resource construction and
its rollback/provider tests plus copied test observations, not another backend
or controller path. File moves are not code deletion. All 58 external integration
contexts remain mandatory once-only T-level S8 proof, not fresh S3 boot evidence.

Eight vm-0-5-0542 Release targets built successfully. Each deployed PE verifies
its host architecture, 0.5.0542 identity and absence of .debug_* sections;
runtime Debug remains linked. Owner INIs and MyNES artifacts are unchanged.

| Machine | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 03882587562CC7C76B537D5F50FE8897A4ACE26B8770B428818188BE95E5F407 |
| default | x86 | 62672E0D813E8A04B8DD7B26363EE47C4BC9CC8CF8D0314A91357C58FFF7182B |
| XT | x64 | 4A6E6C1F87C633FE894649169F4EA39AAEE540BDF7F8DAFF5C121103933F4807 |
| XT | x86 | 0FE814AB479155E65C86F264385B99DF66814459A038AF774A37F0CBCE66A19B |
| AT | x64 | C6D9AAE6620433DFA30D4A47C8A6FB563D5A55FC5CA567220D34813129A03399 |
| AT | x86 | CBB05DC04B9B399006EB802FB4C030BA48FDBA512C07C83E75F088A8255B453A |
| Model40 | x64 | 93C5AA55F48131462612398953883077A1EEA9A006D6D3074DA68FB8EE31B2EE |
| Model40 | x86 | 15173D68E8AFF30461DCEB5BCE209DC5BED769021E3AB16F76B3CB1A471FA6D8 |

These hashes identify the S3 replacement pairs in assets/nxvm/<profile>.
Source baseline is 2c4962430 plus the complete S3 Shared/NXVM delivery. Warm
trees remain owned and needed for acceptance and the next conversion batch.
Publish Shared receiving code first and NXVM consumers/retired paths/artifacts
second as separate target-scoped Ps; coordinator acceptance follows both.

### S3 Coordinator Acceptance

Review pushed Shared P1 d3fe05a67 and NXVM P2 449c8fcc0 against the complete
media batch and packet. Inspect both provider bodies, opaque/public and private
headers, initialization/finalization, stable registry bindings, all changed
test observations and preserved assertions, the generated readonly fixture,
partial-construction regression, moved test registrations, specialized/negative
gates, manifests, documents and eight deployed artifacts. The new ownership
boundary removes App layout coupling without a second media backend or state
mirror. No unresolved media extraction member remains. Source comparisons and
final full-unit runs supply preservation proof; integration remains S8's
explicit task-level obligation, not waived by these units.

Recheck all six manifests, media sole-route, documentation and diff checks;
verify all eight deployed hashes above and no excluded-path or INI differences.
The only live old-path string is the retirement rejection gate. Git HEAD and
origin/master agree at 449c8fcc0; the worktree is clean before this governance
update. Accept/close S3 and remove its packet; retain T542 open. Prevent the
same coupling with the App-private-media include/retired-path gate. Next S4
must receive its own packet before implementation; warm trees remain needed.

## S4: Input And Display Conversion

Admitted from accepted S3 cdb768907. Source inspection finds no profile-specific
mapper algorithm: scan-set selection is a supplied board capability, mouse Y
conversion is a host-coordinate convention. Both move unchanged to Product
Machine conversion. The production frame currently traverses video snapshot,
guest frame, display event and Common frame. The two middle carriers copy the
same pixels/glyphs/palette and have no independent consumer or state owner.
Replace that production chain with the existing copied snapshot directly into
the existing Common frame, retaining the adapter-owned sequence. The old guest
frame remains only a test view and moves to test support, not a public Shared
ABI. App display.c still owns cadence/capture/publication until S5 moves the
cohesive Machine owner; this is live orchestration, not a forwarding shim.

S4 preserves video-owner state, generation acknowledgment, 16ms host publication
cadence, text bounds, zeroed padding, CP437 tables, glyph bytes, palette encoding
and raster-to-8x16 cursor mapping. Shared conversion performs no device capture,
host scheduling or input injection. Existing integration predicates are retained.
No implementation or verification completion is claimed by this admission.

### S4 Implementation And Verification

Both mapper C/H pairs and frame conversion move with git mv to flat
x86/product/machine; their original two tests move with their owner. The sole
x86-product-conversion target builds independently of App source. Mapper bodies
match S3 exactly after include substitution. Scan-set tables, Pause/E0 sequences,
mouse sign/clamping/buttons and error semantics are unchanged; retained symbol
prefixes do not imply profile selection. App still delivers input through its
one existing board path, to move with the cohesive Machine owner in S5.

Frame conversion consumes x86_video_snapshot plus the adapter sequence directly.
App display.c retains only its actual capture/cadence/generation orchestration;
the duplicate guest-frame/display-event production carriers and conversion
copies are removed. The former guest-frame declaration moves unchanged into
test support as a view of the one Common frame. Frame validation, text bounds,
zero padding, CP437 primary/secondary maps, palette/glyph bytes, 8x16 cursor
interval mapping and graphics dimensions remain. The 16ms cadence, dirty check,
generation acknowledgment and failure publication behavior are preserved.
No native handle, new video state, presenter logic or second queue is added.

Complete units pass 497/497 each: final x64 62.94s, x86 64.49s. Commands/tree
settings match S2 above; final build/CTest logs are t542-s4-final-unit-<width>
under build. Standalone mapper/frame/mouse tests plus the actual registered
manifest/corpus/negative/test-manifest tests pass 7/7 in 15.53s. All six manifests,
display authority/ROM-EGA gates, documentation and diff checks pass. The extra
mouse matrix is the one additional registered unit; no old registration or
assertion is removed. Set-1 E0 transitions, mouse sign/clamp/button mask, copied
sequence/glyph/palette and partial text padding have direct tests. Actual review
caught a misplaced new Set-1 failure check in a boolean test helper; move it to
main, then repeat the full suites and independent tests for the final proof.
Only tests changed after product builds; the production artifact inputs did not.

Rename-aware staged Git numstat against accepted S3 cdb768907 counts 15 C/H paths,
108 added, 149 removed, net -41; docs/build/manifests/binaries excluded. File moves
are not deletion. The net reduction removes actual redundant frame preparation
despite added regression coverage. Both manifests use deterministic path ordering
and exact LF hashes. No Lib/Common/MyNES/owner INI/master/firmware input changes.
All 58 integration contexts are retained for S8, not freshly claimed here.

Eight sole vm-0-5-0542 Release builds succeed. PE machine values, embedded
0.5.0542 and objdump absence of .debug sections verify each deployed product;
the runtime debugger stays linked. Warm S2 build trees remain needed for the
next adapter batch. No other product/process/tree is modified or cleaned.

| Machine | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 7955A4D59924AD96F827ECC35B9D26C955CF49F05E56247A4D5B1CAEF0B84E17 |
| default | x86 | F66EB462E7EA6112A5132778880BC2CCA0A378236B3C0F16D8501F400DB1F07A |
| XT | x64 | A63F75DBACDFE0351FE2EF7F27FBAB8D04C10680893B4D56DA7F53451231D1D6 |
| XT | x86 | C2D246916835E1CCF4176FFA79D89761CBAFF5C24EB1E0BAB9FBA85001E67EC5 |
| AT | x64 | 36D3A5420736EFABF1F7A570CAB8C732447D46F10BCC35328CF1A37BA6BD50A5 |
| AT | x86 | 144C534E359969A2FE4C29C153BB9526981EA64F630CD968D6B6DD37C970EBAC |
| Model40 | x64 | E6B902E24333D70CD49453F40AA1670BEDE8A8C193D4E9F2C398D5FA942FD12D |
| Model40 | x86 | 3B661675E838E37CB37032F3436D63220979EA6B43EA609634B188C8B95BC164 |

These are S4 assets/nxvm/<profile> replacements. Source baseline is cdb768907
plus complete S4 Shared/NXVM deliveries; scoped publication and coordinator
actual-diff review remain required before acceptance.

### S4 Coordinator Acceptance

Review Shared P1 5531231e6 and NXVM P2 878468ec4 against every conversion packet
exit and the ledger. Re-read the direct converter, unchanged mapper bodies and
public interfaces, App capture/sequence logic, removed carriers, relocated test
view, complete old assertions/new matrices, source lists and standalone target,
corpus path/edge checks, manifests, documents and artifact evidence. The full
actual diff maps to the declared two targets. No App-private header enters
Shared; no Lib/Common/MyNES/INI change exists. Output padding and cursor/CP437/
palette/font/graphics rules follow the original production path, not a new
renderer. The reviewed Set-1 regression now rejects failure correctly in main.

The final full suites and independent seven-test run pass; six manifests and
display/document/diff gates pass. Verify the eight recorded hashes/PE identities,
retired conversion paths and absence of both production redundant frame types.
Git/origin agree at 878468ec4 and worktree is clean before this governance P.
Accept/close S4. Live App ingress/display cadence/capture remain explicitly
scheduled with the cohesive S5 Machine owner, not an unclassified shared member
or a parallel conversion. T542 stays open for S5-S8 and its full integration
obligation. Warm trees remain needed; no successor App split is admitted.

## S5: Profile Observation Ownership

Coordinator refines the original execution batch before implementation at
6ee21f50a. Actual source still put a borrowed Model40 D4 handle, completed FDC
record and validity flag in generic Machine, with App integration tests reading
that layout. A blind Shared move would retain App state or require a reverse
dependency. Separate this complete prerequisite as linear S5; execution/debug,
fixed composition, aggregate audit and final acceptance become S6-S9. The
earlier S1/S4 sequence descriptions are historical planning. The proposal and
convergence ledger retain the same full scope and all 58 contexts.

The existing opaque prepared Profile context now owns those Model40 facts.
Its materialization operation installs the sole terminal sink and receives the
existing D4 factory result itself. Generic Machine no longer receives either
output or stores the three fields. Core attachment still destroys D4; Profile
does not acquire a second teardown owner. A successful Machine reset clears
the Profile's terminal validity at the same completion point as before; a
failed reset leaves it unchanged. Core teardown precedes borrowed-handle
revocation, then media/plan cleanup and finally Profile destruction.

The copied Model40 observation operation returns D4 and terminal values, never
a device pointer or mutable layout. Non-Model40/invalid calls preserve the
output with explicit status. The Profile already held selected ROM/context
lifetime; no side registry, worker, additional Profile allocation or live
state mirror is added. Its all-profile construction union remains scheduled
for S7 removal, not accepted as the final factory architecture.

App tests use a stateless copied view of the actual Profile. All former D4
assertions and FDC command/result/drive/sequence/success predicates remain.
The retirement capture predicate consumes that copied record, and its former
fake Machine fixture is now a value-only fixture. Review found five existing
synthetic predicate routines with no execution entry: the probe's --self-test
branch now exercises them without a session, INI or external ROM. This is
code-owned predicate proof, not external boot qualification or a new CTest
integration context. Normal probe runs do not capture the extra observation
per instruction unless their existing FDC diagnostic is enabled.

### S5 Sweep And Verification

Search `rg -n 'model40_board|model40_fdc_terminal|vm_profile_machine_plan_materialize'
src test cmake tools` reconciles the complete class. Original hits were the
generic fields/sink/reset/teardown, five D4 unit consumers, the FDC unit, three
integration consumers and the firmware plan fixture. All are migrated. Genuine
Model40 factory/D4 behavior stays Profile-owned and unchanged. The existing
D4 static gate now rejects borrowed D4 pointers or former Model40 fields in
either App or Shared generic Machine. It passes. The DAG review also removes
one stale direct-include allowlist edge for machine.c; its two remaining
header edges are explicitly scheduled for the complete Shared adapter move.

- Full repository units: x64 497/497, 78.54 seconds; x86 497/497, 77.14 seconds.
  Logs remain in ignored build/t542-s5-unit-<width>.log while the next S needs
  these warm trees. Both complete builds pass with warnings-as-errors.
- Existing FDC unit proves terminal capture, successful reset invalidation,
  failed reset preservation, detach validity and copied-record independence.
  Atomicity unit proves unsupported/invalid observation does not overwrite.
- Five code-owned retirement capture matrices pass on both widths through
  vm-model40-byob-retirement-capture --self-test. The final probe-only adjustment
  is rebuilt/retested; it does not change the previously passed unit inputs.
- Six manifests pass unchanged; D4 boundary, dependency DAG, documentation and
  whitespace checks pass. No Shared, Lib/Common, MyNES or owner INI changes.
- Rename-aware Git numstat over 16 tracked C/H paths: +195/-83, net +112;
  documents, scripts and binaries excluded. The positive difference is the
  explicit copied Profile boundary and failure/teardown/predicate proof, not
  another production state or execution path. The moved sink has one receiver.

All eight sole vm-0-5-0542 Release targets are rebuilt and deployed only to
assets/nxvm/<profile>. PE 8664/014C, embedded 0.5.0542 and no .debug sections
are verified; runtime debugger remains linked. Source baseline is 6ee21f50a
plus the complete S5 delivery. Product hashes:

| Machine | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 1A4237F36E529FF94415826234443BA39BE6D3B81169C41B9EB69906F88B8FE4 |
| default | x86 | AA6357B795DAA636BC26244B96FA33A7419D659EF37BDF11DA4EE642A2C090BD |
| XT | x64 | 98726A8F8CC1B22C628B9D9E5DCBD4701F0FD3E49C0D78C716D6EFD042D3D254 |
| XT | x86 | 35302B891B52EBF8FA919154C097CBECF3E2DD92C85F060621BC0A4637B03F3A |
| AT | x64 | 4FAB0A5E45F7B00436623492B11B541169E42F4D3687E200FDD2B8D91BBFF66D |
| AT | x86 | D0DBF791B57B1AAF7217F19451CCB954CAC7E3681C5AA3CB7C415D4EBB7972AA |
| Model40 | x64 | 97C953794857C2118639ABA7ABFE30A2BEAE44BC6FAC1BAD96AEE11A176E3594 |
| Model40 | x86 | 497878C82DA013278C3DA11875150BBD5BBDEC26C5702E95222E1DA11D5DB37A |

No new external integration run is claimed. All 58 once-only contexts remain
mandatory S9 task acceptance. Warm trees remain needed for S6; all S5 native
build/test handles are terminal. Executor delivery awaits scoped publication
and coordinator actual-diff review; T542 remains open.

### S5 Coordinator Acceptance

Review pushed NXVM P1 5202a06b0 against every S5 packet field and the retained
construction contract. Inspect the actual changed source/public layout,
callback publication, successful-reset guard, Core-before-Profile teardown,
all original D4/FDC predicates, stateless test views, probe self-test/control
flow, static prohibition, DAG disposition, documents and eight artifacts.
The unique observer moved, not duplicated; no hardware or timing value changed.
Copied snapshots contain no pointer and failed captures leave output intact.
The old fake Machine fixture is removed, not used as an alternate production
path. The same original predicate now executes in the code-owned probe proof.

Both complete unit suites and predicate proofs pass on final relevant inputs.
Six manifests, D4/DAG/document checks and whitespace pass. Verify all eight
recorded hashes/PE identities, and Git source inspection confirms no shared
six-component, MyNES or INI changes. HEAD/origin agree at 5202a06b0 and the
worktree is clean before this pure governance acceptance. All S5 native handles
are terminal; warm trees remain needed for the next S. Accept/close S5 only.
T542 remains open for S6-S9, complete extraction and all 58 once-only external
contexts. No App-split task is admitted and no task-level completion is claimed.

## S6: Cohesive Execution And Debug Adapter (In Progress)

Admitted from accepted S5 0d8d47c8b. Current holds the complete S6 packet;
this is an internal work checkpoint, not a partial P or acceptance. The
owner's automatic sequential admission applies. Single-session coordinator
reviews admission before switching to execution; no additional agent is used.

Actual dependency inspection covers Machine private/public layout, control,
runner, waiting, lifecycle, devices, information, debug/protocol and the S1
construction contract. Search `rg -n 'vm_profile_|retained_config|profile_kind|app-nxvm/'
src/app-nxvm/machine` confirms the remaining reverse-dependency class: the
whole adapter cannot move while it calls App plan getters/materialization.
Shared receives copied prepared values and bounded Profile lifetime/wiring
operations. Identity projection and the all-profile firmware union remain App
facts; S7 retires the dispatcher rather than moving it into Shared.

The first internal relocation uses git mv for debug.c/h and executor_state.c/h
into x86/product/machine, preserves their algorithms and removes debug.h's
unused App Machine include. The executor signal header becomes
executor_state_interface.h because App currently consumes its opaque public
contract. x86-product-machine is the one receiving target, not another runner;
the same source is removed from the App compilation list. The existing
execution-state smoke follows that owner with its original assertions/marker.
New code-owned debug budget matrices independently cover trace counts
1/10/4096, zero progress, both breakpoint address forms, watchpoint completion,
single consumption and reset invalidation. No external fixture is used.

Current evidence: standalone x64 target builds without App sources and both
mechanism tests pass; root x64 Machine builds, root x86 Machine builds and both
mechanism tests pass on x86. The x86 corpus boundary, changed manifests,
documentation structure and whitespace checks pass at this checkpoint.
All launched native build/test handles have terminated. No full-unit,
integration, artifact freshness, dependency-DAG or S6 acceptance is claimed.

App still owns the remaining Machine runtime and temporarily includes the
moved private debug layout while that same cohesive owner is being relocated.
This is unaccepted in-progress work, not a retained public ABI or permission
to close S6. The complete move must remove those App runtime consumers,
establish neutral construction/rollback, repair every actual caller/test/gate,
run the packet's full verification and deliver the eight current products.
No Lib/Common, MyNES, owner INI or deployed binary has changed. Changes remain
uncommitted until the entire S6 brief is satisfied; no partial P is published.

### S6 Neutral Construction And Whole Runtime Relocation

The next work checkpoint reads the actual contents of all 22 remaining runtime
files before git mv. No App Profile, identity, assets or plan dependency remains
in those files. They now reside under x86/product/machine with their self-includes
updated; the old App Machine production library and source list are removed.
The receiving target compiles the complete runtime, not just debug helpers.

App profiles/machine_factory.c resolves the existing real plan and prepares
copied Core configuration, timing, topology, geometry, CMOS, glyph and firmware
values. Its transferred context has three cohesive operations: configure once,
notify successful reset/board detach, and release after routes/media teardown.
The Shared creation transaction owns allocation, media preparation, Core/reset
publication and rollback; it contains no App headers or machine-name selection.
App identity formatting uses its compile-selected profile, not generic INFO.
The remaining App plan union/dispatch is still the S7 receiver.

App-owned tests retain their assertions while reading the copied construction
or their actual Profile context. A stateless support/profile.h borrowed view
does not add production API, registry or mirror state. ROM fixture constants
now explicitly include their App owner instead of relying on a generic Machine
header. Two existing native integration probes suppress Windows' exception_code
macro after windows.h so it cannot rewrite the Core fault field name. The direct
plan test declares its board/Core dependencies instead of relying on the removed
runtime library's accidental object pull-in.

The full x64 target graph compiled at the preceding internal checkpoint; a fresh
complete build/unit run follows the final header cleanup. The Shared corpus
dependency verifier passes, and stale Shared-to-App allowlist entries are
removed. Full x64/x86 unit results, standalone construction/lifetime proof,
remaining gates and eight artifact rebuilds are not yet accepted. No P is
published and S6/T542 remain open.

Fresh post-relocation proof: the complete root target graph builds on x64 and
x86. `ctest --test-dir build/t542-s2-unit-x64 -L unit -j 12
--output-on-failure` passes 498/498 in 65.18 seconds; the corresponding x86
command passes 498/498 in 57.62 seconds. The standalone test/x86 build compiles
the complete x86-product-machine target without App sources and executes both
executor-state and debug-budget cases successfully (2/2). This archive build
plus those two cases does not yet prove the neutral construction API's complete
successful-publication and failure lifetime; that independent proof remains S6.

The x86 corpus gate, both changed manifests, zero-edge NXVM DAG, machine owner
and lifecycle source gates, documentation gate and diff whitespace pass.
All launched handles are terminal. Protected Lib/Common and MyNES source/test
and MyNES deployed artifacts have no diff. Remaining S6 exits are independent
construction/rollback/lifetime coverage, final ABI and caller disposition audit,
all required gates/manifests and eight fresh 0542 artifacts, followed by complete
target-scoped delivery and coordinator acceptance. No partial P is committed.

### S6 Independent Construction And Closure-Gate Checkpoint

The independent code-owned construction test now exercises the full Shared
adapter, with no App sources or external files: normal publication, copied
construction values, driver reset, configure failure and firmware-reset failure.
It checks configure-once, successful-reset-only notice, output remaining NULL
on failure, one detach notification and context release last. Repeated partial
rollback uses the existing non-NULL Core plan lifetime to avoid repeating the
detach notice; no new state flag or registry is introduced.

Two unused production ingress APIs are removed: vm_machine_submit_input and
vm_machine_submit_host_input. Production already delivers kvm_input_event
through the sole Common driver. Original guest-input fixtures and their
conversion now live in test/app-nxvm/support/guest_input.h, not a second public
production interface. Existing App assertions are retained. The transport gate
checks the actual driver/delivery and the fixture's existing Common input route.

The independent test/x86 build executes construction, construction helpers,
debug budget and executor state: 4/4 pass in 1.25 seconds. The complete current
x86 unit suite passes 499/499 in 61.30 seconds. The corresponding x64 suite is
still running at this checkpoint; it is not claimed as passed. Both root builds
and the standalone full Machine target compile successfully.

All six manifests, the x86 dependency/private/platform corpus gate, documentation
governance and all current specialized gates pass. Repairs retain the original
gate purposes: reject retired App media paths rather than the receiving Shared
path; account for the original Profile-contract unit registration; update the
current artifact presets to 0542; remove obsolete deferred compilation entries
now actually compiled strictly in Shared. The strict matrix has 514 rows, 513
retained-strict and one deferred App composition source; the DAG has zero
migration exceptions. No Lib/Common or MyNES source/test/artifact diff exists.

Eight Release product builds are being refreshed sequentially using the existing
0542 target and deployment recipe. No owner INI is rewritten. Artifact identity,
hash evidence, final actual-diff review and complete target-scoped publication
remain required; S6 and T542 are still open and no partial P is published.

The same x64 unit handle subsequently completes 499/499 with exit zero in
254.82 seconds; it was polled, not restarted. Both widths now prove the current
implementation, including the new construction regression. The final build-only
change preserves X86_BUILD_TOOLS=OFF: the PC adapter/Common are gated with tools,
while chip targets remain independent. A fresh src/x86 chip-only tree configures
without Common or App sources and compiles x86-pit825x successfully. ON builds
retain identical production/test inputs. The final specialized gate is rerun
against that description; final publication remains pending actual-diff review.

All eight sequential Release builds finish with exit zero and deploy through
the existing recipe. Each deployed PE has the expected host architecture,
contains 0.5.0542, and has no .debug/.zdebug sections under objdump inspection.
The S6 working-source hashes are:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0542_x64.exe | 8A6272DDC61A00A439AB2C1A6614F69DFE8AC9A331B58821FE2CF67DFEDE42E1 |
| nxvm_default_0_5_0542_x86.exe | F09EB17638D07C62D173CCBA0B58A87281CCB77514CC9E01D7EC8A0F7691EC8C |
| nxvm_xt_0_5_0542_x64.exe | E1291AF2953B8D5E6B1487D44187993116C8D3F77FF6D1B11DA188F48E789BA2 |
| nxvm_xt_0_5_0542_x86.exe | FDC31DF767BEA4FEE79F9439611A8DFA561193E7E5C45D57C33917465D8CCA39 |
| nxvm_at_0_5_0542_x64.exe | 2572BF541E7ABEF8C0928617769D3345EC0679142157BBDA8EAB6EAB369D5714 |
| nxvm_at_0_5_0542_x86.exe | 1ABB3D0255C729F257A85549FCB748FB4020B6B632199F68B2E0A1517FAF2607 |
| nxvm_model40_0_5_0542_x64.exe | EA5B37DB3A2D5A0FC8B9854938D55EE6430B241216266F67BE73C6719343ADF5 |
| nxvm_model40_0_5_0542_x86.exe | 264EB9CE344AA64D415C3514E3718836B92CB5D97079259BCD1F89B57B5E6E35 |

The source diff and artifact checks confirm unchanged Lib/Common, MyNES and
owner INIs. All 58 original integration contexts remain a T-level S9 exit;
their execution is not inferred from these unit and build results.

### S6 Coordinator Actual-Change Review

The coordinator reviews the rename-aware runtime diff, neutral construction
contract, App factory, caller/test migrations, source lists, gate repairs and
artifact delivery against the admitted packet. Runner, control, waiting and
debug algorithms retain their original implementation; relocation does not
introduce a worker, FIFO, guest clock or model-name dispatcher. Construction
values are copied once. Core tears down board routes before providers and
the transferred Profile context are released; failed publication keeps the
output NULL and rollback sends the detach notice once.

The retired host-input wrappers had no production callers. Their original
fixture conversion now belongs to App test support and uses the existing
Common ingress. The obsolete retained-config mirror assertion is removed
with its field; actual media lease/path/generation assertions remain. The
original executor-state test moves to its Shared owner without losing its
assertions. New construction and debug-budget cases prove the receiving
boundary independently of App sources and external inputs.

Rename-aware production C/H counts are 290 added and 293 removed (net -3);
test C/H counts are 705 added and 262 removed (net +443). These include
boundary regressions, explicit owner includes and test-only fixtures, not a
new framework. Relocated lines are not reported as deleted functionality.
Six manifests, standalone corpus/negative gates, specialized closure gates,
documentation governance and whitespace checks pass. Full units are 499/499
on each width, as recorded above; all eight deployed artifacts are current.
Lib/Common, MyNES and owner INIs have no changes.

The complete S6 package is ready for separate Shared and NXVM deliveries.
Acceptance follows publication of both targets. S7 still owns the App plan
union/dispatch retirement; S8/S9 and all 58 original integration contexts
remain required before T542 can close.

### S6 Acceptance

Shared implementation 8956e9936 and NXVM implementation/artifacts 6f74ef631
are separately committed and pushed to origin/master. The coordinator accepts
the complete S6 boundary after the actual-change review and evidence above.
The eight SHA-256 identities refer to this delivered source package. No new
runtime verification is inferred from the documentation-only acceptance.
S1-S6 are accepted; T542 stays open for S7-S9. The unchanged warm verification
trees remain needed by S7 and no owned native process is still running.

## S7 Fixed Composition Admission And Source Audit

S7 starts from clean delivered S6 edbc98531. The complete remaining
machine_plan.c is read: it still stores a model enum, two all-profile unions,
copied configuration/topology and a retained firmware provider/context. Its
create/materialize/destroy/observation methods branch on the model. The real
EXE configuration already has generated VM_APP_PROFILE_KIND, but vm-profile
compiles every model into the same archive and factory creation still selects
at runtime. Thus a fixed EXE alone has not removed the construction dispatch.

This is the S7 mechanism to retire, not a chip/Core deficiency. Real Model40
D4 and terminal observations remain Model40-owned; PC/AT descriptor-derived
FDC/HDC wiring and XT ROM layout retain their existing semantics. Common
configuration/ROM preparation may share a helper only where those semantics
actually match. No new model registry or executor is needed.

Existing units/integration fixtures explicitly construct several boards in
one test build. Their coverage must survive through explicit real constructors
or test-owned selection, not by weakening checks or shipping a multi-model
production dispatcher. The generated binding and actual source/dependency
graph must select the product composition; per-model firmware storage must
not be an all-model union or a mirror in Shared Machine. Constructor and
failure-lifetime design is the next source-audit step before implementation.

### S7 Constructor And Lifetime Decision

Use explicit Default/5170/XT/Model40 constructors at their real Profile
owners. Each actual context embeds the one opaque plan prefix and stores only
its own descriptor/firmware/observations. The prefix is the accepted copied
construction value, not a second model enum, firmware union or state registry.
Reuse its existing configure/notify/release binding for profile-specific
materialization and teardown; do not add another operations framework.
Default and 5170 may share the actual PC/AT ROM and descriptor materialization
body because their layout/lifetime match; their topology constraints remain
separate constructor inputs. XT and Model40 retain distinct firmware lifetimes.

The generated product binding chooses the constructor at build time. The
App factory owns media-runtime projection and transfer of the prepared plan
to Shared Machine; no production model selector remains. Multi-board tests
select explicit constructors in test support and use the same factory transfer
transaction. Original getters with no callers are removed; surviving fixture
observations remain copied or explicitly borrowed under the existing lifetime.
No Lib/Common or Shared source change is required by this design.

### S7 Implementation And Verification

The common plan now holds only the accepted construction prefix. Real PC/AT,
XT and Model40 contexts own their descriptors, ROM storage and observations.
Default and 5170 share the existing PC/AT ROM layout and materialization body,
with separate topology/constraint constructors. Existing configure/notify/release
operations own materialization and cleanup; no new operations framework exists.
Common validation, CMOS/glyph copying, publication and failure cleanup retain
one path. Model40 keeps its original 925/5/17 geometry, D4 borrow and terminal
invalidation. XT retains its original ROM and media restrictions.

The build table supplies VM_PROFILE_PLAN_CREATE to the factory translation
unit. Inspection of all eight factory objects with the selected toolchain's
nm confirms exactly one unresolved constructor: default, xt, 5170 or model40,
matching its fixed product. The name enum remains for immutable identity and
test fixture selection only. It no longer dispatches production construction.
Original cross-profile units select the real constructors in test support;
the media transfer/publication body is the same production factory transaction.
Unused getters and the old all-profile constructor/reset/detach entry points
are removed. Existing test assertions are unchanged.

Complete root units pass 499/499 on x64 (67.58 seconds) and 499/499 on x86
(53.19 seconds). All current specialized gates pass, including the fixed
composition closure and existing ROM/FDC/HDC/ownership checks. Four standalone
adapter/helper tests and four manifest/corpus/negative checks pass (8/8).
An initial overbroad standalone CTest command selected 299 unbuilt executables
and reported Not Run; this was a verification-selection error, not a test pass
or runtime defect. The corrected selection uses the built independent tests;
the full root unit results cover the full unit universe. The independent
src/x86 tools-off tree also builds successfully without App or Common sources.
All six manifests verify unchanged. No integration execution is claimed here;
the original 58 contexts remain S9's once-only whole-task exit.

All eight retained Release trees build only vm-0-5-0542 and deploy through the
existing recipe. Deployed PE width, 0.5.0542 identity and absence of debug/zdebug
sections are inspected. The working-source artifact identities are:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0542_x64.exe | 09B7A98DBFD1D504D97551BF650421892977F88B969C73309245693917ABA3A5 |
| nxvm_default_0_5_0542_x86.exe | 54A883A20991E034450D8A4D68ADE11AA8B74721DF1E0BCC991BEC23192F2D07 |
| nxvm_xt_0_5_0542_x64.exe | 5DC805C9FD543F1DCE714D35C60AD0A73281D0BAB5D5B33931B40B6D9A894277 |
| nxvm_xt_0_5_0542_x86.exe | 2267CA2E08AE28FAF8A87FBA25CF8D3AB2F529248E65752CD0C06D7D62BA6D8A |
| nxvm_at_0_5_0542_x64.exe | D7192B0F59449DD24ABE3A96D64A3E0DEABDA2BEC450C89C3CBCA88AB185FC81 |
| nxvm_at_0_5_0542_x86.exe | 12EE0917A533440C822A5E8523CDE6F9011581924AD5BEFCEBAC8430187C1134 |
| nxvm_model40_0_5_0542_x64.exe | 887FE74B0D53D064A6ED41F41F0EABF649FBE9A851BFB620FC4558BACA0E2EAB |
| nxvm_model40_0_5_0542_x86.exe | F3DB0CC88E91A7422D9ED55A17769918E63B20CD47F0590ABC52F07C1838AD1D |

Actual source/test/build review confirms one candidate publication/rollback
owner, unchanged hardware/firmware behavior, no Shared-to-App edge and no
new queue/clock/registry. Gate edits follow the relocated real PC/AT owner;
they do not relax assertions. Lib/Common, Shared source/tests, MyNES and owner
INIs have no diff. S7 delivery/acceptance remain pending publication; T stays
open for S8/S9.

Git's staged numstat across the eight changed production C/H paths records
541 added and 532 removed lines (net +9). Across the sixteen changed test C/H
paths it records 67 added and 24 removed (net +43). Counts include blank lines
and relocated bodies, exclude CMake/docs/artifacts, and do not describe moved
code as deleted capability. The small production increase buys real family
contexts and direct constructors while retiring the central union/dispatcher;
the test increase keeps multi-board coverage outside production dispatch.

### S7 Acceptance And S8 Admission

NXVM implementation 7ca0ee5a9 is committed and pushed to origin/master.
Coordinator actual-change review accepts its complete source/build/test/docs
and eight artifacts against the S7 packet: real family lifetime, one factory
transfer, no production model dispatch, unchanged original constraints/assertions
and no excluded-surface changes. The hashes above identify this delivered source.
S1-S7 are accepted; T542 remains open.

Under the owner's automatic sequential admission, S8 consumes the complete
verification/build and obsolete-path ledger batch, not a new extraction target.
It reconciles actual source/test/build/tool callers and all 58 integration
registrations/predicates before S9's once-only execution and whole-task acceptance.
The current packet defines S8; this record is evidence, not another active status.

## S8: Aggregate Build/Test And Obsolete-Path Audit

Actual source lists and callers resolve every S1 mechanism to its accepted
receiver. Retired App machine/media/helper directories contain no C/H source.
Their remaining code/build references are retirement guards and the deliberate
Shared-to-App negative fixture, not live compatibility implementations. Common
still owns the worker/FIFO/generation/paused lease; Core owns guest time and
board teardown. The Product adapter owns its media, converted frames and one
construction transfer. App contexts retain ROM layout, topology and Model40 D4.
The dependency allowlist contains zero remaining migration edges.

Review of every integration diff against T541 confirms the same assertions,
budgets and acceptance predicates. Includes follow receiving owners; media
observations use opaque handles; input fixtures call the existing Common ingress;
Model40 probes observe the actual Profile context. The FDC predicate still
requires a newer sequence, command E6h, drive zero and successful completion.
Its synthetic tests retain the same rejection cases. The additional self-test
entry only runs code-owned predicates, without replacing external acceptance.
No integration registration, timeout, serial policy or skip policy was removed.

A real standalone build gap was found: the root CMake supplied three existing
GNU format-truncation diagnostic settings for Shared Debug and its two fixtures.
The root build passed while standalone test/x86 failed to compile those retained
formatters. Move those same private settings to their actual source/test targets;
delete the root settings. No algorithm, public API or warning exemption is added.
The root Lib fixture stack policy remains unchanged. Shared C/H diff is zero;
the three CMake files total nine added and five removed lines, including comments.
README documents that retained policy; both affected manifests are rehashed.

After this repair, the independent tools-on x86 build completes and all 313
registered tests pass (153.41 seconds), including manifest/corpus/negative checks
and the adapter regressions. The independent tools-off src/x86 tree configures
and builds with no App/Common requirement. Fresh complete root units pass
499/499 on each width; LastTest logs contain 499 passed and zero failed each.
All current specialized gates, six full manifest checks, documentation governance
and diff checks pass. An initial command used the wrong specialized-target name;
the corrected verify-current-specialized-gates invocation passes all its gates.

All eight Release trees build the current product plus only their registered
integration executables. Actual CTest registrations and resolved executable
commands prove 22 default, one XT, three AT and three Model40 contexts per width:
58 total. Direct probes receive the canonical product INI; native console probes
launch the current product EXE in its directory and load that same NXVM.ini.
No external integration execution or skip is counted as a pass in S8.

The eight deployed 0542 hashes remain exactly the S7 table above after current
product-target builds. No product binary commit is necessary. MyNES is not rebuilt;
its source/tests/artifacts, Lib/Common source/tests, owner INIs and external masters
are unchanged. The successor App-split proposal consumes these sole receivers and
retains all 58 contexts; it is not admitted. S8 delivery awaits publication and
coordinator acceptance; S9 must still execute every external context once.

### S8 Acceptance And S9 Admission

Shared e65c421e8 and NXVM d670e49b2 are committed and pushed. Coordinator
actual-change review accepts S8: the only build repair relocates existing
diagnostic policy, no algorithm/API or excluded surface changes; all receiving
owners and original assertions remain represented and the 58 executables are
ready. S1-S8 are accepted, not whole-task acceptance.

Under automatic sequential admission, S9 consumes the entire convergence ledger
and executes the original 58 external contexts once each across the eight fixed
Release trees. Fresh complete units and gates support, but cannot replace, that
runtime proof. Missing inputs/skips/failures block acceptance. T542 stays open;
the independent App split remains unadmitted.

## S9: Whole-Task Verification And Actual-Change Review

Verification source is delivered S8 9cf4f1955, with executable inputs last
changed by S7 7ca0ee5a9; S8 preserves the original diagnostic flags. No S9
production or test code change is required. All eight Release trees had built
their product and registered integration programs before execution. Each external
context executes exactly once; no repeat round, reduced predicate or budget-only
success substitutes for its original assertions.

| Fixed product | x64 contexts / seconds | x86 contexts / seconds | Result |
| --- | --- | --- | --- |
| default | 22 / 53.92 | 22 / 62.88 | All passed |
| IBM 5160 XT | 1 / 17.03 | 1 / 24.58 | All passed |
| IBM 5170 AT | 3 / 37.20 | 3 / 41.76 | All passed |
| DeskPro Model40 | 3 / 59.02 | 3 / 74.99 | All passed |

Parsing the eight CTest JUnit results verifies 58 named testcase entries with
status run, zero failure/error/skipped entries and no missing context. These
include the original DOS prompt/input/video/media, native product lifecycle,
CMOS, real four-profile boot and Windows 3.1 checkpoints. The exact registrations,
arguments and acceptance bodies remain in NxvmProduct.cmake and the retained
App integration corpus; S8 accounts for every source migration.

| Report under its retained build tree | SHA-256 |
| --- | --- |
| t542-s2-release-default-x64/t542-s9-integration.xml | 680B35525C4B0DED202DAB42BE85E53591583EC6D103C5C6F2436E17239916B6 |
| t542-s2-release-default-x86/t542-s9-integration.xml | 6A40ACD50BEF00BA86FCBF19682FA06B4F6EBA83C356B93597C5F73C58AB219D |
| t542-s2-release-xt-x64/t542-s9-integration.xml | DF44B139E7DB1F1A27CB1F7EAF96A8FC2EE7FAD68CC5E7164F942E01D7A67593 |
| t542-s2-release-xt-x86/t542-s9-integration.xml | 751A8DAFB8E125AA05BF5937B997380A33BCD48FDD772F1D6EDA345F2C75BB8A |
| t542-s2-release-at-x64/t542-s9-integration.xml | 79EC2B6EEC5AE8B47B938D56C0AE9F369C56B8C0018780E1C0EC0201E0169119 |
| t542-s2-release-at-x86/t542-s9-integration.xml | 0033F3492F451B9ECA7AFAA2F7C0B10765CCD328DF38AC2AD6877C6AECF5CDE7 |
| t542-s2-release-model40-x64/t542-s9-integration.xml | A66E3F15A20E02A268645293E04361E299A32DE35938FCA9CF8E40D89D16480E |
| t542-s2-release-model40-x86/t542-s9-integration.xml | 7F6A4E9FDBF1B41A4883DD36B9F8AE27DCBB070ABBCDAE13D0412720C503B64A |
| t542-s2-unit-x64/t542-s9-unit.xml | EB3FE10148813C2FB20E5BF9F77A21F097DC24F85ACB02CA6036C67E9B69F495 |
| t542-s2-unit-x86/t542-s9-unit.xml | 77793C0002B15DED8E8AFD768DA774694BB5E7F087AD98DFF004DF06267F3E1F |

Fresh S9 complete units pass 499/499 on each width; the same JUnit inspection
finds no skipped/failing case. The independent tools-on x86 build and complete
313 tests passed in S8; S9 rechecks all four manifest/corpus/negative cases (4/4),
all current specialized gates and six complete manifests. The independent
tools-off tree remains a successful no-op build, with no App/Common dependency.
Documentation and actual-diff checks are required again after closure edits.

The completion audit maps every frozen ledger batch to actual source and proof:
S2 owns floppy geometry/Option ROM validation/contracts in ibmpc-common;
S3 owns media providers and sole Lib leases; S4/S6 own one input/frame conversion
and publication route; S5 owns Model40 observations at its real context; S6 owns
one execution/debug adapter and publication/rollback; S7 owns build-selected
real constructors without all-profile unions or runtime dispatch. Failure,
reset and teardown tests prove transfer and cleanup at the same owner. Retained
Profile ROM/layout/topology/clock/CMOS/HDC/D4 and project firmware have concrete
App owners, not unresolved shared mechanisms. Surviving plan getters/materialize
serve original code-owned firmware/probe fixtures; production uses the one
prepared-construction transfer, not a second constructor. Retirement guards and
the controlled negative fixture are the only remaining retired-path references.

Actual T541-to-final diff inspection retains CPU/chip/Core/family algorithms,
all original capabilities/assertions and board-specific behavior. No new worker,
FIFO, guest clock, registry, presenter, file backend or live video/CMOS truth
is introduced. The successor App split can consume the shared receivers without
copying a peer App; it remains queued, not admitted. Timing/software qualification
is unchanged and remains in its separately bounded queue candidates.

Coordinator count reconciliation includes modified rename records and the
53-line guest-display carrier relocated from production to fixture-only code.
Git rename-aware C/H counts are production 1,079 added / 1,153 removed (net -74)
and tests 1,266 added / 443 removed (net +823). Disabling rename detection gives
production 4,373 / 4,447 and tests 1,632 / 809: the same two net changes. This
corrects P1's counts, which omitted modified trailing-brace rename records and
misclassified that cross-root move. Counts include blanks/fixtures and exclude
docs/CMake/artifacts; moving a file is not deleting a capability. Production
stays essentially size-neutral:
new copied boundaries/real contexts replace old carriers/unions/dispatch; test
growth preserves fixtures and adds ownership, failure and conversion regressions.

All eight deployed EXEs retain S7's exact hashes, expected PE 8664/014C machine
codes and 0.5.0542 identity, with no debug/zdebug sections. No new artifact diff
is needed for acceptance-only work. Runtime debugger remains. Full task diff
confirms no Lib/Common source/test, MyNES source/test/EXE or owner INI changes;
no external master write is introduced. S9 proof is complete and published in
598e64a78, with the proposal retained in history.

### S9 Coordinator Acceptance And T542 Closure

Coordinator actual-change review accepts S9 delivery 598e64a78, all nine
subtasks and the complete convergence ledger. The review reconciles the corrected production/test counts
above, inspects the retained owners and original integration predicates, and
confirms every task exit has direct proof. Complete units pass 499/499 on each
width; all 58 original external contexts passed once without skips or retries.
Standalone builds/tests, all six manifests and applicable static/document
gates pass. The eight deployed 0542 artifacts remain current at their S7 hashes;
acceptance-only edits require no rebuild. MyNES, Lib/Common and owner INIs
remain unchanged.

All owned build/test handles are terminal. Cleanup validated and removed the
12 T542 generated trees under build, without touching source, deployed assets,
INIs, MyNES trees or external masters. The reports indexed above were disposable
build outputs; their results and hashes remain in this committed evidence.
Generated trees can be recreated from source, not recovered as retained logs.

Accept and close S9/T542, remove its active packet, and retain the proposal in
history. The four-App split remains queued, not admitted. No new hardware,
timing grade or guest-software qualification is claimed by this extraction.

## S10: Corrective Quality Convergence

Read-only Lib/Common sweep: Common Machine lifecycle/input submission still
ignores event-signal failures at `machine.c` start/resume/stop/reset/input paths,
debug cancellation and shutdown wakes before worker join.
This is a separately reported error-handling gap, not authorized for this S.
Storage medium size/range checks and operation-status propagation did not show
the PC media owner's protection-residue defect. Atomic flags are not ordinary
boolean-type violations. Lib/Common source and tests remain unchanged.

Owner-approved narrow correction from closed T542 at 881063639 on 2026-10-04.
No successor task is admitted. This finite post-extraction audit supplements,
not replaces, the accepted migration ledger. Lib/Common and MyNES are excluded.

| Batch | Defect and complete repair boundary | Required proof | Disposition |
| --- | --- | --- | --- |
| Protection | FDD/HDD medium installation and all file/in-memory replacement callers | readonly to writable overlay without remove; failed replacement preserves old state | Verified by media regression |
| Capacity | HDD create multiplication and related size calculations | x86 8323-cylinder rejection and extreme geometry rejection, no truncated publication; valid creation unchanged | Verified on both widths |
| Status | Media allocation/open/create/replace and Machine construction/media callers | exact allocation/file/argument statuses survive; rollback unchanged | Verified by construction/media regressions |
| Dead state | FDD/HDD cursor fields and copied floppy comments | no live field caller lost; controller positions remain chip-owned | Verified by actual diff and full units |
| Vocabulary | Ordinary adapter/media/input/debug/display boolean declarations and old thread comments | lib_bool and named truth values, atomic flags unchanged; unit and actual-diff proof | Verified by caller builds and full units |

Each candidate install commits handle, capacity/geometry, protection, marks and
generation in one serialized owner operation. Do not introduce getters,
parallel wrappers, another status enum or Lib changes. Remove obsolete state
instead of adding a framework. Existing API names remain; operation failures
use lib_status and predicates use lib_bool. Direct callers/tests migrate with
that semantic distinction. Complete S10 delivery must satisfy its CURRENT
packet, then coordinator actual-change review accepts and recloses T542.

### S10 Implementation And Review

Shared P1 `42a2c2178` implements the finite five-batch repair and was pushed
immediately. The installer is the sole publication point for protection,
geometry, capacity, address marks and generation. HDD replacement also resets
the actual query geometry, not only its construction metadata. File/open,
allocation and argument failures retain their lib_status at the existing
operation names; failed candidates preserve the installed lease. Startup
construction keeps the same rollback owner and now returns the original error.
Existing Common boolean driver contracts remain boolean: no Common API change
or competing result family was added.

Capacity checks happen before multiplication. The x86 create reproducer is
8323 cylinders; geometry additionally rejects the 2^31 by 2^15 by 2^15 by 512
product instead of wrapping to an empty medium's zero capacity. The smaller
head/sector stride is representable in u64, and the capacity division bounds
the final multiplication. Both regressions preserve valid geometry behavior.

Ordinary flags/predicates now use lib_bool and named truth values. Atomic
storage retains its atomic type; the Core display callback's existing i32 ABI,
Core advance byte ABI, scan/address-mark bytes and tri-state ratio comparator
are not redefined as booleans. Thirteen NXVM direct test callers change only
their advanced local variable to the new predicate output type (fourteen
declarations including the mouse runner). FDC's live CHS/GPL state remains
chip-owned; only unused media cursor mirrors were removed. The old independent
device-thread comments now name Common's sole worker.

The similar-issue sweep searched all Product media install/create/replace
operations, related geometry multiplication, construction status folding,
ordinary flag declarations and obsolete cursor/thread references. Commands
included `rg` for media operation callers, `lib_(i32|u8)` flag declarations,
`device thread`, and `data.(cyl|head|sector|gpl)` in x86/product/machine, plus
the complete actual diff. Core/chip ABI hits above are retained for their real
contracts. Lib/Common were read only; the separately reported wake failure is
tracked in NXVM TODO with a Shared admission boundary, not silently fixed.
Storage media's range/status owner did not show the PC protection-residue defect.

Single-person implementer/reviewer inspection checked every changed C/H hunk,
candidate cleanup, failure publication, caller ABI and original assertions.
No chip waveform, instruction, timing, firmware, scheduler or guest-software
predicate changed. There is no new worker, queue, backend, wrapper family or
parallel state owner. Architecture/coding governance guided owner-local
publication and removal rather than layered compatibility fixes.

Final C/H counts against accepted `881063639`: production 334 added / 325
removed (net +9); tests 63 added / 15 removed (net +48). These counts exclude
manifests, docs and EXEs. Most replacements are type/status vocabulary; net
growth is checked failure handling and regressions, not an extraction framework.

### S10 Verification

The final source passes complete repository-only units 499/499 on x64
(66.87 seconds) and 499/499 on x86 (65.91 seconds), serial between host widths
to avoid cross-process native-desktop interference. The standalone tools-on
x86 tree built and passed its complete 313/313 corpus; after the final geometry
guard its rebuilt media regression and four manifest/corpus/negative gates
passed 5/5. All 82 current specialized verifiers passed, as did the six complete
manifests, documentation governance and actual diff whitespace checks.

Exploratory failures are retained honestly: the initial full Release test build
hit an existing trace-macro definition conflict in a CPU preview target, so
full units use Debug trees and product/integration targets use optimized Release.
An initial Debug x86 DOS-prompt run exceeded its unchanged five-second budget;
the optimized final run passed in 4.32 seconds. This establishes final success,
not a proven diagnosis of every contribution to that exploratory timeout.
The first Model40 x64 list had an unbuilt console runner (Not Run); after building
that required target it passed. No predicate, timeout, skip or serial policy
was weakened to obtain success. Earlier intermediate revision successes do not
substitute for the final guarded source's complete integration run.

Final optimized integration preserves all 58 original contexts (29 per width):

| Fixed profile | x64 | x86 |
| --- | --- | --- |
| default | 22/22, 21.30 s | 22/22, 24.47 s |
| XT | 1/1, 18.50 s | 1/1, 22.53 s |
| AT | 3/3, 32.61 s | 3/3, 42.54 s |
| Model40 | 3/3, 57.04 s | 3/3, 69.77 s |

Each final context runs once after the guarded source rebuild. Revalidation
after that last code correction is distinct from repeating a passing unchanged
context for confidence. No final failure or skip remains. INI/external media
inputs, terminal predicates, timeouts and resource serialization are unchanged.
The four boot-matrix rows reach their original DOS prompt/installer markers;
this is preserved boot evidence, not new timing or Windows qualification.

### S10 Current Artifacts

The eight rebuilt optimized stripped 0.5.0542 products below use Shared source
`42a2c217869fceb7bfcc93ae624f9efd8ffc8c44` and unchanged App production at
accepted `881063639`. NXVM's P2 caller edits are test-only. Build-time firmware
uses the existing qualified embedding route and unchanged BYOB inputs; no raw
ROM is added. Every pre-link Release check and post-link PE check passed;
independent section inspection found no debug/zdebug sections. Runtime Debug
remains part of the product. Paths are under `assets/nxvm/<profile>/` only.

| Current executable | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0542_x64.exe | C72081A5B0D0F1848DEDD4A0C5264228574A8294CE7A899F026ED7CF0B77B8B2 |
| nxvm_default_0_5_0542_x86.exe | 2E6E677755F1121C130CD86A8EE2093DE01AC4F9473D5EB18BC022E44842FB41 |
| nxvm_xt_0_5_0542_x64.exe | E97FDCAEC34FFC4B47A9C6092BBEE8BF6A7E0302F6B0337FEC0AFC1CAD7979ED |
| nxvm_xt_0_5_0542_x86.exe | 05F64052C901F58568669012022CA65FA6D3A5E9177E58619EF5D9E05D8E266C |
| nxvm_at_0_5_0542_x64.exe | 1904E86D287BBAEE545325880EEDDEE538135972321FF401D9CB61CC3DE20ECC |
| nxvm_at_0_5_0542_x86.exe | D3566DE3F35CD5FBCC8DA76DCBE200A00BDECF864C66641A3F31C8A24F975A2A |
| nxvm_model40_0_5_0542_x64.exe | 860A4D555754252EC94BBCD73E6DF4208BDCCD5BDFD561CFAC03A936A0C5A97C |
| nxvm_model40_0_5_0542_x86.exe | A031490C68CDFC4A20D5D3679B5093FEA478256E9F9A17454BDC91A85AC59FCE |

PE machine is 8664 for each x64 product and 014C for each x86 product.
The deployment hash check covers all eight files. MyNES source/test/EXEs,
Lib/Common source/tests and all owner INIs have no diff against `881063639`.
No MyNES target was built, no external master was written, and the four-App
successor remains queued rather than admitted.
