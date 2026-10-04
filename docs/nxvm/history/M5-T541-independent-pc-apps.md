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

S1 has completed read-only Product inspection, receiving-owner/dependency design
and bounded implementation planning. This is design delivery only; acceptance
and commit evidence are recorded below when verified. Necessary connection
permissions are pending, so no implementation S is admitted. No T541 runtime
or artifact has been delivered and no new App is declared runnable.

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
