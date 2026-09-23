# M2 T9: Execution And Management Reliability

## Admission And S Plan

T9 is automatically admitted after T8 closure. It turns the accepted runnable
CPU/debug product into M2's stable no-graphics baseline through four
outcome-bearing stages:

| Subtask | Outcome | Closure proof |
| --- | --- | --- |
| S1 | Complete production media and command state coverage. | Valid/rejected replacement and command-state fixtures preserve the required accepted state. |
| S2 | Complete Common/Core lifecycle and host-failure cleanup. | Real executor scenarios prove repeated run/pause/reset/eject and failure unwinding. |
| S3 | Complete cooked-console input and terminal shutdown reliability. | Queued/partial input and exit paths prove one-reader, bounded cleanup and no extra command requirement. |
| S4 | Reconcile the full M2 ledger and close the milestone. | x64/x86 suites, artifact evidence and an explicit disposition for every M2 acceptance row pass. |
| S5 | Close the remaining ledger cross-products exposed by S4. | Production or shared-owner fixtures directly prove every remaining ROM/APP/HOST row. |
| S6 | Close the remaining storage-fault and host-failure receivers. | Fault injection and a current test-to-ledger map prove every residual ROM/HOST row; then T9/M2 closure is reconsidered. |
| S7 | Reconcile the remaining ROM and App matrix members. | Exhaustive strict-header rejection and command-state/grammar receivers prove every row that S5/S6 did not directly cover. |
| S8 | Make the final M2 ledger decision. | Current evidence maps every row, full x64/x86 regression passes, and T9/M2 closes only if no uncovered member remains. |

## Boundaries

This task repairs product-owned and existing Common integration mechanisms only
where tests expose a defect. It does not add graphics, PPU/APU/controller,
networking, external ROM fixtures or a replacement execution engine.

## S1 Media And Command-State Closure

S1 exercises App -> Common -> Core media operations with original NROM fixture
bytes. A failed replacement retains the copied paused observation; a valid
replacement exposes its distinct reset vector through the same paused debug
route. App command fixtures cover successful stopped insertion/reset request,
failed insertion with no request, running-state rejection and paused ejection
with stop request. Existing debugger command fixtures remain the receiver for
the paused debug grammar. Lifecycle/host cleanup transfers to S2/S3.

## S2 Lifecycle And Cleanup Closure

S2 runs the production Common executor and Core driver through repeated
resume/pause cycles, paused observation, rejected replacement preservation,
valid replacement publication, reset, eject and shutdown. Every pause remains
debug-observable and free of guest traps. The unchanged shared Common suite
continues to qualify its isolated host/synchronization fault paths; S3 owns the
cooked-console reader and terminal exit behavior.

## S4 Audit And S5 Admission

S4 rebuilt both architectures and executed all 76 component/integration
executables per tree. The product formats and hashes are recorded in the final
evidence work. That result is insufficient to close M2 because the original
matrix still names direct ROM flag/length/I/O cross-products, full App grammar
boundaries and host construction/sink/wait/join fault cases without a current
fixture-to-row disposition. S5 is therefore admitted immediately. It must add
or reference direct owner fixtures for those rows; a green aggregate suite does
not substitute for their proof.

## S5 Boundary Delivery And S6 Admission

S5 delivered strict cartridge format/mapping coverage and App media grammar
coverage. The production-path media fixture remains the state receiver. Both
the Core/App/integration subset on x64 and x86 pass. The detailed test-to-row
record is [S5 boundary evidence](../etc/evidence/m2-t9-s5-boundaries.md).

This does not close T9: storage allocation/read/close faults and the HOST
construction/sink/wait/join mapping are still not direct MyNes delivery records.
S6 is admitted immediately with those two implementation outcomes. It may
reuse unchanged Common fixtures by exact name and source revision, but must
record the mapping and run the receiver tests; aggregate test totals remain
insufficient.

## S6 Storage And Host Receiver Delivery

S6 adds a Core-owned storage-boundary test: injected no-memory, I/O and size
failure leave the accepted cartridge in place. It also reruns and records the
unchanged Common construction, sink, native-wait, failed-join and terminal
cooked-session receivers by exact executable. Both x64 and x86 pass. The
complete mapping is [S6 host and storage evidence](../etc/evidence/m2-t9-s6-host-storage.md).
T9 still requires its final all-component dual-architecture regression and
ledger reconciliation before it can close.

## S7 Admission

S6's complete dual-architecture regression establishes its storage/HOST scope,
but the original M2 matrix still requires a direct disposition for every header
truncation, rejected flag/padding byte, and every App command/state member.
S7 is therefore admitted as an outcome-bearing implementation stage. It must
expand production fixtures or record an exact existing receiver for each member;
it cannot close from the prior aggregate result.

## S7 ROM And App Receiver Delivery

S7 expands the strict iNES header fixture to all required byte-value and
truncation members, and completes the App management state/callback receiver.
It also corrects host ERROR handling to allow only status and quit. The direct
unit and production integration receiver map is [S7 ROM and App evidence](../etc/evidence/m2-t9-s7-rom-app.md).
S8 owns the final explicit ledger audit and, only if every matrix row is direct,
the T9/M2 closure decision.

## S8 Final Ledger And T9 Closure

S8 adds the 4095/4096-byte App path boundary and startup-media early-unwind
receiver, then reruns the complete x64/x86 suite. Every M2 row now has a named
direct receiver in the [final ledger](../etc/evidence/m2-final-ledger.md).
T9 and M2 close with no transfer to M3; PPU/APU/controller/presentation remain
separate unadmitted capability work.

## S3 Cooked-Console Closure

S3 reuses the unchanged Common cooked-session implementation rather than
introducing an App-side reader. Its independent session monitor, session queue
failure, frame and input-queue ownership fixtures prove one outstanding reader,
prompt admission only after consumption, partial-edit cancellation before
notifications, queued/rejected lines, terminal exit without a second wait, and
bounded cleanup on write/request/wait failures. The App `quit` result remains
the production terminal request. S4 receives only the full M2 reconciliation.
