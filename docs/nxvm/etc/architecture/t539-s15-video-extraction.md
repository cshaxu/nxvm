# T539 S15: Video Ownership And Extraction Boundary

Baseline: accepted 8831f05c5. This record designs the next complete ledger
batch; it does not claim a moved component, new silicon coverage or passing
receiver tests. The owner's automatic-S authorization covers Shared and NXVM
cutover after this boundary review.

## Complete Batch

Consume vadp.c/h and the video-owned portion of display_interface.h. Resolve
the related display.c, guest_display_frame.h and presentation_interface.c/h
ledger responsibilities; do not move machine/provider/input code into the chip
merely because its filename mentions display. CPU/FPU remain subsequent work.

Retain all existing text/CGA, EGA planar, VGA DAC/chain-4 and Compaq enhanced
color behavior. This is one video family, not a claim of a complete 6845 or
generic Intel video chip. No rendering algorithm, geometry limit or timing
grade is silently changed by extraction.

## Source Findings

- vadp.c currently includes memory.h and port.h. Native port handlers use
  t_port::data.ioByte; initialize/configure functions install PC addresses.
- CGA bytes and EGA planes, register/latch state, dirty generation and capture
  comparisons already have one VADP owner. Keep that ownership together.
- capture_text_snapshot and both CGA graphics captures read through physical
  memory. Some existing compositions use board-backed video bytes rather than
  the VADP-owned CGA provider; neither path may disappear without caller proof.
- advance receives an unused t_ram pointer. Chip time is already elapsed
  device ticks; board clock conversion remains in machine_scheduler.c.
- EGA CPU aperture selection, valid frame geometry and output enable are
  separate predicates. Preserve this distinction and the Compaq alias behavior.
- display_interface.h mixes video config/snapshot values with the machine
  provider-slot API. Shared video must not acquire that machine slot.
- configure_display performs several registrations and state mutations.
  Its port checkpoint does not establish rollback of earlier memory/observer
  registration. Existing registration tests cover the sequencer operation,
  not every whole-display failure position. Verify and repair the complete
  construction mechanism during cutover, including retry.
- App tests and the BYOB probe inspect VADP layout. Replace chip-only white-box
  checks with independent owner tests; board diagnostics use bounded copied
  observations, sampled at their established safe execution boundary.

## Resulting Boundary

`src/x86/devices/video/` owns an opaque video instance, neutral copied video
configuration/frame values, register selectors, local memory-cycle operations,
reset, elapsed-time advance and dirty/capture semantics. Its target links Types
only. Keep cohesive implementation files; do not split one owner by display
mode or invent a device framework.

NXVM owns port-address decoding/registration, physical mapping installation,
fallback backing-memory access, clock conversion, machine display-provider
slot and presentation conversion. A bounded callback reads backing bytes when
the existing composition requires them; no RAM/port pointer or host presenter
enters Shared. Use buffer pointer plus byte count/status, not an integerized
host pointer contract. Writes to mapped video state still reach the sole owner.

Move chip-owned value declarations once and migrate consumers to them; do not
retain a second identical struct or permanent naming facade. The Common/Lib
frame ABI remains separate because it is the actual presentation boundary.
Public observations expose only fields needed by real diagnostic consumers;
tests do not justify exposing the entire register layout.

Construction prepares and validates the video instance before publishing board
routes. Every failure removes only this construction's registrations and frees
its instance, preserving pre-existing routes and allowing retry. Establish the
minimal owner-local rollback contract from actual memory/port capabilities;
no generic transaction framework, private count rewinds or hidden live object.
Shutdown removes routes before destroying their callback owner. Reset reuses
the frozen configuration and never allocates a second VRAM owner.

## Verification Ownership

Original scenarios must be mapped before deleting/replacing tests:

- Chip: vadp text/status; CGA 320/640; EGA sequencer/controller/external ports,
  planar operations, CRTC boundaries/mode10; Compaq CECG S9-S13/S28 and EGA S6.
  Move pure register/memory/frame assertions to test/x86/devices/video with
  code-owned backing data. Keep original expected values and capture ordering.
- Board: display authority/composition, sequencer system, 5170 topology and
  composition, Model40 CECG wiring and registration transaction/failure cases
  stay in test/app-nxvm. Replace private observations without weaker assertions.
- Integration: preserve BYOB diagnostics and all existing video/DOS/Windows
  scenarios. No firmware, media, owner INI or host KVM change is planned.
- Add direct proof for copied frame/glyph/palette/dirty behavior, all retained
  personalities, failed construction/retry and provider lifetime. Compile and
  test the shared chip without NXVM or Common headers/targets.

Run full units both widths, standalone tools-off chip tests, default external
integrations both widths and each other profile/width boot once. Build eight
optimized 0539 EXEs; record PE/hash identities. Review MyNES link inputs and
rebuild only if affected. Verify six manifests, corpus/static/document gates
and whitespace; count source/test/build growth and explain retained paths.

Acceptance requires actual-diff review of every batch member and original
test disposition, not simply a boot or green aggregate. Deliver Shared and
NXVM separately, then coordinator governance. T539 remains open afterward.

## Execution: Value Boundary

The first source step moves copied configuration/frame/observation declarations
to video_values_interface.h. Existing source/test consumers use x86_video names;
no compatibility typedef or duplicated value struct is retained. The machine
display-provider slot stays local. Comparing the moved body against baseline
after identifier substitution proves fields, order, enum values and constants
are unchanged; only the ownership comment also changes. The unused RAM argument
is removed from raster advance and every caller. Initial dual-width compile and
text/status regression checks pass. This is intermediate proof, not complete
S15 verification; no product executable or full suite is claimed current yet.

## Execution: Register Transport Separation

Register handlers now consume a byte pointer rather than the machine's t_port.
Two port callbacks perform the PC address dispatch, retaining personality
selection for overlapping EGA/Compaq/VGA addresses. Mechanical comparison
confirms handler bodies differ only in that transport substitution and that
registration directions, addresses and order are unchanged. Eighteen existing
video regressions pass per width. This is still an in-place migration step:
the dispatch must become the final board-to-register boundary, and the state
implementation must move once, not be copied alongside the old path. No
temporary wrapper or repeated dispatch is accepted as the S15 final result.

## Execution: Borrowed Capture Input

Text and CGA capture algorithms now consume a synchronous bounded reader,
not t_ram. The existing board entry points translate physical reads into that
callback; these are the actual memory-provider boundary, not a second capture
algorithm. Planar EGA/VGA capture continues to use the sole local plane store.
The reader/context is borrowed for a capture call and never retained. New
code-owned backing tests cover missing readers, both failure positions in a
wrapped text read, successful wrap and unchanged repeated capture; both widths
pass those cases and existing low/high-resolution CGA cases. The obsolete
device_support include and historical comment denying now-supported CGA high
resolution are removed. Full extraction and acceptance remain pending.

## Execution: Independent Candidate

The extracted opaque implementation now builds as x86-video linked only to
Types, including the existing tools-off standalone build. Register selectors
carry no PC port numbers; bounded memory and backing-reader operations expose
no RAM, port or machine layout. Its public contract test passes standalone and
in both receiver widths: copied glyphs, unchanged frame suppression, 13-to-25
row transitions, wrapped backing reads with failure/retry, owned CGA bytes and
reset clearing, plus invalid register selectors. These cases establish an
initial independent boundary, not complete video qualification.

The candidate is not yet linked into the NXVM runtime. The old App video code
is the sole current production path, retained only until the S15 cutover and
original-case migration remove it. No product artifacts have been rebuilt for
this candidate, and no complete S15 delivery or acceptance is claimed.

## Execution: Board Cutover And Failure Ownership

The next step replaces App vadp.c/h state and algorithms with board-only
registration, physical-memory conversion and capture-reader adaptation. NXVM
links x86-video; reset, advance and copied observation use that owner directly.
Display configuration prepares a fresh chip before publishing memory routes.
Only complete success replaces the initial chip. Failure removes candidate-owned
memory routes/observers, rolls back newly registered ports and destroys the
candidate; the prior chip and unrelated routes survive. Port/memory owners
perform their own removals, including teardown before callback-owner destruction.
No caller rewinds a private registration count.

Both widths pass whole-display composition, machine-instance and construction
regressions. The latter now covers all eight initial CGA and twenty EGA port
allocation failures, successful retry, retained foreign port/memory/observer
routes and teardown after mappings freeze. Existing provider capacity,
reservation-allocation, observer exhaustion and overlay-priority assertions
remain. The initial constructor now reports allocation/registration failure to
machine construction and preserves an earlier port-registration failure.

Original-case migration so far:

- The complete VADP text case moves to Shared video_text: retained geometry,
  palette/glyph/cursor, raster status, explicit text capture, wrap/failure/retry
  assertions; code-owned backing bytes replace board RAM and register selectors
  replace port dispatch. The tools-off standalone test passes.
- Sequencer allocation failure/retry and untouched private state move from the
  board registration case to Shared video_allocation. Both receiver widths pass.
- Remaining video tests, private boot diagnostics and historical static gates
  still name the old layout/API and are not yet migrated. Consequently a full
  repository build/test is not claimed usable at this intermediate cutover.

The tools-off Shared suite now passes 28/28, including corpus/negative and
source/test manifest checks. This does not substitute for the pending full
NXVM suites. No executable deployment, complete-S commit or acceptance has
occurred.

## Execution: Original Cases And Receiver Observations

The full original text-status, CECG S9/S10/S12/S13 and CRTC cases now run in
Shared. Their fifteen PC port-presence assertions remain in the NXVM S9 board
test, not in a duplicated Shared port decoder. A bounded copied bus observation
replaces the boot probe's private CECG gate/graphics/sequencer reads; a same-owner
test proves observation leaves all chip state unchanged. The probe compiles on
both widths. The 5170 tests now use copied glyph snapshots, aperture queries and
register reads; both widths pass.

Sequencer-only register masks and dirty-generation bounds move to Shared with
their original configuration and expected values. The receiver retains physical
RAM round trips and observer delivery. Its whole-display fixture selects the
original 64 KiB A0000 window explicitly and enables owned CGA backing so the
existing public frame-generation contract is reliable; A0000 remains ordinary
RAM. No public test-only dirty counter was introduced and no assertion was
weakened to accommodate an unreliable observation. CECG S11 retains real
provider-to-RAM gate, generic-personality isolation and reset-route checks.
These receiver cases and Shared diagnostic checks pass both widths.

The five display gates now inspect the Shared owner. The CRTC gate's stale
twenty-register assumption is replaced by the existing last-index 18h storage
rule and existing mask predicates, not a new hardware claim. Updated source/test
manifests and the complete tools-off standalone suite pass 35/35. These are
intermediate results: remaining original graphics cases, full receiver units,
integration boots and artifact publication still prevent S15 closure.

## Execution: Controller And VGA Route Preservation

Controller masks, invalid indices, attribute flip-flop/storage, all four aperture
selections, dirty bounds and reset assertions now run in Shared ega_controller.
The receiver keeps actual port decoding and non-planar RAM round trips. Its
complete generic-board setup explicitly selects the color status alias in Misc
Output; the original chip-only setup had not installed that personality gate.
No register expectation or mask changed.

The external-register case preserves mono/color CRTC selection, feature-control
state, DAC masks/components, chain-4 pixels, frame generation, attribute phase
and reset in Shared ega_external. Seven original port-presence checks and the
actual mapped chain-4/frame path remain in the NXVM receiver.

Review found the first atomic board adapter had omitted the old optional VGA
DAC registration path. A construction-only vga_present field now selects the
existing VGA implementation and seven existing DAC routes, within the same
prepare/publish/rollback operation. Non-EGA and Compaq combinations are rejected
before publication. This preserves an existing capability; it neither enables
VGA in existing profiles nor creates a second post-construction configure API.
Failure tests cover every EGA20 and VGA27 registration position, retry and
teardown, alongside the original eight CGA constructor failures. Both widths
pass the controller/external and rollback cases. Tools-off Shared tests and
manifests pass 37/37. Full S15 completion remains pending.

## Execution: CGA Backing And Compaq Board Pages

CGA320/640 original pixel, odd-row, palette, light-pen, dirty acknowledgement,
text transition and reset expectations now run independently. The 320 case reads
the sole chip-owned CGA bytes through a borrowed capture reader; the 640 case
retains caller-owned backing bytes. Neither path is replaced by a mirrored VRAM
fixture. NXVM retains the seven CGA port-topology assertions and actual board
memory/capture routes, with one whole-display constructor instead of the deleted
piecemeal CGA-memory API. Both widths pass the four chip/board CGA cases.

CECG S28 receiver construction also uses the whole-display operation. Original
page switch/restoration, generic port separation, B0000/B8000 alias and reset
pixel assertions pass both widths. The same-owner independent S28 disposition
remains pending; this receiver result alone does not complete that ledger row.
The tools-off suite and refreshed test manifest pass 39/39. Full receiver suites,
remaining original video cases and eight artifacts remain outstanding.

## Execution: Mode10 And Compaq S6

Mode10's entire original case now lives in Shared: CRTC stride/start offset,
640x350 first/last-row pixels, palette, output disable and invalid-geometry text
fallback retain their expected values. Its fallback consumes code-owned backing
bytes. The former App executable/source is removed, not retained as a wrapper.
Compaq S6's invalid personality, default identity bytes, planar/palette result and
reset similarly move to Shared. Its four physical-port presence assertions join
the existing App personality table; its former App target/source is removed.
Both new independent cases and receiver table pass both widths.

The existing full planar receiver now constructs through the atomic board API,
selects the color status alias explicitly after configuration/reset and no longer
reads private video state. Original mapped read/write modes, latch behavior,
RAM fallback, reset and copied-frame checks pass both widths. Its independent
owner-case extraction, along with S28, remains pending. Old VADP configure/reset/
observe APIs and private video fields have no remaining App source/test callers.
The tools-off suite and manifest checks pass 41/41. The first full x64 unit
build/run has been started; its terminal outcome is not yet recorded here.

## Execution: Independent Planar And S28 Completion

The first complete x64 unit build/run finished successfully: 366/366, four test
workers, 214.17 seconds test time. This precedes the two final independent cases
below and is not claimed as their x64 receiver verification.

S28 now has its independent counterpart: page selection/restoration, shared
B0000/B8000 plane alias, capture pixels and reset use only the chip contract.
Physical port-presence and mapped-memory assertions remain in the receiver.
The full planar owner case also runs independently, preserving text/graphics
visibility, all read/write/latch modes, aperture bounds, copied pixels/palette
and reset. Where the original receiver expects ordinary RAM after a device
declines a cycle, the owner case asserts UNSUPPORTED; the original receiver
still tests the actual RAM fallback and returned byte. Code-owned text backing
remains separate from device planes. No board dispatcher is copied into Shared.

The complete tools-off suite and source/test manifests pass 43/43. The complete
x86 receiver build/run is now in progress; no outcome is claimed yet. A cleanup
scan found obsolete chip-size constants remaining only as unused App VADP
header definitions; remove them after the running build, then qualify the final
source. Full final qualification, original-case/diff review and all eight EXEs
still precede any S15 P delivery.

## Execution: Typed Memory Boundary And Qualification

The preceding complete x86 receiver run finished successfully: 368/368,
36.18 seconds test time. This includes the independent planar and S28 cases.
The definition-only App chip-size constants were then removed. Final source
review also removed the old integerized pointer transport inside Shared video:
CGA/planar operations now consume typed byte pointers and lib_size, and the
single owned plane allocation is a byte pointer through allocation, reset and
destruction. Aperture predicates, register algorithms, pixel expectations and
failure status remain unchanged. NXVM alone adapts its existing physical-bus
ABI to that typed boundary; no new public API or second memory store is added.

After that cleanup, the tools-off standalone build and all 43 checks pass.
All six corpus manifests match both file membership and SHA-256: source
Lib/Common/x86 108/22/47 files, tests 50/19/50 files. The NXVM documentation
structure check passes. Final x64 receiver units are running; final x86 units,
integrations, vendor boots and eight artifacts remain required. No S15 commit
or acceptance is claimed by these intermediate results.

## Review: Obsolete Presentation Helpers

The related presentation_interface.c/h ledger members have no production
consumer: whole-repository symbol/include search finds only their definitions,
the core-machine source list, and presentation_smoke.c. That test exercises
only the unused local queue and mutable text helper, not machine input or video
capture. The coordinator therefore resolves these members as obsolete and
removes both files, their isolated test and all build entries in S15. This is
within the admitted related-display batch, not a chip move or loss of a live
guest path. The final unit count decreases by one for this explicit retirement.

guest_display_frame.h remains a live copied machine-to-driver value contract;
display.c remains the live construction-frozen provider slot. Neither owns
registers or VRAM, and neither belongs in the independent chip. Their current
receiver is app-nxvm/machine/display.c; later board integration may relocate
these non-chip boundaries. No shim is added. The current x64 run was already
built before this retirement, so final qualification must use the new graph.

The typed-pointer x64 run subsequently completed: 368/368, 209.31 seconds.
It is passing evidence for the typed buffer/plane change, not final proof for
the subsequently removed presentation helper. Final x86 test dependencies are
being rebuilt from the 367-case graph; no runtime result is claimed yet. The
five existing video static boundary gates also pass after the source cleanup.

## Execution: Final Unit Graph

Both final 367-case unit graphs pass after retiring the unused helper: x86
48.27 seconds and x64 215.88 seconds. The specialized aggregate initially
identified one stale scheduler verifier name, core_machine_vadp_advance. The
actual scheduler already advances the sole x86_video owner with its existing
board-converted ticks. Updating that expected symbol preserves the ownership
check; the complete specialized aggregate then passes, including strict direct
compilation. No runtime algorithm changes accompany this verifier correction.

The default product's x64/x86 0539 EXEs have been rebuilt and deployed by the
existing optimized/PE-checked product target. Owner INIs have no content diff.
Default integrations and six vendor boots are the next receiving checks; the
other six product artifacts are not yet claimed current for S15.

## Final Receiving Qualification

Default integrations now pass 20/20 per width. All six vendor/profile/width
boots, each run once, reach the DOS5 installer. Eight optimized stripped EXEs
are deployed, owner INIs are unchanged, and both reusable build trees return
to default. The [final evidence](../evidence/t539-s15-video-extraction.md)
records original-case dispositions, actual source review, exact counts and
artifact hashes. Documentation and whitespace gates pass. These results
supersede the intermediate pending checks above; delivery/acceptance remains
owned by Current, and T539's full remaining scope is unchanged.
