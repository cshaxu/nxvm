# T541: Shared PC Product (Original App-Split Admission)

## Original Request And Admission

After T539 independent chips and T540 shared board extraction, split the
existing four-machine NXVM product into app-mypcxt, app-mypcat,
app-mypcdeskpro386 and default-only app-nxvm. Preserve machines, debugger,
INI/media semantics, firmware-driven boot, existing assertions and dual-width
artifacts; share actual mechanisms instead of copying four product runtimes.
The owner admits the next queued task on 2026-10-04. Allocate M5 T541 S1 from
closed T540 at 9240a3041. That was the original admission, not the current
implementation scope; Current is the sole active packet.

The owner further specifies `src/x86/product` as the sole receiver of identical
Console/API/startup/UX across the four Apps. This supersedes any reading of the
initial proposal that would copy product command/configuration/entry flows
into each App. S1 includes that complete dependency class in the inventory;
implementation still follows its design and separate governance prerequisite.

## Owner Scope Revision

On 2026-10-04 the owner separates the four-App split into an unnumbered
queue-head [successor proposal](../proposals/m5-independent-pc-apps.md).
T541 instead extracts all common Product logic of the four current PC builds
into src/x86/product. Its [revised proposal](../proposals/m5-shared-pc-product.md)
owns the current target. Lib/Common and existing x86 mechanisms are excluded;
new App creation, governance targets and deployment cutover are transferred,
not silently counted as T541 delivery.

## S1: Product Inventory And Boundary Design

Read actual product source/test/build/tool/document/artifact consumers and
freeze the shared Product receiving-owner map. This is a design prerequisite,
not permission to relocate runtime files or claim the split implemented. Include
generated CTest registrations and all 58 existing profile/width integration
rows, preserving their predicates; name shared tests that must remain shared.

The product universe is exactly these four existing builds:

| Existing product | Planned App receiver | Board composition |
| --- | --- | --- |
| IBM 5160 Model 268 | app-mypcxt | Existing XT 8088/8253/PPI/Xebec |
| IBM 5170 Model 339 | app-mypcat | Existing AT 80286/8254/8042/WD1003 |
| DeskPro 386 Model 40 | app-mypcdeskpro386 | Existing Compaq board and local D4 |
| Default PC/AT | app-nxvm | Existing project-firmware 386 build |

S1 classifies all current Product capabilities and their source, test, build
and adapter dependencies: shared Product receiver, fixed App-owned binding,
or split-only work transferred to the queued successor. The map records
dependencies, resource lifetime, unique parser/runner
ownership, coverage/asset receiver and evidence. No generic forwarding shell,
new device registry, peer-App includes or unexplained duplicated runtime is
eligible. PC110 and MyNES are not migration products.

Common Console/INI/keyboard/startup/composition moves to x86/product once;
Common/Lib continue owning generic control and presentation. Inspect the
actual generated profile_binding dependency in config/main and all command/
composition dependencies on the Machine adapter. Resolve the entire class
without Shared-to-App includes or four copied adapters. App identity,
selected hardware/ROM bytes, board defaults and construction remain fixed
App bindings. The concrete minimum public definition and common adapter owner
are S1 design outputs, not an assumed plugin or per-command callback framework.

New App target/deployment governance belongs to the queued split, not T541.
Existing assets/nxvm profile pairs remain authoritative. S1 names necessary
App connection/removal, test/build and manifest edits for explicit permission
before implementation; it does not interpret Product-only approval as unlimited
scope. Later S admissions follow the complete shared capability inventory.

## Completion Predicate

Every shared Product member has its verified final receiver; all four current
builds consume the one shared implementation without App-private imports.
No original duplicate Product path survives. Original coverage remains,
required units/integration and gates pass, and affected optimized stripped
x64/x86 pairs use the unchanged deployment mapping. No independent App split,
Lib/Common edit, hardware/timing upgrade, new ROM or MyNES change is included.

## Progress

S1 is accepted and closed with design delivery 360dfd776. It completed read-only
Product inspection, receiving-owner/dependency design and bounded planning.
This is design delivery only, not runtime extraction. The owner approved the
enumerated necessary connections on 2026-10-04; S2 now implements the complete
INI/request/startup batch. No independent App split is admitted.
S2 is accepted and closed at Shared P1 d0a7499a9 and NXVM P2 4d6750b1a.
The remaining command/factory/composition/entry batches are not delivered yet.

## S1 Actual Product Inventory

Read-only inspection at baseline 9240a3041 confirms 16 tracked Product files
and one generated binding header. The Product sources currently form one
implementation compiled into four fixed-profile EXEs, not four existing App
directories. Extraction changes its ownership; it must not manufacture copies.

| Current file(s) under src/app-nxvm/product | Receiver / disposition |
| --- | --- |
| ini.c, ini_interface.h, request_interface.h | x86/product: one parser, copied runtime request, media-mode and relative-path semantics. |
| startup.c, startup.h | x86/product: executable-adjacent NXVM.ini path; existing Base API, no native API. |
| command.c, command.h | x86/product: provider callbacks, HELP/INFO/SPEED/FLOPPY, lifecycle requests, Debug continuation and monitor output. |
| keyboard.c, keyboard.h | x86/product: common P/D/F/M registrations and guest chord policy, using existing Common input/lifecycle contracts. |
| composition.c, composition.h | x86/product: ordered Common Machine/Session/UI creation, sinks, rollback and teardown. Remove the unused machine/frame.h private include. |
| config.c, config.h | Split responsibility: copied runtime values belong to Product; build-selected hardware facts and translation into the existing vm_machine_config stay in App binding. No generated App header in Shared. |
| main.c | Shared startup/control/cleanup body in Product; App entry only supplies its frozen definition and machine factory. |
| banner.h, version.h | Banner rendering belongs to Product; identity, copyright and build version are immutable App definition values, not a new runtime selector. |
| generated profile_binding.h | Keep App-owned alongside generated immutable firmware; not a Shared include or tracked ROM source. |

All product source bodies and their direct interfaces were inspected, including
command state notifications and Debug entry/completion. Configuration projection
hardcodes six generated facts; it cannot be moved unchanged. Composition
directly creates/describes/binds the App Machine; command INFO/SPEED and keyboard
chords call its API. These are actual dependency gaps, not a need to edit Lib
or Common. Formatting/tokenization already have the necessary Types vocabulary
in types/file.h and types_interface.h; migration uses those existing symbols.

### Minimum Machine Connection

Product keeps the existing Common Machine creation and worker ownership.
The App supplies one frozen factory binding: prepare an opaque Machine owner
from the parsed runtime request and return its existing common_machine_driver;
bind/revoke the resulting Common owner; destroy the App owner after successful
Common shutdown. Failed construction uses this same ordered cleanup. The App
retains firmware, profile validation and board construction, not Product CLI.

Only copied INFO facts and speed read/write need PC-specific operations beyond
Common's public API. Their typed operations consume the factory's opaque owner,
not a Core/CPU/RAM pointer. INFO uses Common's completed state for Running,
never the App worker's active flag. Do not export all private diagnostic fields
just to retain an oversized struct. The App provides selected machine/CPU names
as immutable facts; Product does not contain vm_profile_name dispatch.

One actual Product behavior gap needs a named disposition: the existing
keyboard chord arrays choose Alt for both positions of the non-CAD branch,
so the action named alt-enter does not enqueue Enter. S3 must explicitly
review this against the approved Alt+Enter UX and test its complete make/break
sequence; do not silently copy the defect or claim equivalence without this
exception. This needs only Product code, not a keyboard chip or Common fix.

FLOPPY already wraps common_machine_set_removable_media; Product uses that
existing route directly, preserving its overlay mode and messages. Guest
CAD/Alt+Enter chords use common_machine_enqueue_input with copied KVM events,
the same neutral transport the current adapter already reaches. Frame reads,
Debug rendezvous and lifecycle continue using existing Common contracts.
Use copied KVM events with the existing physical scan-code meanings and neutral
key identities. The current Machine adapter maps scan codes to the selected
board's native set; Product must not duplicate its set-1/set-2 conversion.
Preserve FIFO make/break ordering and verify the extended Delete sequence.
No per-command callback, second execution loop, duplicated pause state or new
Lib/Common API is necessary. Machine/media/profile implementations stay put.

### Test And Build Dispositions

The three behavioral tests in test/app-nxvm/unit/product cover INI grammar,
INFO lifecycle and composition atomicity; their reusable assertions move to
test/x86/product. The INI test's fixed-machine configuration assertion remains
App-owned. Preserve rejection/cleared-output cases and every construction,
bind, UI, shutdown-failure/retry assertion rather than replacing them with a
simple happy-path fixture. firmware_build.cmake and firmware_embedding.cmake
remain App-owned because they prove its firmware build/input contract.

Integration support/session_ini.c currently uses the same parser and fixed
config projection. Repair its includes/binding, not its ROM/media route or
acceptance. All 58 prior profile/width contexts remain the T-level universe:
44 default, 2 XT, 6 AT and 6 Model40. Their original names/predicates and
asset selection remain authoritative; this planning audit does not claim a
fresh replay. Unit input remains code-owned, integration uses external assets.

The source lists and strict-owner entries in cmake/nxvm/NxvmProduct.cmake,
generated binding in NxvmProductProfile.cmake, src/x86/CMakeLists.txt and
test/x86 registration need precise connection edits. Existing ownership gates
read Product paths in verify_console_adapter_closure, verify_core_lifecycle_ownership,
verify_hdc_portal_closure, verify_t447_build_ownership, verify_vm_provider_composition,
verify_vm_machine_owner and verify_vm_machine_lifecycle; rehome their checks
without relaxing their predicates. VerifyProductSessionManager.ps1 retains its
single-session assertion against the real owner. The x86 source/test manifests
must include the new receiver. Root build policy and MyNES are not changed.

### Bounded Next S Tasks

| Proposed S | Complete batch and exit |
| --- | --- |
| S2 | INI/request/path extraction and shared behavioral tests; old parser sources removed, all callers/build entries use the one receiver, strict 0/1 and path/mode assertions retained. |
| S3 | Frozen factory/INFO/SPEED binding plus shared composition, command and hotkey implementation; no App includes in Shared, original rollback and command/Debug coverage retained, old corresponding Product bodies retired. |
| S4 | Shared entry/banner and remaining fixed App projection/build ownership cleanup; all four real EXEs link the shared Product, no obsolete shell/header/source registration remains. |
| S5 | Whole ledger/actual-diff review, required dual-width units and all integration contexts, manifests and static/document gates, latest eight verified stripped artifacts and closure. |

These are bounded planning outputs, not simultaneously admitted packets.
Each implementation S requires complete units and affected dual-width products;
the final audit does not defer earlier executable delivery. Source-style changes
are limited to removing actual dependency/Types boundary violations.

### Scope Permission And Current Evidence

Only the newly added x86/product implementation is approved as Shared code;
Lib/Common and other existing x86 implementations are untouched. Necessary
App removal/binding, test/CMake registration and manifest edits have been
reported together for owner approval before implementation. The request to
extract Product does not silently waive the prior product-directory-only limit.
No implementation batch starts until its exact connection surface is authorized.

Read queries: git ls-files src/app-nxvm/product test/app-nxvm/unit/product;
rg app-nxvm/product and vm_app_ over test/app-nxvm, cmake/nxvm, tools/nxvm;
direct reads of all listed Product files, Machine public API, Common Machine
API, generated binding template, relevant build source lists and three tests.
Documentation governance and git diff --check passed after the scope revision.
No production/test/build/artifact, user INI, external input, Lib/Common or
MyNES file has been changed by S1. S1 design can be accepted independently of
the pending implementation permission: it names the complete permission gate
rather than authorizing extra files. T541 is not code-complete or closed.

The completed design check verifies all changed relative document links and
16/16 active packet fields. Documentation governance and diff whitespace checks
pass. Actual-change review confirms the eight edited/new files are NXVM
documents only; no executable input changed, so no artifact rebuild or runtime
test replay is applicable to this design P. The S1 implementation delivery is
the design itself, not a partial code-extraction milestone.

## S1 Coordinator Acceptance

P1 360dfd776 was pushed to origin/master. Coordinator review inspected the actual
eight-file committed change, the two proposals, owner-map/typed boundary and
scope transfer against the original request and latest owner restrictions.
Every changed file is an NXVM document; no source, test, build, artifact, INI,
Lib/Common or MyNES change is present. The four existing builds retain their
accepted T540 baseline. All changed relative links, the 16-field P1 packet,
documentation governance and whitespace checks passed.

Accept S1's design, not completion of T541. P2 records only this acceptance and
removes the active packet. S2 is not admitted until the owner authorizes the
enumerated necessary App/test/build/manifest connection edits. No new target,
rule change, hardware behavior or artifact is admitted by this closure.

## S2: Sole INI And Startup Receiver

The owner approved the enumerated connection surface on 2026-10-04. S2 moves
ini.c, ini_interface.h, request_interface.h, startup.c and the renamed public
startup_interface.h with git mv into x86/product. All live App and integration
includes now use that receiver; the App source list and strict residual owner
list no longer compile the old parser/startup. The fixed hardware projection
remains in App config.c. Existing symbol names and request layout are retained.
No compatibility include, duplicate parser, new configuration key or native
file/presentation API is added. NXVM-only presets select the current 0541 target.

Shared INI assertions move into test/x86/product; the fixed-config cleared-output
assertion stays in the original App unit target. Strict shared compilation
requires explicit char-pointer conversions at existing Types/Base calls and
uses existing lib_c formatting in the test; these are signature adaptations,
not changed parse/error behavior. Original style and all prior assertions remain.
The new x86-product-config target links only Types, Storage and Base; it works
with tools disabled and does not require a Common driver or NXVM profile.

The corpus gate registers Product and rejects App-private imports. New negative
cases reject an App config include and an undeclared Common private dependency.
The similar-issue sweep searches old INI/startup/request paths across src, test,
cmake and tools and finds no surviving live references. Historical records keep
their baseline paths. Lib/Common, existing chip/Core/board implementation,
MyNES, owner INIs, raw external inputs, root README and rules are unchanged.

Complete repository units pass 493/493 on each width. The extra registration
separates the old combined parser/fixed-config test without losing an assertion.
x64's first complete run reaches 445 tests but exceeds its unchanged 300-second
aggregate budget; that is not acceptance. Its complete retry passes in 88.43s;
x86 passes in 66.49s. Shared tools-on unit suites pass 295/295 each, plus all
four manifest/corpus/negative gates; tools-off complete suites pass 293/293 each.
Six shared manifests pass 6/6 each width. Both current specialized static gate
aggregates pass, including 516 strict matrix rows, 497 retained strict and 19
explicit residual owners. Documentation governance and whitespace checks pass.

Review uses git diff --cached --numstat --find-renames, counting tracked C/H
source/test changes including renamed headers and excluding documentation,
manifests and generated/artifact paths: 13 logical paths, +103/-96, net +7.
Those seven lines come from splitting the fixed-config test into its own owner;
there is no production algorithm increase. Including build/registration/gate
files gives 19 logical paths, +123/-105, net +18. Independent compilation and
the new forbidden-edge regressions justify that small registration increase.
Warm build/test caches remain needed by S3-S5; no temporary source is published.

### S2 Product Artifacts

Shared P1 d0a7499a9 is pushed to origin/master and contains only the x86
receiver, matching tests and their build/boundary/manifest metadata. The
ordered NXVM P2 removes the original bodies and delivers actual caller/build
connections, current artifact revision and product evidence. This separation
keeps each commit within its declared target rather than mixing a cross-root
rename into one commit.

All eight existing t535-s4 profile/width Release caches build vm-0-5-0541.
The actual deployed files pass the existing PE-width and optimized-Release
checks; objdump finds no debug/stab sections and strings confirms 0.5.0541.
They retain the runtime debugger and unchanged adjacent INIs. Firmware input
masters are not changed. Once validated, superseded 0540 EXEs are removed from
the four deployment directories; Git history retains them. MyNES is not linked
to x86-product-config and neither its source nor its 0043 pair is changed.
This is S2 packaging proof, not a new whole integration qualification; S5 owns
the original 58 integration contexts after the complete Product extraction.

| Deployed file under assets/nxvm | Bytes | SHA-256 |
| --- | --- | --- |
| compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0541_x64.exe | 1346537 | AFF0624C74130E0045DAF1BDF832D077E01F3D478E4967A8E7876818DB8BCF62 |
| compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0541_x86.exe | 1517277 | DAF0497C55A4D3F92D23C83717B10EB550A58069C6A28E84E768449A42B67153 |
| default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0541_x64.exe | 1362854 | F65B7192DB1EF7916D093955BEB2F121B6BE0463A541084FA579C48E38F4321A |
| default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0541_x86.exe | 1533592 | D277FE315A50BE25AF6C165A3ACFB4C6D1979DABFD8F501ACE3FEBF897BA8482 |
| ibm-5160-model-268-360k/nxvm_xt_0_5_0541_x64.exe | 1362822 | 15AD2C410EE6995708241DB489BFA846115E1FDDF4F265C76FBA483A6B6D1DF3 |
| ibm-5160-model-268-360k/nxvm_xt_0_5_0541_x86.exe | 1533559 | CCC0B31412092C68239E11AC64B3B5EC2385CBB6C6256E2FB3A58F80EA4098FC |
| ibm-5170-model-339-1200k/nxvm_at_0_5_0541_x64.exe | 1362888 | C14135FF5239C35E9BBA2F74F09F0897B6F29C2B486AE1A81599A1AFCE42E695 |
| ibm-5170-model-339-1200k/nxvm_at_0_5_0541_x86.exe | 1533627 | D1AC6065AA65671FAA16E134E931B46FA19269A818383F1A2E00892D129A72AE |

### S2 Coordinator Acceptance

Review the actual two pushed commits, every relocated source/header and test,
caller/source/link/preset changes, manifest entries, negative checks and the
eight deployed identities against the complete S2 ledger batch. All original
assertions have their correct owner; parser grammar and failure publication are
unchanged. Exactly one parser/startup source is linked. Shared-to-App imports
and old direct sources are absent. Artifact hashes match the table; the four
owner INIs and excluded paths have no diff against bba227a8b. Complete unit,
independent, static and manifest results above apply to the delivered tree.
Packet fields are 16/16; changed relative links, documentation governance and
diff whitespace checks pass. HEAD equals origin/master and the worktree is
clean before this governance-only P3. Accept S2 and remove its active packet;
T541 stays open, with no timing/guest/four-App qualification claim. Automatically
admit the next bounded S3 under the owner's standing sequential authorization.

## S3: Sole Command And Composition Receiver

Shared P1 4b6c0ca19 receives command, keyboard and Common composition bodies
with their original symbol/style vocabulary. NXVM's paired P2 removes their
old source/test paths and registrations and binds the receiving implementation.
The factory is copied once; App retains fixed hardware/firmware projection,
construction/driver description, candidate cleanup and INFO/speed translation.
Product alone owns Common construction, sink publication and ordered teardown.
No Core pointer, App header, per-command registry, queue or worker is added.

The complete S1 command/composition batch changes from App-owned to the sole
Product receiver. Preserve all five machine-construction failure classes,
Session/UI create/bind failures and shutdown-failure/retry assertions; the
prepare fixture includes App-owned rollback before candidate publication.
INFO still derives Running from Common's completed state (all six states,
both worker-active values), not the worker. Added command/lifecycle/prompt,
speed, Debug lifecycle and keyboard make/reverse-break assertions reinforce
the migrated owner. AltEnter now sends Alt/Enter, rather than Alt/Alt; CAD
retains Ctrl/Alt/extended Delete. FLOPPY and chords use the existing Common
media/input transport; machine-side codec/media implementation is unchanged.

The similar-issue sweep reads all Product files, callers, registrations and
ownership gates; searches old command/composition/keyboard paths and App
imports in Product. Obsolete paths and residual warning rows are removed.
App media/input APIs remain live in machine-owned tests/integration and are
not copied into Product. Existing Lib/Common, x86 chips/Core/boards/Debug/xasm32,
MyNES, root README/rules and owner INIs have no diff against 9432c70b1.

Full repository units pass 494/494 on each width (final x64 68.22s, x86
55.43s). Standalone shared tools-on units pass 298/298 per width; tools-off
complete tests pass 293/293 per width. All six manifests pass on each width;
x86 corpus and negative gates pass. Specialized gates pass with 515 direct
compile rows, 499 strict and 16 retained residual rows; the new Product targets
are explicitly included. The duplicate-ownership negative self-test emits its
expected rejection then passes. Documentation governance and diff whitespace
checks pass. Source/test manifests identify shared-m5-t541-s3.

All eight Release caches rebuild vm-0-5-0541 and deploy only their existing
NXVM EXE. PE width and stripped-section verification pass; runtime Debug
remains linked. MyNES is not rebuilt. Owner INIs and external firmware/media
masters remain unchanged. This S claims Product/unit/packaging proof, not the
T-level 58 integration closure, which remains S5.

Counted baseline-to-delivery Git numstat with rename detection: 15 logical
C/H source/test paths, +451/-203, net +248. Documentation, artifacts and
build scripts are excluded. The positive cost is the real frozen factory
boundary, App projection and added command/chord regressions; no second
implementation/state owner is retained. Cross-target renames are delivered as
Shared additions followed by NXVM deletion, as in S2.

| S3 deployed file | Bytes | SHA-256 |
| --- | --- | --- |
| assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0541_x64.exe | 1348135 | 0F0F40166FA04A3AAAB6AA5C8C424F32709619B77F7332FA1EB200665BD5F257 |
| assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0541_x86.exe | 1519408 | DA77AABB2ACDE2CA41C676165C03DCEC78753114549DF6E97C71CF29054A7AAF |
| assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0541_x64.exe | 1364452 | BFBFB702445104193859ACA51920364A6F75B7BCB8A6944FE63801654E0D85E7 |
| assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0541_x86.exe | 1535723 | 9FA84CF77D203E061516652EA5A26A8BCD370EE4C41C094BFD0EF444ED52210D |
| assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0541_x64.exe | 1364420 | C5A3184B3BF45BE4780287E2582A2CB8C74F5511A4192F48E5090A825534EB8D |
| assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0541_x86.exe | 1535690 | 89F7448C5605100A76B8136E49D39A0AF6E2CE828AAD871021B85734D77545C1 |
| assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0541_x64.exe | 1364486 | 2759889703C3F1E39A14C93B8A4847A349E1E181DD4B50DBC2FC87DDA22036E8 |
| assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0541_x86.exe | 1535758 | 885068BD89F7CC37DBD8DCCE0837D92738C5697FEF74B6D56C1980C91550CAA3 |
