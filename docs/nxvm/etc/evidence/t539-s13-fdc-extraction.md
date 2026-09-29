# T539 S13 Working Evidence

Baseline `579f4e35a`. The [boundary](../architecture/t539-s13-fdc-extraction.md)
and active Current packet govern this batch. The sections below retain the
chronological investigation; the final delivery section supersedes their
in-progress results. Current owns coordinator acceptance.

## First Cutover Check

The original `devices/fdc.c` was git-moved to the shared chip directory and
its board/media/port/PIC/DMA operations replaced with neutral pins, records,
byte service, timing and signal contracts. NXVM's replacement `fdc.c` contains
only those actual adapter/drive responsibilities, not a second command switch.
The private chip state has no DOR/DIR/CCR, physical-cylinder array, media
registry or peer pointer. The original command dispatch style remains.

Configure-FDC now checks chip allocation before port-publication success and
uses its existing rollback on failure. The machine scheduler's private phase
read becomes a due-now check on the existing deadline route. Diagnostic/test
private consumers have not yet all migrated; that work remains before closure.

Observed checks on the worktree:

- Strict C11 `-Wall -Wextra -Wpedantic -Werror -fsyntax-only` passes for chip
  and adapter.
- CMake `core-machine` builds all changed machine/adapter objects and links
  with the new Types-only `x86-fdc8272` target; x86 corpus dependency check passes.
- New shared fixture `fdc_contract_smoke.c` builds and runs with exit 0:
  reset deadline and four SIS results, 512-byte DMA read/TC/result withdrawal,
  independent unit-2 SEEK, and controller-reset versus physical-head ownership.
- Existing `nxvm-firmware-floppy-smoke` builds/runs with exit 0 and reports
  `Project BIOS floppy read/write/CHS/buffer/reset/error contracts: OK`.
- The first test registration attempt used a missing `_smoke` suffix; corrected
  to the shared registrar's existing convention. No failed case was removed.

These runs are transient selections, not a new fixed test suite. They do not
replace original FDC case migration, full units, standalone build, production
boot tests, manifests, static negative controls, final artifact rebuilds or
actual-diff acceptance. The follow-up null-byte guard/source simplification
was rebuilt and both named checks passed again with exit 0. No current product binary was regenerated in
this first check and no partial P was committed.

## Diagnostic And Board-Test Cutover

Both external boot tools (`vm_profile_floppy_boot_matrix` and
`vm_byob_dos_boot_probe`) now read copied chip observations. The existing
terminal trace needs PCN alongside the board's physical cylinder, so the
observation includes the four PCNs; the chip test verifies that modifying this
copy cannot mutate the controller. DOR/CCR and physical position stay with
the adapter. The tools build on x64 and x86; this is not a fresh boot result.

Five existing board regressions now use byte/status operations and the
deadline contract instead of private phase/seek arrays: controller authority,
FDC media-change ports, FDC topology ports, Model 40 FDC S24 and Model 40
integration S8. The product-local fixture owns wire-value expectations and
bounded deadline advancement, with checked failures; it neither recreates
controller state nor depends on a chip-private header. Topology retains the
77-step recalibration bound. Model 40 retains explicit 128-tick byte spacing.
The new chip contract and all five board regressions pass on both host widths.
An initial fixture build used a nonexistent `LIB_U64_MAX` spelling; corrected
to the existing `LIB_UINT64_MAX` before these passing runs.

Remaining private test consumers, including the large original FDC command
suite and T242 corpus, still need migration and ownership separation. These
results do not replace that coverage, full units, external boots or final
artifact verification. No S13 P or product EXE update is claimed.

## Record Tables And READ TRACK Progress

The original SCAN status table (nine combinations), SCAN sequence table (nine
rows) and ordinary/deleted READ table (six rows for each of two commands) now
live in `test/x86/devices/fdc8272/fdc_records_smoke.c`. All three expected-value
tables were compared byte-for-byte with their former NXVM functions and are
unchanged. The old three functions/calls were deleted, not retained as another
test implementation. The shared fixture supplies only sampled pins and bounded
record callbacks; owner-local phase checks use the chip-private header solely
inside the chip's own tests. Both chip executables pass x64/x86.

The tools-off standalone tree `build/t539-s3/fdc-independent` independently
builds and executes both FDC tests using only the chip and Types. It is retained
for this S's final independent checks. Common's existing manifest script is
a governance tool reference, not a linked Common runtime dependency. This is
not yet a full standalone-suite or manifest pass.

Migrating `vm_fdc_t242_corpus_port_smoke` exposed an old false-success exit:
`goto done` after failed progression could return success when the session
existed and `failed` was still zero. Success now additionally requires reaching
the final stage. With the old 1024-event bound, the repaired test failed at
stage 5, guest tick 1024, with 8876 of 9216 bytes remaining. The test now budgets
phase-level progression against the full transfer size; hitting the bound is
still failure. Its actual oracle remains all 9216 bytes plus seven result bytes,
IRQ withdrawal and the non-MFM error case. The full test passes x64/x86.
No production instant-completion path or timing relaxation was introduced.

The similar-exit sweep used `rg` for `goto done`, cleanup labels and success
returns in all NXVM unit files matching `*fdc*`; only this corpus uses the shared
cleanup jump, and every early jump now fails the final-stage condition. Other
matching tests report their accumulated failure or return failure immediately.
The large remaining mixed FDC suite is still being separated; these migrated
tables and the READ TRACK pass do not constitute complete S13 acceptance.

## Mixed-Suite Ownership And Input Trigger Repair

The original READY-edge and seek-ownership functions now live in the third
chip executable, `fdc_causes_smoke.c`. The fixture samples four READY inputs
independently of media/Track0 and tests per-unit cause identity, interleaved
ST3, parallel completion order, duplicate rejection, four outstanding causes,
the fifth-request refusal, fifteen blocked command families and reset cleanup.
SPECIFY retention across both reset edges and the original 24000-tick step /
parallel-seek deadlines also moved here. The cadence test starts a fresh local
axis instead of inheriting the mixed test's backward time jump. A provider
pulse counter retains exact 39/77/2-pulse recalibration and zero-pulse unready
checks; the board test still proves physical end stops and DOR STEP routing.

The remaining original NXVM suite now uses wire MSR/status, checked deadline
advancement and copied observations, with no chip-private struct/header.
Its command helper reports bounded seek failure instead of silently returning
after exhausting its loop. The ten-command readiness matrix, eight ST0
identities, 24 WRITE-TC variants, 64 TC/CHRN variants, no-implied-seek checks,
DMA2/NDMA ports, media failures, marks/FORMAT and physical-drive cases remain.
Copied observation failure terminates this test as failure, not a zero value.
Markers for moved-only cases no longer falsely claim execution in this suite.

Two fixture dependencies were made explicit without changing production:
DIR acknowledgement issues an actual STEP instead of depending on the moved
cadence case's leftover PCN; fixture-only frequency changes reprogram CCR so
the adapter supplies the intended timing rather than retaining the preceding
case's configuration. Real product configuration remains frozen.

The retained readiness matrix caught a real extraction regression: its
mid-transfer READY-low rows returned the expected `48/00/00`, but IRQ remained
asserted after draining the result. The replacement DOR writer had called
the combined refresh operation, adding a READY/SIS cause that the accepted
writer never polled. Inspection of `579f4e35a` confirms two separate triggers:
DOR revalidates active transfer inputs; the existing refresh path polls READY.
The chip now exposes those distinct operations (`refresh` and `poll_ready`),
with one state/cause owner. The boundary record is clarified accordingly.
An owner-local check proves refresh alone does not enqueue a READY cause;
the following poll does. Original board expected statuses/IRQ assertions were
not relaxed. Temporary per-line instrumentation was removed.

All three chip executables and the migrated original board suite pass x64 and
x86. The tools-off standalone build also runs all three chip executables with
exit 0. `git diff --check` passes; a source/test sweep finds no remaining direct
FDC command/phase/seek/timing-state reads in NXVM. These checks do not claim a
full-unit run, complete old-case ownership audit, static negative-control pass,
allocation/registration failure proof, external boot or refreshed EXE. Those
remain S13 work, along with manifests and final actual-diff acceptance. No P
has been committed and no deployed artifact has changed in this batch.

## Failure Boundaries And Full-Suite Verification

The fourth independent chip test injects actual allocator failure, invalid
timing after allocation, repeated creation/destruction and destruction with an
active request. It proves null publication, allocation release and output
withdrawal without a production allocator hook. The existing NXVM port-assembly
test now covers failed chip creation and each of seven FDC port-registration
positions, checking cleared adapter/topology, withdrawn routes and successful
retry. The test-only injection wraps the actual adapter implementation.

The FDC state-machine and DMA/FDC gates now inspect the split owners. Seven
negative controls reject reverse App dependencies, public mutable chip state,
old/private App phase access, raw DMA access and a revived FDD transfer cursor.
The first specialized aggregate exposed a missing auxiliary registration for
the new negative test; adding it to the existing registration list fixes that
omission without weakening the count check. The complete specialized aggregate
then passes, including the 379-row strict/deferred compilation audit.

Both x86 manifests were regenerated for the current corpus. All six manifests
and the x86 corpus boundary pass. The tools-off independent x86 build executes
its complete 22-test suite successfully (4.83 seconds), not just the FDC tests.

The first full x64 build found remaining old FDC wire constants in a port test;
the caller sweep also found two in the READ TRACK integration test. They now
use test-owned wire constants without reintroducing a private chip dependency.
Complete rebuilt units pass 355/355 on x64 (199.42 seconds) and x86 (35.41
seconds). Runs are sequential to avoid simultaneous host-interaction suites.
Default x86 external integration passes 20/20 (17.67 seconds). Default product
builds have refreshed both embedded-ROM EXEs; this is not yet an eight-artifact
or vendor-boot acceptance claim. Owner INI contents and external originals are
unchanged. Final coverage/diff review and remaining receiver verification are
still required before any S13 implementation P.

## Receiver Runs And Open Actual-Diff Finding

Default x64 integration also passes 20/20 (18.49 seconds). The once-per-width
vendor receiver sequence has passed XT x64 (21.42 seconds) and AT x64 (28.59
seconds); remaining results are not claimed yet. The build's existing PE checks
pass for these candidates. These are worktree builds, not a pushed delivery.

Actual source comparison with `579f4e35a` found an additional extraction
difference that the passing suite does not qualify. The old per-byte
`core_machine_fdc_media_offset` validates media geometry, whereas the new
`x86_fdc_record_valid` calls `track_info`, whose provider also qualifies current
recording density/pitch. Changing CCR during an already-started transfer can
therefore terminate it earlier than the accepted implementation. READ ID and
FORMAT also call the same provider twice before deciding the command outcome.
This is an open S13 review item, not an approved new hardware behavior. Separate
record bounds from ID-readability qualification at the existing provider
boundary, preserve command-entry qualification and add explicit regressions
before acceptance; do not cache another media/track owner or add a BIOS branch.
The current passing boot evidence cannot close this item.

## Recording Boundary Correction

A new regression changes recording-channel qualification after command entry.
It fails against the extraction before correction (CTest exit 8), then passes
after separating copied `id_readable` from successful record-bounds queries.
The adapter still owns density/pitch resolution. The chip checks readability
at READ/WRITE/SCAN/READ TRACK/FORMAT admission, but keeps the accepted geometry
checks for subsequent bytes. READ ID and FORMAT now sample once rather than
twice. No media cache, BIOS condition or second track owner is introduced.

The shared regression covers all eight data/scan/read-track opcodes plus
FORMAT, asserting that an admitted operation completes with the original data
and result, while a new command rejects the now-unreadable channel. READ ID
tests both outcomes and exact single sampling. The four independent FDC tests
pass; the complete tools-off suite passes 22/22 (4.08 seconds), with all six
manifests checked again after the source/test updates.

The earlier vendor sequence completed successfully before this correction:
Model 40 x64 61.12 seconds; XT x86 25.37, AT x86 42.45, Model 40 x86 83.97.
These complement the earlier x64 rows, not fresh evidence for final hashes.
Both reusable build trees are back on default. Receiver rebuild/verification
for the final source is still required.

The production-call sweep also found that `core_machine_fdc_advance` had only
test callers. Its replacement read a diagnostic snapshot to increment time;
that convenience now lives solely in the NXVM fixture as a checked one-tick
helper. Production retains only caller-owned absolute-time advancement.
All callers were migrated; no old symbol remains in source or tests.

One verification invocation mistakenly requested both aggregate targets in
one Ninja command. Its owned process tree was explicitly terminated to avoid
overlapping CTest routes, and the complete verification was restarted with
separate sequential build invocations. That interrupted run is not a unit-suite
pass. Final sequential runs, artifact refresh and actual-diff review remain
in progress; no S13 P has been committed.

## Final-Source Verification Refresh

The final x64 specialized aggregate passes after removing a redundant direct
ISO-header include from the test fixture; its declarations already come from
Types. This does not change the gate or production behavior. The complete x64
unit suite then passes 355/355 (205.29 seconds), including the recording-boundary
correction and the removal of the production test-only advance helper.
The corresponding x86 full suite passed 355/355 (37.79 seconds) immediately
before that include-only cleanup; its default integration passed 20/20 (20.32
seconds). These are terminal completed runs, not inferred from live logs.

All receiver artifacts and once-per-width vendor boot checks are now being
refreshed against the corrected source, sequentially, followed by restoring
each build tree to default and running its full external integration suite.
Results and final artifact identities remain pending; S13 is not accepted.

The refreshed vendor sequence has completed XT x64 (20.18 seconds) and AT
x64 (34.81 seconds), each once with its rebuilt embedded firmware target.
Model 40 and the x86 receiver sequence are still running, not assumed passed.
All six source/test manifests pass again after the final source edits.

The staged source/test/build diff, excluding the two generated manifests,
currently covers 32 files: 3,300 added and 2,297 removed lines, net +1,003
(`git diff --cached --numstat`). The increase buys the explicit opaque chip
and provider boundary, independent failure/command matrices and boundary
negative controls; the former App command engine is removed, not retained
behind a forwarding facade. Documentation and EXEs are outside this count.

## Batch Review And Receiver Boundary

The complete FDC ledger row now has one mechanism owner in `x86-fdc8272`.
App `fdc.c/h` are the PC adapter, not another command engine;
`fdc_observation_interface.h` remains the product terminal-observation sink.
The machine scheduler consumes the public due-time contract. Chip allocation
precedes port publication; the existing board checkpoint rolls back failed
registration and destroys the chip before clearing its borrowed connections.
The independent allocation and seven-position port-failure cases exercise this
actual path, including retry, rather than a replacement constructor.

Original case ownership is accounted for by the record tables, cause/seek
suite and retained mixed board matrices described above. Diagnostic boot tools
and the T242 corpus now use copied values, with unchanged completion markers;
the previously false-success T242 exit is no longer possible. Static rejection
of reverse/private dependencies has seven executed negative controls. This
review preserves the accepted flat-record/512-byte and limited READ TRACK
model; extracting it does not certify new silicon or timing coverage.

MyNES receiver review inspected `src/app-mynes/core/CMakeLists.txt` and
`src/app-mynes/product/CMakeLists.txt`: its executable reaches its own driver,
Common, Storage, Base, Types and Audio, but no x86 target. The changed x86
build/test registration therefore affects shared tests, not either MyNES EXE's
inputs. Its existing 0043 pair and user configurations remain unchanged.

## Final Delivery Verification

All final receiver runs completed with exit zero. NXVM units are 355/355 per
width (x64 205.29 seconds; x86 37.79 seconds, with the include-only fixture
cleanup subsequently compiled by the static/full-build route). Default
external integrations are 20/20 on x64 (19.74 seconds) and x86 (18.29 seconds).
Final vendor boot cases ran once each: XT x64/x86 20.18/23.70 seconds,
AT 34.81/41.51 seconds and Model 40 74.65/76.22 seconds. These use the selected
INI, external media and the same compiled firmware objects as their products;
they are boot-checkpoint evidence, not a claim of exhaustive manual UX testing.
Both reusable trees are restored to default configuration.

The complete specialized static aggregate, independent tools-off suite 22/22,
six manifests, documentation governance and whitespace checks pass. Retain the
three bounded `build/t539-s3` trees for the next chip batch; they contain the
reusable toolchains and verification inputs, not a second release location.
No external ROM, user INI, MyNES source or MyNES executable changed.

Eight optimized, stripped `0.5.0539` product EXEs were rebuilt, deployed and
verified as PE 8664 (x64) or 014C (x86). They are the worktree built on
`579f4e35a` plus the complete S13 delivery; the implementation commits identify
that source after submission. The owner-approved embedded-ROM route remains.
Files reside only under their existing `assets/nxvm/<profile>/` directories.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| nxvm_xt_0_5_0539_x64.exe | 1324273 | C6A38711F8A30B5ABBAF101EC24E08C3791CCC6774DA8D27368EE35A0F8E496B |
| nxvm_xt_0_5_0539_x86.exe | 1463200 | 32670E95B57167486D62F82EA9630E2497B6F6E3DD18757EDBAE28916FC4A298 |
| nxvm_at_0_5_0539_x64.exe | 1324339 | A995082B0291B78D13BBE45C72F4FAC3E95E06E586B22189328A08FEF6CF4C61 |
| nxvm_at_0_5_0539_x86.exe | 1463268 | 5BB1326F75B2482026574FF21CE9708EA003D34410739488205924BA8BAF3B29 |
| nxvm_model40_0_5_0539_x64.exe | 1307988 | 7391A4410FE065163AADC2288109E2601E21FD6B02F03DAB5AF750A801F98943 |
| nxvm_model40_0_5_0539_x86.exe | 1446918 | 337EB609AD3517300DF7191DC3CDF117AF60B8FD445FAF5F3A872DF81D628A97 |
| nxvm_default_0_5_0539_x64.exe | 1324305 | 5450A5F1A22BEE4FA92339E0325C9D949FFF9ACE94A76FFA1BF4B123D9CECD42 |
| nxvm_default_0_5_0539_x86.exe | 1463233 | 9473267E0E918C82498011B1441A7F5F45A971ED00A3A887AEFF33CAE76D7992 |

The full FDC ledger batch is implemented and ready for actual-commit review.
T539 retains the CPU/FPU, HDC, video and remaining inventory dispositions;
neither FDC boot success nor this source move closes those rows.
