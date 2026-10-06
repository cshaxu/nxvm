# T545 S6 IBMPC Test Completion

Baseline e7ac6ce4d; Shared test work and NXVM receiver records only. Production,
public API, owner configuration/assets and current 0545/0043 EXEs are unchanged.

## Initial Contract / Assertion Map

Generic Machine memory replacement identity and floppy insertion/ejection
generation/running rejection are currently in App fixtures. Existing IBMPC
construction tests already own a neutral assembly/firmware/lifetime probe;
extend that owner rather than copying default-profile factories or adding a
framework. Preserve fixed default reset-vector and supported-capacity choices
in App tests. Media fixtures are code-owned bytes, not outside input files.

Existing AT contract/assembly tests cover leaf/route materialization but the
PC/AT descriptor/CPU-contract preparation layer needs its own direct review.
Inventory Board, Machine and Product public mechanisms and existing assertions;
function-name references alone do not prove coverage. S6 is not accepted yet.

## First Owner-Local Batch

construction_smoke now uses its existing neutral assembly to prove memory
replacement preserving Machine/Core/FDD/HDD/Debug identities and Core byte
count, plus invalid reset-vector outputs. Its own firmware/geometry/media
choices are code values, not a copied default factory. A temporary zero-filled
floppy tests insertion generation/path publication, valid-image insertion and
ejection rejection while Machine control is running, unchanged state after
each rejection, stopped eject, Common binding teardown and file cleanup.

The App reconfigure case retains the real default factory's 32MiB support and
F000:FFF0 reset-vector predicates through copied public information. Its generic
identity/null-output assertions move to IBMPC. The App media case retains real
default floppy/HDD construction and accepted 1.44MiB image via public information;
generation/running/eject claims are now owned by IBMPC, not removed from coverage.
Original App target names/markers remain. No production/API or artifact changes.

These three current receiving cases pass x64. Standalone IBMPC configures with
only inward production dependencies on both widths; complete builds are active.
Remaining width/runtime proof, AT preparation and whole package inventory are
pending. No current success closes S6 by itself.

## Shared AT Preparation Proof

The AT assembly test now owns a code-defined descriptor, all supported AT CPU/
FPU request projections, default selection, invalid CPU/FPU and rejected model
validation preserving output, Core materialization and copied arrays/services.
Snapshot's prepared-values prerequisite is exercised explicitly; the initial
empty-values call failed and was corrected as a fixture precondition, not a
production change. A compile-time field spelling error (cmos_defaults versus
the actual cmos member) was also corrected and both widths rebuilt.

Machine preparation now uses its own neutral callback and real Shared helpers
to produce the candidate, then verifies delegated failure, wrong ROM size,
copied single-ROM ownership and two-chip interleaving through the real
vm_pc_at_construction_create API. All firmware bytes are generated in C; no
App model or external ROM is borrowed. Candidate releases remain explicit.

Initial full independent IBMPC runs pass 181/181 per width (114.51s x64,
94.84s x86). The subsequently expanded preparation case passes both widths;
current assembly/construction cases and manifests also pass both widths.
Whole inventory and final receiving/full-unit checks remain required. README
now reflects the actual x86 ownership of the five tests moved by S5.

## Complete Owned Mechanism Inventory

Scope is Board-common/XT/AT wiring, Machine adaptation/media and shared Product
contracts. This is owner-local test completion, not fresh chip/CPU manual
qualification. The API-name sweep is diagnostic: callback return types such as
core_machine_media_result are not callable omissions; object creation, reset,
forwarding and teardown helpers execute under the actual owning paths below.

| Owned mechanisms | Local assertion proof / disposition |
| --- | --- |
| Construction/plan/resource lifetime | board, plan, controller_authority, board_binding_identity and registration/rollback cases retain validation, unpublished candidate failure, callback lifetime and unique teardown ownership. No App source supplies their objects. |
| AT shared contract/assembly/profile values | contract, assembly, profile_contract and construction_helpers cover enabled leaves/routes, IRQ/DRQ ordering, topology preservation on failure, drive/media combinations and code-owned ROM admission. This S adds descriptor validation, CPU/FPU/default projection, Core materialization and prepared snapshot array ownership. Model-specific validators stay App callbacks. |
| XT/AT physical wiring | existing PPI/keyboard/KBC, planar-parity/NMI, PIT/refresh/DMA/PIC/cascade and scheduler cases observe the real board chain, not chip internals masquerading as board truth. Private endpoint create/reset/advance functions are exercised through those constructed adapters. Chip behavior and neutral Core cases remain at x86. |
| Media registry and display binding | media/provider and display_provider verify copied descriptors, registry/slot freeze, retained provider identity, missing/failed callback boundaries and reset/change visibility. Board/FDC/HDC adapters consume the real contract. |
| Machine ownership/lifecycle | construction and executor_state prove publication, copied construction values, reset/failure/release ordering, control and Common driver boundaries. New neutral cases own memory identity/capacity replacement and floppy generation/admission/eject/teardown, previously only in App tests. |
| Media adapters | provider and direct_readonly exercise allocation into empty slots, identity preservation, CHS/sector/format operations, readonly/direct/overlay behavior, copied information and discard; tests use created bytes only. Lib file/locking mechanics stay Lib, not a second implementation. |
| Preparation | preparation proves asset copying, CMOS/font shape/conversion, candidate finish/release, floppy policy and new real PC/AT ROM preparation, status propagation, separate/interleaved chips and copied lifetime with a neutral callback. App-specific ROM identity remains external integration. |
| Input/frame/debug/pacing adaptation | input, keyboard_mapper, mouse_mapper, frame, debug_budget and timing/source cases retain neutral copied input, text/graphics/character/palette/cursor projection, bounded debug completion and guest-time policy. Lib presenters and Common lifecycle retain their own tests. |
| Product | ini, command, keyboard, factory, composition, entry and build_smoke own strict parser/options/path rules, injected factory/CLI, hotkey product meanings, creation/rollback and failed shutdown retaining dependencies. Common/UI/Machine doubles are local product-boundary dependencies, not replacements for their component test proof. |

No registration or external integration context is removed in S6. The two App
targets and markers remain; their generic assertions have explicit receivers in
construction_smoke. Default supported 32MiB/reset vector, actual default media
acceptance and HDD construction remain App assertions. Running insertion is
strengthened to try the valid image, with state checked separately after insert
and eject rejection. Source/backend tests are not allowed to borrow App factories
or any sibling test fixture. Independent builds prove that dependency closure.
All named owner gaps are now implemented; final verification and actual-change
review, rather than this ledger text alone, decide S6 acceptance.

## Final Verification And Actual Review

Current independent suites pass 181/181 per width (8.13s x64, 5.63s x86).
Complete repository units pass 532/532 per width (67.93s x64, 98.47s x86).
Six IBMPC source/test manifest, corpus, Types/ownership and negative gates pass
per width. C11 strict-warning builds, documentation governance and diff checks
pass. Every changed receiving executable was rebuilt before its complete run;
standalone builds require only declared inward sources and root test tools.

Sequential executor/coordinator actual-change review accepts each added assertion,
the generic-to-owner relocation map, unchanged App markers/targets and stronger
valid-image running rejection, without shrinking the external scenario universe.
Temporary media is closed/ejected before removal; Core/Profile/Common borrowed
contexts survive until teardown. No production/API or App configuration/media/
binary change occurs, so current 0545/0043 EXEs remain valid without rebuild.

Tracked Shared C tests: three paths, +207/-0; cases reuse existing fixtures,
add no target/framework or runtime path. NXVM test receivers: two paths,
+16/-50 (net -34), with private/raw-CRT includes retired and copied public
information used for actual default-profile checks. Combined source/test change
is +223/-50 (net +173), excluding README/manifests/evidence and artifacts.
Added code owns missing assertions, not a parallel implementation.

Deliver Shared implementation P1, NXVM test receiver/evidence P2, then coordinator
governance P3. S6 closes only on acceptance. All four requested test packages
are then accepted; T545 retains its original final S7 external qualification.
