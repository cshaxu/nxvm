# T539 S18: CPU Extraction Work Record

Reference baseline 25ec0f6c3. Current owns admission and acceptance. This is
in-progress evidence, not a completed CPU migration or a delivered P.

The owner's 2026-09-29 [S18-S32 work split](../architecture/t539-cpu-work-packages.md)
supersedes the original all-in-one S18 delivery. Earlier sections below are
chronological, including the incomplete opaque-allocation/test cutover; the
incremental recovery at the end records their current disposition.

## Initial Boundary Cleanup

Rechecked all three production and four test firmware-provider initializers:
XT, Model40, external PC/AT, two firmware-capability fixtures, reset-ROM alias
and runner reset-failure fixture. All software-interrupt slots were null.
Removed the unused provider member, copied frame/result types, CPU binding and
helper, board forwarder and their initializer entries. Real-mode INT directly
uses its original `_ser_int_real`; protected-mode dispatch is unchanged.
Configure/reset/after-run, ROM installation and guest INT diagnostics remain.

The sweep `rg -n 'firmware_interrupt|firmware_software_interrupt|_e_try_firmware|firmware_handle_software_interrupt' src test`
has no remaining matches. Legitimate guest software-interrupt timing and
diagnostic counters are not host firmware hooks and were not removed.

The x64 full build passed, followed by 370/370 complete units in 208.48 seconds.
This result applies to the interception removal before rebuilding the board
function move; it is not the final S18 source/receiver proof.

## Board Timing Ownership

Moved the existing transaction trace, DMA-handoff invalidation, external-cycle
matching, wait-window and page-locality functions from cpu_timing_model.c to
NXVM cpu_bus.c. Function bodies compare exactly with the baseline, ignoring
only line endings; the redundant forward declaration is removed because
machine.h already declares it. CPU formulas remain in their original file.
The single core-machine source list includes cpu_bus.c; the observable test
target derives the same list rather than maintaining another implementation.
The x64 full build after the move passed. The nine affected transaction,
arbitration, prefetch, competition and timing regressions pass in 9.95 seconds.
Receiving execution is still pending
for the complete CPU extraction, as are x86 and all deployed artifact updates.

## CPU Bus Cutover

The execution context no longer stores RAM, port, PIC-pair or transaction
pointers. Its borrowed bus provider handles physical transfers, port admission
and completion, interrupt sampling/acknowledgement and extension notification.
NXVM cpu_bus.c owns the former board operations. Port reads still publish the
CPU operand before transaction and external-cycle commit; failed transfers do
not call completion. Memory provenance, reset-fetch identity, preview intent,
address-width masking and the original cancellation order are preserved.

The existing CPU context fixture now exercises all five implemented families
without RAM/port/PIC implementations: address width, observational decoding,
reset fetch, byte/word and supported dword port transfers, result-before-commit
ordering and interrupt entry. It passed directly after the incremental x64
build. The complete x64 unit suite then passed 370/370 in 199.90 seconds.
This full-suite proof precedes the rejected port and interrupt-acknowledgement
cases and the three descriptor timing inspections changed to the same bus
observation callback. The subsequent complete x64 build passed, followed by
14/14 affected CPU context, preview, call-gate, transaction and timing
regressions in 11.87 seconds. Rejected transfers preserve the operand and do
not call completion; failed acknowledgement leaves the IRQ pending and stack
untouched. Documentation governance and diff whitespace checks pass. These
remain intermediate proofs, not complete S18 receiving acceptance.

The timing source no longer directly names executor_memory or the memory
inspection function. Its remaining machine-shaped state and diagnostic/private
CPU consumers still require the following extraction; the bus cutover alone
does not complete S18.

## Copied Fault Diagnostics

All seven exception/fault publication sites now build the existing fault
snapshot inside the CPU before calling the diagnostic consumer. The source
CPU and old-instruction identity, register fields, exception masks/codes and
publication positions are unchanged. NXVM keeps first/last retention, counting
and machine fault reporting, but no longer reads private state in these two
callbacks. The instruction callback was migrated in the subsequent step below.

The CPU context fixture retains the copied fault value after CPU state changes
and checks the port failure's exception, origin and register value across all
five CPU families. The complete x64 build passes. Twelve selected unit cases
cover fault retention, real/protected exception delivery, TF/debug traps,
hardware entry, interrupt return and the new fixture; all pass. The selection
also unintentionally matched three integration cases because `fault` matched
`default` and no unit label was supplied: they passed, but the invocation used
four-way concurrency rather than the packet's integration serialization. It is
not final integration acceptance. Current now limits this transient selection
to the unit label; final integration remains serialized and pending. The whole
15-case invocation took 15.19 seconds. No claim is made that the preceding
370-unit result covers this newer diagnostic change.

## Instruction Observation And Debug Ownership

The instruction callback now carries a bounded copied observation rather than
CPU/decoder pointers. It contains only the execution point, operand/mode facts,
the previous EIP/default size and the register values used by existing branch
and I/O observations. The execution point retains its original fetch-window
meaning. Retirement assembly uses the same copied CPU operation after execution;
board sequence/time/qualification publication remains outside the CPU. Removed
the duplicate point-copy helpers from board debug and retirement code. Null
instruction callbacks still avoid snapshot construction.

Moved the existing segment snapshot, register read, instruction/access snapshot
and candidate-register patch mechanisms into CPU implementation. Their copied
types move unchanged from the board debug header to the CPU contract. Board
debug entry points retain lifecycle admission and delegate the actual register
operation; they no longer access private CPU/decoder data. The patch's existing
candidate validation and publication order are retained, not replaced by
per-register board setters.

The first complete x64 unit run after instruction-observer migration passed
369/370 in 208.92 seconds. The new CPU fixture incorrectly expected decoded
instruction length in the preserved 15-byte fetch-window byte_count. The source
assignments confirm the old value is 15 on successful fetch; the test is corrected
to that contract, not the CPU changed to satisfy it. The subsequent debug
migration and added direct CPU debug cases pass the complete x64 rebuild and
all 17 selected unit cases in 11.98 seconds. The subsequent complete x64 unit
run passes 370/370 in 184.45 seconds, including the corrected fetch-window
fixture and CPU-local register operations. Documentation governance and
`git diff --check` also pass. This proves the current intermediate source,
not the pending opaque-instance/timing migration or final S18 acceptance.

## Remaining Caller Audit

The resumed caller audit distinguishes raw-layout access from already bounded
operations. `machine_scheduler.c` only advances CPU prefetch reservation through
the existing operation; `memory_interface.c` only invalidates prefetch after a
successful write. They need opaque-instance rebinding, not new mirror state.
The PC/halt/NMI accesses found in `machine.c`, `machine_board.c` and
`trace_interface.c` are migrated in the signal step below. Entry-plan
construction still copies the entire
CPU and validates segments before publishing it, so its atomic validation order
must survive the replacement. The whole-machine timing dependency is removed
in the step below; opaque ownership and standalone catalogs remain unfinished.

## CPU State And NMI Admission

CPU operations now own the current execution-state copy, linear PC, halt query,
external NMI mask input and NMI admission. The board retains parity/D4/XT source
latches and refresh-on-unmask policy; it marks a source signaled only after CPU
admission. No additional persistent state is introduced. Masked requests remain
rejected; an already admitted pending request survives a later mask and is
delivered after unmasking. CPU interrupt handlers are unchanged. Trace consumes
the CPU PC operation and its duplicate PC helper is removed.

The five-profile context fixture verifies copied state lifetime, masked
admission, unmask without a fabricated edge, repeated admission coalescing and
NMI delivery waking HLT without PIC acknowledgement. The complete x64 rebuild
passes, followed by 10/10 selected unit cases in 8.42 seconds covering the
fixture, board parity/RTC/XT/D4, HLT, hardware delivery and trace/retirement.
The preceding 370/370 proof predates this signal step; final full receiver
verification remains required after the complete extraction.

## CPU-Local Timing Ownership

Both timing translation units now accept the CPU execution context instead of
the entire board. CPU owns the immutable resolved recipe, repeated-instruction
identity and last timing result; those fields are removed from `core_machine`.
NXVM retains external wait accounting, retirement qualification and the sole
time advancement path, consuming copied timing results. CPU reset clears its
repeat identity; board reset no longer edits it. Formula provenance/repeat
enums and the recipe/result value contracts move to the CPU header without
changing their existing values.

A mechanical comparison of all 4,267 lines of the current timing-model unit
against this step's input confirms only the declared owner/type/member
rebinding: no formula, constant or branch-body change. Selector publication
now saves one CPU-owned result, including the previous separate form/repeat
fields. An existing 80286 test's private machine field read is replaced with
the copied CPU timing operation; its XLAT expectation remains unchanged.
The first build exposed this stale test access, which was corrected; the
subsequent complete x64 rebuild passes. Added direct five-profile CPU coverage
interleaves two REP executions to check independent FIRST/CONTINUATION state,
copied result lifetime and reset. The full x64 unit invocation passes 369/370
in 217.77 seconds; its sole failure is the stale gate described below. The
corrected gate then passes alone in 6.66 seconds. All 370 cases have passed
against this step, but this is not represented as one clean full invocation
or final cross-width/standalone S18 acceptance.

The running suite identified a stale source-spelling assertion in
`Verify-8086DecoderLedger.ps1`: its POP CS timing guard still required
`machine->cpu_profile`. It now requires `context->cpu_profile` with the same
8086 predicate; XLAT/POP CS source rules, key counts and runtime-result checks
are unchanged. The five timing-manifest runners, direct interleaved CPU
fixture and all behavioral tests pass without changing expected timings.

Catalog audit also found that the production key-string array is referenced
only by its 4,906-entry static count assertion, not by runtime timing selection.
The metadata catalog is genuinely consumed by the four timing-runner sources
(five CPU profile targets). The standalone cut must distinguish that test
input from runtime formulas, rather than importing a needless production
dependency on App documentation or a second canonical catalog.

The unused production key array and its build dependency are now removed.
The same 4,906-entry assertion checks the actual metadata array in the common
8086/8088 runner source. The exporter emits only the metadata consumed by the
five runners; generated include directories are private to those targets,
not transitive Core include paths. No timing formula or manifest row changed.
The complete x64 rebuild passes, followed by all five timing-manifest runners
in 7.51 seconds. Searches across src/test/tools/cmake find no retired key-array,
key-catalog include or MetadataOutPath usage. This is local migration proof,
not the final S18 full-suite or independent-component acceptance.

## CPU Allocation Migration

CPU now has one private allocation containing its execution context, registers
and decoder, with create/destroy operations. The new five-profile fixture uses
operations and copied state to verify reset identity, independent stop requests
and destroying one instance without affecting the other. Machine production
construction has not switched to this allocation yet.

An attempted bulk pointer conversion was rejected by tool safety review because
it would retain private CPU reads in many board tests. The six partially applied
files and the machine-layout change were reversed; their integration/runner
diffs are absent again. No rejection workaround was used. Migrate remaining
consumers by actual responsibility and existing public operations before the
production allocation switch; temporary chip allocation work is not an accepted
second production path. The complete x64 rebuild passes, followed by the CPU
context, board CPU/PIC lifecycle, executor-storage and session-isolation tests:
4/4 in 3.78 seconds. This is local construction proof, not final opaque-boundary
or full receiving acceptance.

## KBC Reset Wiring

The NXVM KBC attachment held a CPU execution pointer solely to request reset.
It now emits a reset callback/context bound by machine composition to the
existing CPU request operation. KBC no longer includes CPU headers or names
CPU state. Output-port bit zero and pulse commands retain one reset route;
A20, IRQ wiring, keyboard algorithms and timing are unchanged. No Shared API
or second reset latch is introduced.

All four binding callers were updated. The controller fixture counts reset
pulses directly: FF produces none, FE one, and D1/output zero a second pulse.
Aux/FIFO fixtures no longer construct an unused CPU context. The production
reset-ROM-alias test retains end-to-end CPU reset coverage. Complete x64 build
passes; six KBC/reset/CPU-context units pass in 2.46 seconds. Searching kbc.c/h
finds no CPU include, execution type or former connect.execution field.

## Entry Candidate Ownership

Entry-plan preparation previously copied and committed `t_cpu` in board code.
The unchanged segment-load order, register assignments and FLAGS predicate now
live in CPU behind a short-lived prepared-entry handle. Board route/preload
checks retain their order; invalid routes/ranges/overlaps discard the candidate,
and successful validation commits CPU before the original preload writes.
No second boot route or persistent register mirror is introduced. A candidate
allocation failure returns NO_MEMORY before register/memory publication.

Original entry-plan atomicity, mantle and AUX guest cases remain unchanged.
The CPU fixture adds preparation-without-publication, discard, commit, instance
isolation and invalid-FLAGS coverage for all five profiles using copied state.
The complete x64 rebuild and four applicable units pass (2.24 seconds).
`entry_plan_interface.c` no longer names `t_cpu`, decoder storage or the raw
executor CPU. The inactive CGA system source still uses the same unchanged
board entry-plan API; it is not claimed as an executed CTest case.

## Public Header Boundary And Consumer Migration

CPU operation declarations and copied value types now live in cpu_interface.h;
register/segment layout stays in cpu.h and decoder/context layout stays in
cpu_instructions.h. Private timing evaluators remain in cpu_timing.h. The
machine and retirement adapters no longer include that private timing header.
This changes declaration ownership, not instruction bodies or timing formulas.
The complete x64 rebuild succeeds and the full unit suite passes 370/370 in
206.07 seconds. This is current x64 proof, not independent/cross-width acceptance.

The Windows INT13 probe now reads the required registers through the existing
paused-debug API and gets PC through the existing copied CPU-state operation. It no
longer includes machine.h or accesses the CPU layout. Its original AH/DL,
carry, geometry and successful-read checks are retained; read failures fail
the existing probe path rather than supplying fabricated values. Removing the
private header also exposed the old private linear-PC helper; that caller now
uses cs_base plus eip from the copied state rather than exporting the helper.
The complete incremental x64 build passes, followed by the unchanged registered
INT13 integration case (1/1, 0.57 seconds). The full unit result above predates
this integration-only rebuild; no unit or production input changed afterward.

The subsequent integration cleanup migrates the Windows Setup fault report to
the existing copied debug snapshot and removes its cross-test raw-CPU fixture
dependency. The floppy matrix reads its seven diagnostic registers through the
existing paused-debug operation. Its report is already called only after a
successful pause; no machine lifecycle or success condition changes.

Fault-mask definitions move unchanged from the private decoder header to the
public CPU diagnostic contract that reports them. No number or handler changes;
the DOS MEM probe now includes only that public CPU header. The complete x64
build succeeds. Four CPU context/fault/UD units and the original DOS MEM and
default floppy-matrix integrations pass 6/6 in 10.15 seconds. Windows Setup's
standalone probe is built but has no registered case in this build, so no
separate runtime execution is claimed for its diagnostic report change.

Remaining integration private-CPU consumers are the BYOB boot probe and Model40
retirement capture. These remain S18 work, not an approved public-layout
compatibility route. Production machine.h still embeds CPU/context storage,
and its final opaque allocation cutover is pending. The previous 370-unit run
is not final proof for the latest declaration and integration-only changes.

## Retirement Register Copies

BYOB and Model40 retirement callbacks formerly read both live CPU registers and
the decoder's saved oldcpu. These are different observation points; replacing
either with a paused-only debugger read would reject normal running callbacks.
The CPU's existing copied architectural snapshot now includes general registers,
IP and FLAGS and can select current or instruction-entry state. The board
publishes both copied values in the existing retirement event, only for an
installed observer. The decoder remains the sole saved-entry owner. The
per-instruction eligibility initializer clears only the original event prefix;
the larger register copies are initialized at publication, not on an unobserved
instruction. Paused debug admission remains unchanged.

The two retirement callback bodies now consume those event values, including
the original current/old stack distinctions, low-word casts and segment-size
attributes. Their checks, addresses and counters remain unchanged. Memory-write
callbacks and after-run diagnostics still need migration; this is not a claim
that either complete probe has reached an opaque CPU boundary.

The complete x64 rebuild passes. Five-profile CPU snapshot tests prove entry
versus current AX/IP, copy independence and rejection of an invalid selector.
The board retirement test verifies pre/post CR0 mode transition and IP values.
Those tests plus x86 debug mapping pass 3/3 in 2.73 seconds. The original paused
debug and unified-debug integrations pass 2/2 in 0.43 seconds. One default BYOB
run with retirement observation enabled reaches `BOOT-PROBE=dos-prompt` and
`A:\>` within its existing 25-second short budget (process 3.33 seconds,
exit zero). This is default callback-path proof, not a vendor boot-matrix claim.

## Model40 Stopped Diagnostic Reads

The Model40 capture main loop now obtains current and instruction-entry values
through the existing paused debug snapshot operation. Its selector is passed
to the CPU owner; PAUSED/STOPPED/FAULTED admission is unchanged. Both existing
production/Windows diagnostic callers explicitly request current state. The
probe installs its write watchpoint through the existing bounded operation,
not decoder fields. Final output takes a fresh current snapshot and only emits
it on successful capture. NMI/interrupt stack source selection, low-word casts,
diagnostic markers, execution budget and terminal checks are preserved.

No executor_cpu, executor_cpu_instructions, t_cpu or t_cpuins dependency remains
in this Model40 file. Its remaining private board reads are RTC/A20/physical
memory diagnostics, not CPU layout access. BYOB's separate after-run/memory-write
consumers still require migration.

The complete incremental x64 build passes (394 actions), including the Model40
capture executable. Four CPU/retirement/debug units and two existing debug
integrations pass 6/6 in 3.76 seconds. The retirement test now checks that both
snapshot selections reject running callbacks, return the matching pre/post
LMSW values after execution stops, and reject an invalid selector without
altering the output. Its EAX setup also uses the public register operation.
This step does not rerun or claim a Model40 vendor boot; final packet-wide
verification and artifact delivery remain outstanding.

## BYOB Stopped Diagnostic Reads

BYOB fault, HLT/no-progress, final register/stack and descriptor-table reports
now read the existing paused current snapshot instead of t_cpu. Capture occurs
only when diagnostics need it, not on each execution quantum. The original
low-word register operations, stack arithmetic, output markers and success
conditions are retained. A failed final capture reports unavailable and fails
the probe instead of printing zero/stale registers; cleanup still runs.

The incremental x64 build passes. One default BYOB invocation with the original
short budget reaches DOS prompt and exits zero in 3.52 seconds; final register
and stack reports are present. This is not vendor or full-suite qualification.
The only remaining direct CPU-field reads in this file are two running memory
write observers, so the probe is not yet claimed public-only.

Inspection also finds that the small vector observer is registered after the
explicit reset through a configuration-only API, with its rejection ignored.
Reset requires frozen providers, while that registration requires INITIALIZED
and unfrozen state. Model40 separately installs its live detailed observer in
the board memory observer list. Reconcile these actual paths before replacing
their PC attribution; a retirement record or paused read cannot substitute for
the synchronous write-time CPU position.

## CPU Allocation Cutover And Remaining Test Migration

The rejected-after-reset duplicate vector observer and its ignored registration
are removed. The live Model40 write observer now captures copied CS/IP through
the CPU's existing public operation on the synchronous board callback stack.
It keeps the same write-time position, independent of retirement observation.
Five-profile bus tests execute a memory-writing MOV and verify the consumed
operand PC and copy lifetime. No running machine-debug admission was added.

The DOS video integration's last raw CPU fixture use is replaced by the existing
paused EAX read, retaining INT10 AH accounting and its original success checks.
No t_cpu/t_cpuins or executor_cpu/instructions layout reference remains in the
integration C sources. Board memory/controller diagnostics remain board-local.

Production machine now owns one opaque CPU allocation: create failure releases
the already-created FPU and machine; normal/error teardown destroys the CPU.
The embedded register/decoder/context layouts and CPU-private header includes
are removed from machine.h. All production bindings use the opaque handle.
CR0 masks exposed by copied snapshots move unchanged to the public CPU header,
removing their duplicate definitions; instruction algorithms are untouched.

The production Core target builds, followed by the production adapter/profile
libraries and three migrated probes (65 actions). Newly rebuilt CPU-context
and DOS-video cases pass 2/2 in 3.81 seconds. One bounded default BYOB run reaches
DOS prompt and emits its final register/stack reports. The earlier four-case
result (4/4, 0.48 seconds) predates this allocation cutover.

This is an intentionally incomplete worktree transition, not S acceptance:
112 unit/fixture files still reference the removed CPU fields. They require
owner-based migration, not a private-layout pointer alias. The full suite is
not currently claimed buildable; do not use stale test executables as proof.
Independent Shared placement, full receiving tests and artifacts remain pending.

## Board Test Consumers After Opaque Allocation

Six board regressions now use the owned CPU handle or existing copied/public
operations, without a private-layout alias or additional production API:

- CPU/PIC lifecycle checks retain instance identity across reset and execute
  memory fetch and PIC port I/O instead of comparing internal callback pointers.
- Memory inspection sets paging through the existing register patch and retains
  original parity, overlay, reset-fetch and page-table side-effect assertions.
- Adapter storage checks reset registers and independent register read/write;
  debug mapping checks read-watchpoint installation/removal through its existing
  query rather than decoder fields.
- PIC phase retains INTA-before-stack-write, handler entry, IRR/ISR and reset
  transaction checks. Its old ordering predicate could pass with no INTA pair;
  an explicit missing-pair rejection now enforces that original requirement.
- KBC retains the real 286 FF/FA/BAT/IRQ1/STI-shadow sequence. Reset applies the
  existing CPU register patch on the reset callback stack; after-run EIP uses
  the machine's copied state. The redundant second reset-entry preparation is
  removed, not replaced by a hidden private-state fixture.

The four initial migrations passed their eight-action x64 rebuild and 4/4 tests
in 4.64 seconds. PIC and KBC then passed their own rebuilds; the combined six
freshly built board regressions pass 6/6 in 0.50 seconds. This is migration
evidence only, not a full-suite or cross-width acceptance claim. The old mixed
CPU fixture and remaining raw-state tests still prevent complete extraction.

## Board Fixture Ownership And XT NMI Migration

The old CPU fixture also owned seven board-only helpers: lifecycle bind/reset,
construction tail, reset physical mapping, memory provider registration, PIT
divider wiring, port read and memory-route query. Their bodies move unchanged
to `support/core_machine_board_fixture.h`; the old header includes that owner
instead of retaining duplicate definitions. An exact normalized body comparison
against HEAD confirms all seven are unchanged. Twenty-six consumers select the
board header. Twenty-five rebuild and pass their original registered cases
(25/25, 24.55 seconds); prefetch-locality remains a mixed-owner case, not an
accepted migration. No test or production API is added by this helper split.

XT PPI parity/IO-check tests now execute code while the source or NMI is masked,
then prove vector-2 handler execution after enable/unmask. The existing PC7/PC6
status, source latch and reset assertions remain. They no longer read CPU's
private NMI pending flag. The rebuilt XT case passes (0.91 seconds).

The CPU-owner context test now checks reservation creation, advance, cache
preservation, invalidate and reset across all five existing profiles. It
preserves the original 386 internal assertions and adds the byte-queue variant
without any machine dependency. CPU-context and XT jointly pass 2/2 in 0.10
seconds after their rebuilds.

Prefetch-locality's five board timing cases now call the same board-owned
external-cycle function directly, rather than retrieving it through CPU's
private context. Its final private reservation/board-HOLD scenario still needs
owner reconciliation and remains unbuildable after the opaque cutover. An
attempted replacement using actual 8088 memory-read grants was not equivalent:
the tested legacy NOP/INC handlers advance IP directly without establishing the
sequential marker consumed by the reservation producer. The temporary probe
was removed and the original assertion body retained; no opcode, timing or
production behavior was changed to make this test pass. Reconcile the complete
prefetch producer/caller family before accepting its replacement, not just this
one observed path. The successful chip-local cases do not prove board grants.

## Remaining S18 Work

The mantle and two-instance isolation tests no longer include the raw CPU
fixture. Mantle retains the same IF-enabled HLT entry plan. Isolation retains
board-resource identity and distinct memory/EAX values, and checks read-watch
enable/address through the existing debug contract on both instances. The
unused embedded-storage coherence and isolation helpers were removed; no new
production API was added. Both x64 targets rebuilt and their exact registered
unit cases passed (2/2, 1.49 seconds). An initial underscore-name filter matched
no tests and is not counted as execution evidence. Full S18 verification remains
outstanding.

The mixed prefetch test is now split by ownership. The existing chip-local
five-profile cases retain reservation address/count, consumption, cache
preservation, invalidation and reset assertions. The board locality test counts
the existing outgoing CPU grant, forwarding every call to the real CPU. Its
test-only object compiles the unchanged scheduler source with that one callee
renamed; it neither duplicates the scheduler nor adds a production callback or
CPU field accessor. This uses ordinary CMake object linking, not a GNU-only
linker wrap. The board test proves one initial grant, no grant during DMA HOLD,
no grant during a pending D4 refresh, resumed grant on the following tick, and
grant availability after reset. Existing external-cycle locality cases remain.
Both rebuilt x64 cases pass (2/2, 1.29 seconds). The first build invocation used
an incorrect CPU target name and failed before compilation; it is not counted
as proof. This resolves the mixed-fixture boundary, not the wider legacy
instruction prefetch-producer investigation or whole CPU extraction.

The next machine-layer caller batch uses existing register patches for reset
entry preparation and copied registers/snapshots for post-run assertions.
Default AT retains its four FDD formats, CMOS checksum and 80186 refresh poll;
5170 retains topology, native/compatible/rejected media, refresh polling and
calibration, DMA page word-I/O and external ROM mapping assertions. PC/AT reset
still proves timeline progress and reset/NMI restoration. Fault-outcome setup
now executes LIDT with limit 0017h before the original invalid D6 opcode instead
of mutating private IDTR state; its original fault identity, lifecycle and reset
assertions remain. All four rebuilt x64 cases pass (4/4, 2.51 seconds).

The unregistered legacy no-media video source now reads EAX to obtain AH,
rather than copying a private CPU; it is not included in that runtime count.
Search of test/app-nxvm/unit/core/machine finds no remaining raw executor CPU
member accesses, t_cpu, cpu_instructions.h or CPU fixture includes. This is a
directory-specific boundary result, not whole-corpus acceptance. The five
changed test files total +81/-60 lines against HEAD; production is unchanged.

Retirement-observation and 8087 CPU/FPU composition tests now use existing
register operations and copied snapshots instead of private CPU/decoder state.
An initial EFLAGS export was redundant with the existing public debug masks;
that export was withdrawn and the consumer uses CORE_MACHINE_DEBUG_EFLAGS_*
instead. Original VCPU_EFLAGS_* definitions remain private and unchanged.
Retirement timing values, profile/form cases and callback lifecycle
rejections are unchanged; FPU arithmetic, stack/reset, exception frame and
profile cases remain intact.

The first FPU migration used the machine debug API inside its reset callback,
which correctly rejected the still-initialized machine. The callback now uses
the existing CPU public patch operation on the board's opaque CPU handle,
matching other serialized reset owners. Machine debug admission was not relaxed.
Reset errors and per-case failures are now reported rather than hidden. Both
rebuilt x64 cases pass (2/2, 1.45 seconds) after this correction; initial failed
runs are retained as diagnosis, not acceptance evidence.

The real-mode #UD delivery fixture now executes LIDT on both 286/386 instead of
writing the private IDTR. It then uses the public register patch to establish
the original faulting PC, stack, EAX and FLAGS. Copied snapshots retain PC,
stack, general-register, FLAGS and CS/SS rollback comparisons, while the stack
frame mask is the unchanged complement of the original reserved-bit mask.
All five opcode/form/profile/LOCK cases and the undersized-IDT delivery failure
remain. The fixture and retirement observation pass together (2/2, 2.46 seconds).
No opcode implementation or public API was added for test preparation.

The protected #UD fixture also no longer imports CPU/decoder private headers
or the raw CPU fixture. It loads the unchanged GDT/IDT with guest LGDT/LIDT,
then uses the existing CR0 and segment/register patch operations to enter the
original 16-bit protected code and stack. Copied snapshots preserve GPR,
data-segment, FLAGS and PC/stack rollback assertions, 32-bit interrupt frames,
and subsequent handler HLT. All five invalid forms and invalid-gate failure
remain. A direct HEAD/worktree comparison confirms the complete metadata and
lexeme test-function region is unchanged. After rebuilding the affected batch,
protected/real #UD, retirement observation and 8087 composition pass together
(4/4, 2.47 seconds). This does not establish full CPU corpus buildability.

CPU no longer includes App device_support.h. Its used arithmetic/bit notation
now lives privately in cpu.h as X86_CPU_* macros with the original expansions;
instruction handlers, tables and formulas retain their structure. No Lib type
or public API was introduced. The board helper remains for its board/test
consumers and must be swept again after those tests migrate.

Mechanical replacement touched 1,366 lines (cpu.h 92, cpu.c 1,
cpu_instructions.c 1,261, cpu_timing_model.c 12), plus 31 private definition
lines. To distinguish the textual diff from executable changes, the current
x64 GCC preprocessed each translation unit before and after with
`-E -P -std=c11 -I src`; LF-joined UTF-8 output SHA-256 matches exactly:

- `cpu_instructions.c`: `928E36730B6AAD1457AFCB496E43ADCB482823E3B08CB9E01E19B90124763694`
- `cpu_timing_model.c`: `43E9A822A745D77C032112B88A9894C36B4ED94A972B81E645E7D19D9116F737`
- `cpu_timing.c`: `A3494F043E29FCBA6CE594A0579BCAB9489340897DD7EDC8F213DDEE52E33D1C`
- `cpu.c`: `64F09FCDC7C8877A1E88228EA995EAD3A0E30C274284212E887BE461CCC67F08`

The six rebuilt CPU-context, retirement, FPU, real/protected exception and
prefetch cases pass (6/6, 5.69 seconds). This proves that macro dependency
cleanup and that batch, not complete standalone CPU extraction or all tests.

CPU execution and internal timing now compile once in `x86-cpu`, rather than
execution in the board executor and timing duplicated in normal/observable
machine libraries. Its only declared links are Types and the FPU capability;
the context test links just the CPU/FPU archives, with no machine, board or
Common archive. The generated Ninja link statement was inspected directly.
Timing retains its prior strict compiler options; legacy execution warning
classification is unchanged. The target still uses the original App paths
until the consumer migration and final Shared relocation, so this is independent
link proof, not a claim of completed Shared packaging.

CMake regeneration exposed a stale #UD inventory classification for the runner
fault test already migrated to guest LIDT. Its terminal marker now identifies
that real instruction setup instead of requiring the removed private fixture
call. The limit of 0017h still excludes vector 6 and the original fault,
lifecycle and reset assertions remain. After regeneration and rebuilding,
seven affected x64 cases pass (7/7, 6.33 seconds), including the isolated CPU
context and the runner fault path. Full-suite migration remains incomplete.

The CPU fault-diagnostic fixture now establishes the unavailable vector 6 with
guest LIDT before its measured sequence. The original NOP count, fault PC,
fault bytes, result and history-disabled assertions remain; private CPU and
memory-header dependencies are removed. CMake regeneration accepts its explicit
guest-LIDT terminal classification.

The mixed CPU-preview fixture now uses existing public register operations,
copied PC and the opaque preview operation. Its limited-fetch setup loads an
actual GDT and 16-bit code descriptor at 00fffffch instead of assigning hidden
CS.base. Setup checks the resulting base and IP. The same truncated memory
window, unavailable lexeme, unchanged board observation, transaction counts
and trace count are asserted. Taken Jcc, reset alias and CR-MOV quirk cases
remain; no production CPU algorithm or API changes were required. The entire
1,454-line lexeme-only function region compares byte-for-byte (normalized
line endings) with HEAD. It remains to be rehomed with the chip at final move.
Both tests rebuild and pass on x64 (2/2, 2.90 seconds); this is not full-suite
or S18 acceptance. The 80186 artificial IDTR-limit fixture cannot use LIDT;
its chip-local disposition remains required rather than adding a test-only
public setter or silently weakening its negative assertions.

The 286 protected-mode corpus now uses public reset/register patches and copied
CPU snapshots. Guest LIDT establishes the same terminal IDT limit before its
negative programs; protected-entry, invalid selector/nonpresent code/stack,
#SS/#TS handler frames, protected LIDT, configured IDT and rejected 386 opcode
assertions are retained. The original 80186 LGDT rejection moves to the
CPU-owned context fixture, keeping its exact opcode, GDT/pointer and artificial
IDT limit. It asserts the terminal #UD and fault snapshot without exporting an
IDTR setter or loading a nonexistent 80186 instruction. Thus the original ten
scenarios are nine board cases plus one independent chip case, not nine cases.

The first migrated run caught lost implicit handler continuation in the two
delivery cases: the old CPU fixture had macro-replaced core_machine_run.
Its unchanged two-round helper now lives with the board fixture and this test
calls it explicitly; its obsolete compile definition is removed. A normalized
HEAD/body comparison verifies that helper's executable body is unchanged.
Both rebuilt test targets pass (2/2, 1.09 seconds). No CPU algorithm change or
relaxed frame/PC expectation was needed. Remaining corpus consumers and final
two-width/full-suite proof are still incomplete.

The 386 paging corpus now separates its chip-only control cases from board
page-table, transaction, permission and cross-page cases. Five CR rejection
rows and seven INVLPG rejection rows move unchanged (apart from helper names)
to the independent CPU fixture; normalized HEAD comparisons confirm both
matrix bodies. INVLPG still compares the complete private CPU before/after.
The mutable-CR0 case also moves there so its six complete segment-cache
comparisons are preserved, rather than narrowed to public snapshot fields.

The remaining seven board groups use public register operations and copied
snapshots. Delivered #PF loads its IDT through guest LIDT; halted re-entry uses
the existing CPU-owned prepared-entry contract. Permission preparation loads
real GDT descriptors before installing restrictive page entries and checks
the resulting base, limit, privilege and IP. The original page A/D, data,
stack rollback, fault-frame, CR3 reload, stale-translation, cross-page and
transaction-provenance expectations remain. No production API or instruction
algorithm was changed. The board file no longer includes the private CPU
fixture or dereferences CPU layout. Its obsolete terminal-#UD inventory entry
is removed; the independent CPU fixture retains that inventory responsibility.

After correcting a test context member typo, both final sources rebuild and
pass on x64 (2/2, 0.43 seconds). The accidental test run following that failed
compile used old executables and is explicitly not evidence. This remains a
partial corpus migration, not whole-suite buildability or S18 acceptance.

The D4 platform test no longer reads CPU pending-NMI fields or manufactures
HLT. IOCHK masking is proven by ordinary execution while masked and actual
vector-2 handler execution after unmasking. Failsafe likewise delivers the
handler; its real HLT supplies the shutdown-arbitration precondition. The
original no-time-advance, retained failsafe latch, reset state, port, speaker
and refresh expectations remain. Reset also proves no stale NMI by executing
the ordinary NOP. The same existing XT instruction/vector fixture is moved
unchanged (apart from naming/linkage) into the board test support header so
the two board sources exercise one observational contract.

Real-mode corpus and 386 address-size tests remove their remaining private CPU
headers; the carry assertion uses its existing public debug mask. Four rebuilt
tests pass (4/4, 3.26 seconds). All 32 direct consumers of the updated board
fixture then rebuild and pass together (32/32, 27.55 seconds). This does not
include still-unmigrated consumers of the legacy CPU fixture. The similar-issue
scan still finds private NMI accesses in hardware-delivery, interrupt-entry
and protected-16 delivery tests; those remain in this S18 migration, not a
waived boundary or a completed full-suite claim. No production API, chip
algorithm, Shared corpus, executable artifact or INI changes in this step.

The real-mode final-exception test now establishes each IDT limit with actual
LIDT and uses public entry patches/snapshots. Both original cases remain:
INT 0F outside the table delivers #GP through its valid entry with the same
stack/FLAGS frame, while an unavailable #GP entry preserves IP/SP/FLAGS and
reports the original terminal fault. The known CF/IF input is explicitly
checked; expected saved FLAGS remains 0203h without importing a private mask.

The six real-mode tick cases also retain their instruction/result increments.
Only the terminal 286 prefix case needs a guest LIDT preparation. Its cumulative
elapsed baseline is captured after setup; the failing instruction must still
retire zero instructions and add zero ticks. The first run detected that the
run result's elapsed_ticks is cumulative, not a per-call delta. Correcting that
test baseline preserves the original zero-increment constraint; production
timing is unchanged. CMake regeneration accepts the existing guest-LIDT marker;
the T331 construction gate and both rebuilt x64 tests pass (2/2, 1.85 seconds).

The protected interrupt-entry fixture is also included by software-INT and
hardware-delivery tests, with the latter additionally including VM86 delivery.
Those dependencies must migrate as a coherent group; their artificial segment
cache and pending-event assertions are still unresolved in this active S.
This step does not claim to have migrated that group or restored full-suite
buildability.

The existing CPU bus fixture's 166-line implementation is moved unchanged into
test support and reused by the EFLAGS-local corpus. That target now links only
x86-cpu, not the board library. All 23 original instruction/profile cases and
their flag/GPR expectations remain; the SAHF EAX check now compares against
the original input rather than another post-execution copy. Removed construction
guards no longer wrap infallible stack-fixture setup. The 386 REP test remains
board-owned and retains all three CMPS/SCAS/segment-override programs and result
checks, consuming public CPU snapshots instead of private layouts.

The final x64 rebuild and these three tests pass (3/3, 0.20 seconds). The first
compile caught a missing CURRENT snapshot selector; no stale executable was run
as evidence for that failed build. HEAD-relative counts for the two migrated
corpus files are +71/-93; the bus fixture is a move, not a second implementation.
No production behavior/API change is made here. Final Shared relocation,
protected-interrupt dependency migration and full cross-width proof remain open.

The interrupt-entry corpus now compiles without CPU private headers or layouts.
Preparation executes real LGDT/LIDT, loads valid CS/SS through public CPU register
operations, then restores each intentionally invalid descriptor-table image.
Thus a cached segment remains distinct from a later table edit, including the
original zero code limit and short stack cases. Protected LGDT extension uses
the readable CS override, not the deliberately null DS. Delivery/handler rounds
are called explicitly; the old target-level fixture macro is removed.

Original gate, privilege, prefix-width, external IRQ/NMI, #GP/#NP/#SS and double-
fault cases keep their architectural frame, PIC, access-byte and stack-memory
checks. Two NMI pending-bit cases and four whole-CS/SS-cache rollback cases are
retained in the independent CPU fixture, with the original cached state and
failure variants; the board observes public copies instead. No test-only public
API or production change is added. Temporary per-case diagnostics were removed.
The final x64 rebuild and both tests pass (2/2, 1.25 seconds). Earlier failures
identified the missing explicit handler round and invalid DS setup, not changed
instruction expectations. Software-INT, CLI/STI, hardware delivery and VM86
includers still require migration; this is not complete dependency-group or
full-suite acceptance.

- Complete negative-path proof and final independent creation validation for
  the bus contract; retain the successful transfer/publication ordering.
- Finish migrating the remaining mixed CPU test fixtures and consumers after
  the production opaque allocation cutover; keep one CPU owner and restore
  full-suite buildability without a private-layout compatibility pointer.
- Verify CPU-local timing/repeat ownership in the independent component;
  production has no generated catalog dependency. Board manifest tests retain
  their metadata/provenance inputs; board wait/page/DMA state stays outside CPU.
- Rehome all nine original CPU files and chip tests, reconnect every caller,
  enforce the new boundary, then run the complete packet verification and
  deliver target-separated P commits and eight current EXEs.

No CPU ledger acceptance, timing promotion, S closure, commit or push is claimed
by this work record. No owner INI, external asset master, Shared corpus or
MyNES file has changed in this initial implementation segment.

## Incremental Baseline Recovery, 2026-09-29

The [inventory](t539-cpu-incremental-inventory.md) preserves the pre-recovery
diff identity and assigns all 100 remaining direct private-test consumers,
plus their indirect includers, to subsequent S packages. No accepted S was
reopened or silently reclassified as completed CPU extraction.

The board once again stores the original single CPU/decoder/execution context
until S30. Initialization binds that same storage to the existing bus provider;
new copied observations, CPU-owned timing/repeat and entry preparation remain.
Machine creation/destruction no longer requires the premature allocation
cutover. There is no mirrored CPU or public private-pointer getter. Existing
CPU-owner allocation/lifetime tests still cover create/destroy independently.

The first full x64 rebuild confirmed two remaining type failures: software-INT
and hardware-delivery include interrupt-entry and expected its original t_cpu
result. Rather than add a second result API or weaken private-cache assertions,
defer the whole incomplete interrupt-entry migration to S27. Its source now
matches HEAD exactly, with the original CMake delivered-fault definition
restored; the migrated source is preserved in the recovery artifact. All other
already migrated test scenarios and CPU-local NMI/rollback checks remain.

The subsequent full x64 build passes. Fresh full `ctest -L unit -j 4
--output-on-failure` passes 370/370 in 224.45 seconds, including the five CPU
timing runners, both formerly failing includers, interrupt-entry, private CPU
context and migrated board tests. This supersedes the earlier unbuildable
worktree state, not the still-open final extraction. Cross-width and complete
receiver verification, artifacts and actual-diff acceptance remain pending.
No S18 commit or push is claimed.

The retained x86 tree subsequently rebuilt completely; its fresh full unit run
passes 370/370 in 37.82 seconds. Full default integrations then pass 20/20 on
each width, including DOS, video, media, debugger and Windows setup checkpoints.
Six source/test manifests verify without Shared edits. These elapsed results
are not controlled cross-width performance measurements. Specialized gates,
tools-off proof, vendor boot pairs, release artifacts and final review remain.

### Incremental Boundary-Gate Reconciliation

The first complete specialized aggregate failed seven gates. Direct inspection
distinguished stale ownership checks from two actual fixture duplications:

- T332 now accepts the CPU-local EFLAGS fixture without requiring a whole
  board, and recognizes the split board fixture for board-owned consumers.
  Its 47-owner inventory and rejection of direct bind/freeze remain.
- T344 found four new direct constructors. KBC and PIC had unnecessarily
  expanded the shared lifecycle sequence; both now reuse the board helper.
  The real/protected UD tests intentionally use the built-in execution provider
  and real guest table loads. Those two are explicitly inventoried, yielding
  103 direct constructors, with no wildcard allowance or lost test cases.
- The PIC authority gate follows board bus binding, pending/acknowledge and
  the INTA transaction; it additionally rejects concrete PIC dependencies in
  CPU files. The old duplicate/private PIC guards remain.
- The timing seam checks the two CPU-owned result-origin assignments rather
  than deleted board fields, and rejects a board-state dependency.
- Jcc preview checks the observation flag through the CPU bus and the board's
  early inspection return before transaction creation. Existing guest opcode,
  timing expectation and effectful-read rejection checks remain.
- Direct-compilation classification now combines target and source options.
  The two CPU timing files already retain all four strict flags; they were
  incorrectly classified deferred after moving out of the board target.
  The unchanged verifier inspects actual Ninja compiler commands, not labels.
  T345 ownership and its negative self-test pass with 33 residual entries.

The complete `verify-current-specialized-gates` x64 aggregate now exits zero.
Its direct-command audit passes 370 rows: 337 strict and 33 deferred. No gate
was removed. Only NXVM build checks and two NXVM test fixture tails changed in
this reconciliation; no production algorithm, Shared corpus or owner INI changed.
After the fixture edits, full x64 units pass 370/370 (28.47 seconds); the
incremental x86 build and full units pass 370/370 (24.97 seconds). The existing
standalone test/x86 tree with X86_BUILD_TOOLS=OFF rebuilds without work and
passes all 45 tests (4.24 seconds), including source/test manifests, corpus and
negative verifier checks. CPU has not yet moved into that Shared corpus; this
proves existing extracted chips remain independently usable, not final CPU
independence. Vendor boots, eight current artifacts and actual-change review
still precede S18 delivery. No commit or push is claimed.

### Fixed-Product Receiving Proof, 2026-09-29

Built and deployed vm-0-5-0539 and rebuilt the BYOB probe for each selected
profile, sequentially in the two retained trees. Each vendor/profile width ran
exactly once against assets/nxvm/<profile>/NXVM.ini, with
--no-retirement-observation and no F1 or turbo override. All six returned
exit 0 and installer-ready from the copied display snapshot. Limits remained
90 seconds; a timeout is not an accepted terminal.

| Profile | Width | Wall seconds | Executed | Guest elapsed ticks |
| --- | --- | ---: | ---: | ---: |
| XT | x64 | 21.16 | 12868797 | 127571436 |
| AT | x64 | 40.81 | 26310929 | 127245757 |
| Model40 | x64 | 69.28 | 26658886 | 167930358 |
| XT | x86 | 28.42 | 12860369 | 127478054 |
| AT | x86 | 47.26 | 26306689 | 127229264 |
| Model40 | x86 | 76.83 | 26658886 | 167930358 |

Instruction counts depend on the diagnostic display polling boundary, not a
timing-grade assertion. Default's two full 20-test integrations were already
verified on these unchanged production sources; no additional default boot
was substituted for that suite. Both retained trees are restored to default.

All eight deployed files contain 0.5.0539, have the expected PE machine
(8664 x64 / 014C x86), and objdump -h finds no compiler debug sections.
Release/architecture gates ran during deployment. Owner INIs, external
masters, Shared source/test trees and MyNES remain unchanged. Artifacts are
local pending delivery, not a pushed baseline.

| Artifact | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 2687839596576482B42C5E8D53DCD95D05051BB12E48BBBAAC0530C66E31E850 |
| nxvm_model40_0_5_0539_x86.exe | 033A023B6F2CABD2671EF39AF8E4A34AD63DEF0686BC6537475AFD312D508F60 |
| nxvm_default_0_5_0539_x64.exe | E8F986929132F4532721741C333F92E999DAE55E0A8A00AC622F67FB02E75968 |
| nxvm_default_0_5_0539_x86.exe | 68F334A75FD6CE1DCB00DC525623C09114F66A895B559705796FCF3E5020CBC4 |
| nxvm_xt_0_5_0539_x64.exe | 1DDE7751E987CAA20022A99C4A9FEC4F06C8F15D597BA7603B751B3A8EFB77EC |
| nxvm_xt_0_5_0539_x86.exe | 42849CFF98C617E22FC6DD3D856982079BAF5C9A3F5AA035C566689A3AAE5BB4 |
| nxvm_at_0_5_0539_x64.exe | 40FE3C866A48C5936D1DFAACCC42EC63B8EEF38A81DF116B43B3429CD598E387 |
| nxvm_at_0_5_0539_x86.exe | 1885D30FD35D544ACE0FFFC5E37B41325EDDD93F775A0B91EECAD9E2B48F9E3A |

Source review in this receiving step checked CPU creation/reset, prepared-entry
validation/publication, KBC reset injection, board NMI routing, firmware hook
removal and timing selector ownership. The complete pending source/test diff
still requires final actual-change review before S18 implementation delivery.
No S18 commit, acceptance or final CPU extraction is claimed.

## Timing Diff Review Continuation

Actual-diff review of cpu_timing.c and cpu_timing_model.c distinguishes owner
substitutions from algorithm changes. The selector order, LOCK addition, WAIT
addition and existing fallback classifications are retained. Timing results
and repeat state now belong to the CPU execution context. Three descriptor
observation reads use the bus with observe_only=true and reset_fetch=false;
the board provider returns through the same physical inspection routine before
opening a transaction. The timing formulas and constants were not changed.

The 196-line suffix beginning at core_machine_transaction_trace is byte-equal
after newline normalization between the baseline timing model and cpu_bus.c;
only its preceding forward declaration is omitted at the new location. This
proves the moved board wait/page/transaction bodies were retained, not replaced
with a new timing recipe. Mechanical owner/macro substitutions were filtered
for review, and the remaining include, bus-read and relocation differences
were inspected explicitly. Review of the remaining migrated tests is still
pending; this partial review does not authorize S18 closure.

## Test Ownership Review Continuation

Compared the original and migrated prefetch, EFLAGS, paging, protected-mode,
real/protected UD, final-exception, CPU diagnostic and retirement-observation
changes. Prefetch reservation contents/invalidation/reset remain in the CPU
fixture; the board scheduler test checks grants and suppression during DMA and
refresh holds. Paging's five control-rejection cases, seven INVLPG cases and
mutable-CR0 register/cache assertions remain in cpu_execution_context_smoke.
The 80186 LGDT rejection also moved there. Board paging retains page-table,
fault-frame, permission and cross-page side-effect checks with real descriptor
setup instead of private cache writes. EFLAGS retains the original opcode and
profile loops. Retirement tests distinguish entry/current copies and reject
running-state synchronous snapshot access.

Review found a real coverage loss: the public segment snapshot omits internal
cache validity/type bookkeeping, so replacing a whole-cache comparison with
that snapshot alone is not equivalent. Added CPU-owned full ES/SS/DS/FS/GS
comparisons for the original five protected UD forms, with both valid and
rejected delivery gates. No production API was expanded. The first test run
used the existing ring-three/32-bit NMI setup and failed; the migrated cases
now explicitly restore their original ring-zero/16-bit code setup. Temporary
diagnostic output was removed. The sequential full-suite command then passed
370/370 on x64 and 370/370 on x86 (final command exit 0; x86 20.77 seconds).
Documentation governance and diff checks also pass. This was a test-only
correction; executable inputs and deployed hashes are unchanged.

The experiment also observed that 32-bit BOUND register-form input returned
the CPU emulator-error path instead of the UD result obtained in 16-bit mode.
This is not a corrected or accepted form: S24 must reproduce it against the
baseline, determine the architectural expectation and review the other
memory-only operand forms together. No CPU behavior or timing grade was
changed to make this migration test pass.

## Public Boundary And Receiving Caller Review

Reviewed the complete CPU public-interface addition, execution-context header,
timing-header changes, CPU bus fixture and NxvmProduct.cmake diff. Public bus
callbacks carry values and borrowed opaque context, not RAM/port/PIC/transaction
objects. Instruction and fault observations are copied; snapshot entry/current
selection is explicit. Existing constants/declarations move to the public
header without changing their values. CPU arithmetic macros retain their old
bodies under CPU-local names. The single x86-cpu target owns the four CPU
translation units; board routing remains in core-machine. Generated timing
metadata is still built for the five ledger runners, not linked into production.
The prefetch scheduler spy compiles the same scheduler with one outgoing call
redirected; it is a test-only object, not a second production scheduler.

Reviewed all changed machine-layer tests: reset/register/watchpoint and memory
isolation checks now use public operations; refresh thresholds, HLT outcomes,
DMA values and video function observations retain their expectations. Terminal
UD setup uses guest LIDT rather than private IDTR mutation. Reviewed the timing
preview, D4, FPU, memory-inspection, REP CMPS, real-mode tick and PIC lifecycle
changes as well. Real-mode tick accounting explicitly subtracts the guest setup
phase; the measured instruction tick expectations are unchanged. FPU reset now
records and checks patch failure instead of ignoring the fixture result.

Integration review distinguishes stopped snapshots from synchronous write-time
CPU capture. Boot success predicates, wall limits and fault handling are not
relaxed. Model40's retained memory observer already records vector writes before
the removed duplicate observer would run. The removed small observer had also
been registered for other profiles; its removal drops that Model40-named
diagnostic there, not a boot acceptance condition. This distinction must not be
described as byte-for-byte preservation of all probe diagnostics.

This continuation is actual source review, not a new test run or S acceptance.
Final review of the remaining small device callers and delivery is still open.

## S18 Implementation Delivery Review

Completed the remaining small caller diffs, board-fixture extraction, XT NMI
delivery checks, debug forwarding/moved snapshot logic and task-document review.
The small timing/board edits preserve their original assertions while changing
their owning API or fixture include. The real INT/IVT path remains live. The
original interrupt-entry test remains at baseline, with its migration explicitly
assigned to S27; retaining it is not a new private-state access route.

Latest on-disk unit logs contain 370 passed and zero failed tests on each width
(x64 2026-09-29 10:06:33, x86 10:06:56). Earlier recorded default integrations,
standalone tests, specialized gates, manifests, six vendor boots and eight
artifact hashes cover the unchanged production inputs. The subsequent change
was CPU-owned test coverage plus evidence, not a production/artifact change.

Counted 105 source/test/build/tool paths under src/app-nxvm, test/app-nxvm,
cmake/nxvm and tools/nxvm: 6,139 added, 4,772 removed, net +1,367 lines. Method:
git diff --numstat against 25ec0f6c3, plus the complete lines of the three new
CPU bus/fixture files; documentation and binaries excluded. Mechanical macro
renames contribute to both columns. The positive remainder supplies the bus
boundary, copied observations and independent CPU regressions; it is not a
second executor. Embedded CPU storage and the mixed legacy fixture remain
deliberately until S30 and S29 respectively, as mapped in the inventory.

Requirement disposition: complete-unit recovery and receiving verification are
proven; every deferred consumer has S19-S32 ownership; the pre-recovery patch
preserves deferred edits; instruction tables and timing formulas were not
rewritten. Shared/MyNES/INI diff is empty. S24 owns the unresolved 32-bit BOUND
observation; it is not claimed fixed. CPU Shared relocation is not yet achieved.
Executor review finds this amended baseline brief ready for implementation P;
coordinator actual-commit acceptance remains required.
