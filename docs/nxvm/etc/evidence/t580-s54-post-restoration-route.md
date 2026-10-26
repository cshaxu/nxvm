# T580 S54 - Post-restoration Setup and boot route

## Admission and complete route inventory

Baseline is clean pushed `63f1f6e5a`, after accepted S53. This S consumes the
original POST/Setup/boot ledger batch; no first-PC result substitutes for its
product predicate. The first phase changes only the existing external-BIOS
harness and records. Production, Shared packages and protected inputs remain
unchanged until a complete justified receiver is selected and admitted.

The admitted TFT BIOS remains SHA-256
`232101C88466F311BCC32FBC215A4D7569F695CE19F9C07CA67CE2AEE5232312`.
Read-only reference pins are PC110-EMU `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`
and PC110-QEMU `bae390ebf21665ec97860d28e26ec46f1fe8f4e9`. No implementation is
copied. Existing App firmware/configuration, EC, lower-Flash and Chip owners
remain the production receivers.

| Finite member | Source and current disposition | Required actual evidence |
| --- | --- | --- |
| Return from retained restoration | S53 proves RSM to F000:DD11; original bytes there finish the I/O helper and return to its guest caller. | Continue normal execution with copied context, no forced return. |
| Ordinary POST completion / reset continuations | Original 4Bxx branches distinguish guest BDA signatures, shutdown codes and processor reset paths. Core/Board already own the actual reset operation. | Record subsequent reset/exception/terminal context; never seed a warm-boot signature from the host. |
| Setup selection / loader | Original 3273h bytes compare F1's scan value and branch, but the cited 3391h raw Flash slot is also inside the resume routine. Runtime view/bank/loader context is not yet reconciled. | Follow actual firmware/guest mapping before calling any address a working Setup entry. |
| Lower Flash / Setup payload | App owns port-98 selection and shadow independently; existing units prove the receiver but not its BIOS loader consumer. | Actual guest bank/copy/loader execution; no host decompressor or payload load. |
| IPL / bootstrap | Original bytes at F000:52BA are OUT 8Bh,AL, STI, INT 19h, HLT. Current Board owns the zero-reset scratch/IPL latch. | Watch actual INT 19h execution and copied instruction bytes; entry is not a boot-sector/guest claim. |
| Terminal wait/fault | Existing Core result and copied CPU/fault observations own the distinction. | Classify stable hardware suspend, interrupt wait, BIOS error halt, architectural fault and budget exhaustion separately. |

PC110-QEMU's `pc110post.c` contains host PC interceptions, warm-signature
writes, direct Setup-image loading/entry and an optional palette patch. Those
are not receiver specifications or permitted implementation paths here.
Its comments provide research leads only; actual admitted bytes and runtime
mapping must independently validate each claimed address. Linear-sweep NASM
output also emitted invalid-opcode diagnostics outside selected windows, so
bounded objdump/raw-byte comparisons, not that unrelated sweep, anchor the
inventory. Neither disassembler can prove which runtime shadow view is active.

## Initial observation design

Extend the one existing harness with a temporary `--resume-boot` mode. It
reuses the complete S53 suspend/button/retention/RSM proof, then continues
ordinary Core execution and watches the actual INT 19h instruction at F52BD.
The watch must report that address and bytes CD 19; a timeout/budget is never
success. All earlier normal modes retain their exit predicates. The same
public debug retirement-tail guard is preserved. No per-retirement observer,
new executor, ROM patch, memory write, fake key or boot service is introduced.
The existing explicit F12 pair remains the named physical wake operation,
not automatic wake merely because the guest stopped.

Containment: 100M total instructions, 300 native seconds, task output at most
16 MiB under build/t580-s54-boot. Begin with one x64 execution after compiling
the reviewed observer; the x86 comparison follows only when that result
selects a meaningful, unchanged predicate. Failure means classification of
the whole route batch, not an arbitrary new production patch or product claim.

Pre-result observer review finds that later ordinary POST resets would clear
the boot watchpoint and incorrectly rearm the earlier restoration address.
The first owned x64 observer (PID 5880, exact executable/--resume-boot command
verified) is stopped before a result is claimed. The correction rearms the
current semantic milestone after every CPU reset. This changes no guest
instruction, register, hardware value or production reset path; that aborted
attempt is not evidence of a firmware failure or a passing route.

The corrected x64 attempt completes the original no-event/retained-image/RSM
milestones, but no INT19 marker appears before the 300-second native deadline.
The owned process is terminated by the existing bounded tool, with no final
post-RSM CPU snapshot. Log SHA-256:
`C68691CC8474EA76227787141757307AB3F3578C5230060BE8ED780D9603BBBF`.
This proves neither a BIOS deadlock nor a missing storage receiver. The 100M
whole-run instruction maximum did not ensure a terminal context before the
wall-clock deadline. The observer therefore bounds the new phase to eight
million additional instructions after RSM and prints its copied final CPU
context plus sixteen actual runtime instruction bytes. This remains within
the initial maximum and changes no execution, hardware, success predicate or
source grade. One corrective x64 observation is justified by that diagnostic
defect; an x86 repeat of the unclassified timeout is not useful or admitted.

The phase-bounded x64 run terminates normally with the expected diagnostic
failure (no boot), after 56,828,542 total instructions. Retained wake/RSM
still passes. The delivered exception count grows from four to 398,666;
current context is F000:EB1F, CR0=10h, FLAGS=56h, SS=0/SP=03F6h. Runtime
bytes match the original PIC/unexpected-interrupt handler there. There is no
terminal Core fault. This is evidence of repeated delivered exceptions after
restoration, not ordinary boot delay or a demonstrated disk defect.
Log SHA-256: `E44C5249171EF878926EE652267D760360CFA9AB10FAD6A328612F027F1EB352`.

The next meaningful observer uses the existing copied diagnostic after each
bounded Core quantum, only after RSM, and stops on the first new delivered-
exception batch. It records the existing fault snapshot's mask/code/point,
not just a later BIOS handler PC. No public trap API exists or is added;
there is no per-instruction observer, exception suppression or guest-state
write. That changed predicate follows the newly demonstrated exception class,
not another repetition of the boot timeout. Its cause and all peer route
members must be reconciled before any production repair is selected.
The first diagnostic build rejects an incorrectly nested `point.eflags`;
the existing snapshot owns `eflags` directly. The caller is corrected, with
no Shared declaration change and no native execution from that failed build.

## First post-restoration exception and source reconciliation

The corrected existing observer terminates at the first new delivered
exception, after 48,853,394 total instructions, four resets and five delivered
exceptions (four at RSM). The fault snapshot identifies #NM (mask 80h), code
zero, F000:6F25, bytes DB E3 B9 64: the FNINIT at the beginning of the BIOS
coprocessor probe. CR0 is 10h, so EM and TS are clear. The current handler
is F000:EB1F; no terminal Core fault is reported. Retained wake/RSM still
passes, but no INT19 handoff is observed. This is a diagnostic failure,
not a passing boot result. Log SHA-256:
`C6D6FAA124612A4A42103BB43B146638AC258CDA8CF6A04F3B1A7E3A101270E3`.

Bounded disassembly of the unchanged original F000:6F17--6F92 routine
shows FNINIT, a 5A5Ah memory sentinel, FNSTSW, and the no-FPU branch at
6F90. Thus the missing boot handoff cannot yet be attributed to storage.
The CPU's FPU_ESCAPE currently forces #NM for every 80486SX ESC, independent
of CR0; the existing CPU full-state test deliberately asserts that behavior
across modes, prefixes, memory/register forms and all EM/TS/MP/NE settings.
Changing only FNINIT or removing that test would not repair the whole class.

The archived Intel Embedded Intel486 Processor Family Developer's Manual,
order 273021-001 (1997), SHA-256
`BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3`,
section 4 CR0 discussion, recommends EM=1 for SX and describes an EM=0
hang; its NE discussion also describes interrupt 7 for floating-point
instructions. Those statements do not establish successful EM-clear BIOS
probing or justify claiming a hardware-exact correction.

Michal Necasek's original experimental report,
[How Does FPU Detection Work?](https://www.os2museum.com/wp/learn-something-old-every-day-part-xviii-how-does-fpu-detection-work/)
(2026-03-28), reports actual Intel SX683 and AMD 486SX observations: no-FPU
control stores can write memory and do not behave as unconditional #NM.
This is primary experimental evidence, not an Intel specification or a
PC110 ULP-SX measurement. Under the owner's source convention it supports
L2 research, not an L3 claim. It also warns against replacing all no-FPU
instructions with no-ops: memory validation, control stores and arithmetic
stores differ by generation/vendor. No report text or external source is
copied into production.

Before a Shared repair is admitted, reconcile register/control operations,
memory load/store widths and validation, EM/TS precedence, WAIT and legacy
CPU consumers as one finite absent-FPU batch. Preserve the actual CPU profile
and no-FPU binding; do not attach a fake numeric device, skip this BIOS PC,
alter the ROM or suppress delivered exceptions in the observer. Production
and Shared source/test remain unchanged in this S54 observation phase.

## Admitted complete absent-FPU correction

The observation phase is complete. The packet now admits Shared X86 Core
and receiving PC/MyRaidenII artifacts, with Lib/Emulator and MyNES excluded.
This revises the source applicability of T560 C14, not its historical record.
The original 1992 PRM 19.2.3.2 blanket SX statement is not sufficient to
describe actual EM-clear firmware detection. The independent hardware report
above and 86Box source pin `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`
(`src/cpu/x87_ops.h`, no-FPU execution) independently support the EM/TS
predicate without an unconditional SX exception. No external implementation
is copied or transliterated. The hardware report, rather than 86Box's simpler
no-op treatment, additionally anchors the Intel-like memory-store model.

Operand-format cross-check: the archived 1992 Intel486 Family Programmer's
Reference, SHA-256
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`,
FSTENV/FNSTENV and FSAVE/FNSAVE instruction entries explicitly specify
14/28 and 94/108 bytes. These are standard format facts; the manual's timings
for an actual FPU are not used as absent-SX execution times.

One existing FPU_ESCAPE path remains authoritative. The finite batch is:

| Member | Retained or corrected contract |
| --- | --- |
| EM or TS set | Existing architectural #NM before operand access, across all eight ESC opcode classes and modes; LOCK/fetch priority unchanged. |
| EM/TS clear, register form | Consume the no-device command without numeric state or extension callback; FNSTSW AX reports FFFFh. |
| EM/TS clear, memory form | Decode the standard D8--DF operand width/direction once in an owner-local table. Loads validate the operand without fabricating a numeric device; stores publish ones through the existing CPU memory path. |
| Environment/save formats | Standard 14/28-byte environment and 94/108-byte save widths follow the existing operand-size attribute. Intel-like absent fields are all ones; no AMD-style pointer tracking is claimed. |
| Reserved memory encodings | Preserve decoded consumption without inventing memory or numeric effects; these encodings are not advertised as defined x87 instructions. |
| WAIT, earlier CPUs and actual 8087/287/387 | Existing behavior and owners retained; no generic replacement of their distinct mechanisms. |
| Fault/publication | Validate complete segment/page range and applicable alignment before a store; issue bounded existing word transfers, never overflow the CPU's scalar access log with a large environment copy. Existing transaction failure semantics remain authoritative. |

This is explicitly L2 compatibility behavior for absent hardware, including
Intel-like all-ones values, not a claim of ULP-SX silicon-exact undocumented
bus values or full x87 arithmetic/timing. It covers the complete admitted
class rather than only FNINIT or this BIOS PC. No public field, function,
enum, persistent state, second decoder/executor or board flag is added.
Expected source +50--90/-2--8 and existing tests +120--220/-10--35. Existing
mode/control/prefix matrices are refined, not replaced by a boot-only test.

The first x64 CPU tests pass after correcting their reused exception-frame
helper: #GP includes a zero error word whereas #NM does not. This is an
assertion repair, not a production exception-entry change. Extending the
existing alignment fixture adds five operand forms and two cross-page
environment scenarios; #AC and no-partial-write #PF checks pass.

The actual Core test then reveals a related timing omission, independently
confirmed by the unchanged BIOS: FNINIT retires to F000:6F27 without #NM,
but Core rejects the unallocated time with detail TIME. Exception count
remains four. Real-BIOS log SHA-256:
`64EBCB7E4F4339485606706DA00AE154E49A24B1D295A8736617A52D78404342`.
The revised batch includes `cpu_timing_model.c` and its existing nominal/L2
test: successful EM/TS-clear absent-device ESC uses a one-clock noncomputing
decode envelope, explicitly L2, not a recovered SX hardware formula. This
matches the reference's non-numeric consumption behavior and reuses the
canonical one-clock no-op envelope. Existing fetch, prefix, memory and bus
wait owners still account for actual transfers; no zero-time exception,
unqualified compatibility fallback or numeric execution is substituted.
All eight ESC classes and operand/address attributes enter the existing L2
matrix, and the existing Core test adds actual control/save memory forms.

The similar-issue sweep finds one architectural dispatch owner for all ESC
classes, one WAIT owner and one 486 L2 cost selector. Earlier 286/386 no-FPU
paths remain live and deliberately distinct: experimental pointer/control
store semantics differ by generation, so copying the SX all-ones model into
them would be unjustified. Their newly identified absence-boundary debt is
assigned explicitly to the existing queued x87 proposal's initial audit,
not hidden as a verified behavior or a new duplicate queue item. Actual
8087 arithmetic/handoffs and NEC FPO1/FPO2 are separate device/protocol
semantics and remain unchanged. Tests for rejected EM/TS, WAIT, LOCK and
fault entry remain live at their existing owners.

Both widths now pass the four affected CPU/Core tests, including the full
encoding census and explicit L2 cost checks. The x64 initial test failure
was the #GP error-word assertion described above. Adding a failure diagnostic
then exposed uninitialized output warnings in the test; its local copied
outputs are initialized, with no warning suppression. The x86 build and
four tests pass in 5.09 seconds. Complete repository units, actual BIOS
completion and receiving artifact qualification are still outstanding.

The x64 real BIOS subsequently reaches the actual F000:52BD INT19 and
lands at its BIOS service F000:E6F2. It has 56,679,591 instructions, four
resets and the same four exceptions, with all retained-power predicates
passing. Log SHA-256:
`A9245195CE3FDF6F04EFCA6400D7F753A973A44EA259BB27EE4166A07F9A940F`.
This establishes ordinary POST/bootstrap handoff, not a boot-sector or DOS
claim. The registered boot-handoff replay replaces the shorter power-only
registration, retaining every original power predicate and the same 300-second
deadline; the independent FDC/timer/ASMI regression remains registered.

The first x86 real execution reaches retained RSM but exceeds that unchanged
300-second deadline, without an INT19 result or final diagnostic context.
It is not passing, nor evidence of a new CPU fault. Its log SHA-256 is
`17BFD6696B775554B362C31A8B436E9D78EE66AC73278B8F7CBC31909FD8C4F8`.
Observer review removes the temporary per-quantum full diagnostic copy used
to locate #NM: the final unchanged exception-count assertion already rejects
new exceptions. The power/boot observer now asks the same bounded Core runner
for 65,536 rather than 4,096 instructions per call. Core still advances every
instruction/device deadline and stops at the same watchpoints, suspend and
reset events. This only reduces host observer/entry overhead; no guest time,
instruction, firmware, native budget or success predicate is skipped. The
timer/FDC observation quantum stays unchanged. One corrected x86 run is
admitted by that concrete observer change, not repetition of the timeout.

The corrected registered x86 replay still exceeds 300 seconds, after real
RSM and without an INT19 marker. CTest records a timeout, not success;
log SHA-256 is
`351AD62368A1B15F735B6F75B627490CF0EDAB2DD7AF430313E72FB7FF6F7F8E`.
The observer cleanup is retained for simplicity, not claimed as a sufficient
performance fix. No further unchanged replay is admitted. The next bounded
diagnostic must record host elapsed time and copied post-RSM progress at
sparse semantic/count boundaries before selecting a cause. These host values
must never drive guest time, execution or the success result. S54 stays open;
the CPU/model correction is not delivered as a partially qualified P.

While that separate qualification remains open, the frozen CPU repair runs
the required full cached x64/x86 build and repository unit regressions. The
Types layout selftest is scheduled alone. The full default build also invokes
unaffected MyNES's deployment recipe; any generated timestamp-only MyNES
artifacts are excluded and restored from the exact clean admission baseline
after owned builders exit. Its source/configuration/snapshot remains untouched.
This is generated-output scope cleanup, not removal of another session's work.

Both full cached builds complete successfully. The isolated Types selftests
pass in 107.56 s x64 and 99.84 s x86. The first controller ends after its
x64 Types command; the exact remaining x86 Types and two 711-case unit runs
continue in controller session 35732, without re-running the successful
x64 Types case. At this record the x64 remainder is still live, so no full
unit-corpus result is claimed. MyNES's two generated deployment copies are
restored exactly from HEAD after the builders exit; its paths are clean.

The subsequent sparse observer draft uses existing Lib Base's monotonic
milliseconds and copied Core observations at startup, physical wake, RSM
and each further million instructions. A single test-local formatting helper
owns that bounded diagnostic output; no production helper, clock owner or
public interface is added. Clock/capture statuses are printed, not converted
into guest state or success. This diagnostic-only draft follows the completed
full builds and is not yet rebuilt/executed while the full unit controller
is live. It must be compiled before the next evidence-led BIOS replay.

The x64 remainder completes in 341.31 s: 710/711 pass; the one failure is
the existing executor-run smoke's separate SX FPU matrix, which still
expects unconditional #NM for controls=0/operation=0. This was missed by
the earlier FPU-filename-only search. Its matrix is retained and corrected
to EM/TS for ESC versus TS-and-MP for WAIT; it now asserts the actual
two-byte ESC retirement length and a real following NOP, while keeping all
fault-frame/GPR checks. No production code changes follow this finding.
The corrected x64 executor case and changed test manifest both pass, 2/2
in 1.43 s. Combined with the 710 passing cases and isolated Types result,
all 712 distinct x64 repository units have passing evidence, with the initial
failure preserved rather than relabelled as an all-green first attempt.

The expanded search is `rg -l 'VCPUINS_EXCEPT_NM|sx-fpu|Core SX FPU|FPU_UNSUPPORTED'
test src -g '*.c' -g '*.cmake'`. It finds the sole production dispatch,
the two existing CPU/Core FPU tests, this executor receiver and the Board
386 FPU escape smoke. The latter tests actual EM/TS delivery on 80386,
not unconditional SX behavior, and remains unchanged. There is no second
production #NM policy. The full x86 remainder, now built with the corrected
executor test, is live in session 50342; after it completes successfully,
that same controller runs the sparse-timing x86 BIOS diagnostic alone.

That x86 remainder now passes 711/711 in 131.39 s, including the corrected
executor matrix (0.35 s). With its previously isolated Types pass, every one
of the 712 distinct x86 units has passing evidence. The two refined builds
also compile the new sparse observer without warnings. Session 50342 has
continued to the one isolated actual-BIOS x86 diagnostic; no changed outcome
is inferred before that native process terminates.

## Whole-route budget correction, not another CPU fix

The sparse x86 observation ends at its original 300 s deadline, log SHA-256
`619A14EB0130F8A3B37FBC7E9324617F6A8C9FD5F6E8DED43954A3BDE499867D`.
Host milliseconds are measurement only:

| Semantic point | Host elapsed | Instructions | Exception count |
| --- | ---: | ---: | ---: |
| Reset entry | 0 s | 0 | 0 |
| Genuine suspend / physical wake | 171.182 s | 27,893,749 | 4 |
| Actual RSM return F000:DD11 | 270.414 s | 48,828,526 | 4 |
| Later EA88 finite delay | 299.697 s | 55,080,898 | 4 |

Each later 1,048,576-instruction sample advances in 4.75--5.06 s through the
original EA86/EA88 port-61 delay, with no new exception. The already proven
x64 INT19 point is 56,679,591 instructions: about 1.60M instructions beyond
the last x86 sample, approximately another eight measured host seconds.
This classifies the observed deadline as an undersized whole-route budget,
not a halt or a remaining demonstrated FPU fault.

The coordinator corrects the new boot-handoff registration to a finite
360-second envelope (original power-work envelope plus the newly added POST
delay and modest measured margin). Earlier 300-second results remain failed;
the shorter original power predicate and stronger INT19/retained-state/
exception assertions are not removed or weakened. This is not a claim of
real-time emulation or a new guest clock/timing model. Under the owner's
automatic in-scope S planning authorization, the packet explicitly records
this changed workload budget before execution. No unrelated Core performance
patch, skipped busy-loop instruction or firmware-specific fast path is admitted.
Both widths must pass the final registered whole route; a projected eight
seconds is not proof of that result.

The expanded CPU-bound boot registration also has RUN_SERIAL, so an eventual
parallel CTest integration invocation cannot silently invalidate its isolated
workload budget by co-scheduling other tests. This does not add a scheduler,
thread, timing relaxation or guest behavior. The final actual-BIOS controller
already executes its two selected regressions and both widths sequentially;
the x86 FDC/timer/ASMI regression passes in 62.42 s before boot replay starts.

Actual-diff quality review covers both production functions and every changed
test owner. The memory-direction table covers the existing eight ESC opcode
classes, not firmware addresses; full-range protection/alignment/page checks
precede stores. Numeric device state and the earlier CPU/FPU branches remain
unchanged. There are no new public declarations, mutable production fields,
helpers, threads, clocks or dispatchers. Three bounded test helpers cover
different responsibilities: the full operand expectation matrix, real Core
store publication and sparse external-BIOS progress formatting. Existing
fixtures and registration are reused, with no new test executable/framework.
The ordinary runtime and absent-hardware behavior remain explicitly L2;
neither a passing test nor a BIOS checkpoint upgrades their source grade.

Final x86 registered results: FDC/timer/ASMI passes in 62.42 s and full
retained-power/POST/INT19 passes in 302.96 s, 2/2. Its actual marker is
INT19=1, target F000:E6F2, 56,679,591 total instructions, four processor
resets, one genuine suspend/wake and exactly four delivered exceptions.
The same 512 retained CPU bytes match and RSM returns F000:DD11 with flags
46h. No terminal Core fault is present. The final summary log SHA-256 is
`3DCCC202F55C887B384197D086505968FD86F82C31B8CD4A3B0C34A3A9CDA3CD`.
This independently confirms that the 300-second cutoff was short of a valid
finite boot path, not proof that larger wall-clock time repairs guest state.
The x64 final selected regressions are still live; its FDC prefix has passed
in 45.76 s and its final full boot result must be received before delivery.

The inherited instruction-source warning exception is unchanged. No new
warning suppression is added; neither a clean log nor these tests is claimed
as a complete no-waiver strict-warning qualification of all CPU source.

The final x64 registered regressions both pass: FDC/timer/ASMI 45.76 s,
full retained wake/POST/INT19 224.16 s. It has the identical 56,679,591
instruction / four-reset / four-exception / retained-512-byte / actual-RSM
and INT19 predicates as x86. Summary log SHA-256:
`AB2D50FBEF2C5072D766601E51CDA0560F2E1ABE9C82631DA00F67B1CA28F151`.
Before deployment identity checks overwrite CTest's standard temporary logs,
the actual output hashes are retained: x86
`FD3E748FE5A58D322AC83C340A8E1A9B24E97392C1DDFAF8576A009ACA850E08`,
x64 `A7EC93A54A5A65BDFA1CA35E77BF3E99ABCEEB85A2BA7186EA0FF991F8ABEA7C`.
The first full unit remainder hashes retain the honest initial failure and
green x86 run respectively: x64
`A2501A3DAC35FF39D88B5D6F64460455E0D55DA1A2BB2D4F399B0D1FD4CF72E2`,
x86 `EC109B60FADCA957BADF9E25451895A10B55B0418E507023EC1F6D83526C9B04`.

## Complete route disposition after the repair

| S54 finite route | Disposition and next owner |
| --- | --- |
| Retained restoration return | Actual driver F12, retained CPU image and real RSM pass both widths; existing S53 power receiver remains the one owner. |
| Ordinary post-restoration POST/reset | Both widths execute the unchanged firmware to the same real INT19 service, without a new exception. The one CPU absent-FPU class and its timing allocation are repaired, not a BIOS-PC exception. |
| Lower Flash / Setup payload | Static bank/shadow/loader context remains reconciled at the existing App firmware owner. Setup entry is not exercised by the selected ordinary-boot replay; its real F1/loader/UI gate remains unaccepted in the original ledger. |
| IPL / bootstrap | Actual CD19 at F000:52BD retires and enters F000:E6F2 on both widths. This accepts a firmware handoff, not an inserted-medium or guest-OS result. |
| Terminal class | The former #NM loop and TIME rejection are corrected. The short x86 deadline is classified by sparse actual progress and the successful longer whole-workload replay, without a performance patch. |
| Remaining original product gates | App-owned actual storage/selected guest, Setup, display/panel, live input, original/derived DSTN identity, desktop/lifecycle and four-product publication remain open at their named original owners. No successful prefix or re-labelled budget closes these gates. |

## Delivery and actual-change review

Both original Release Ninja graphs are restored to the default PC profile.
The eight receiving PC EXEs retain 0.5.0580; all four real deployed INI
identities pass on each width. The Core-linked MyRaidenII pair retains
0.0.0007, with its existing release stripping and all 13 owner unit cases
passing on each width in the repository suite. The four rebuilt PC110
previews remain only in the reusable graphs, not published artifacts.
The [artifact inventory](t580-s54-artifacts.json) records exact PE widths,
versions, byte counts and SHA-256 for all ten receiving files. This count
does not include the unchanged MyNES pair. All PC INI Git blobs match HEAD;
no protected input, guest medium, snapshot or App production source changes.

A final build of the four receiving targets on each width reports
`ninja: no work to do.` The explicitly selected repository ccache reports
107,251 hits / 184,267 cacheable calls, 58.20%, and 0.8/5.0 GiB storage.
These are cumulative repository statistics, not an isolated S54 hit rate.
No new build tree or cache reset was used. An unrelated `wow32` build was
observed outside this task and left untouched.

Actual `git diff --numstat` counts against 63f1f6e5a, excluding documentation,
manifests and artifacts: production two C files +61/-2 (net +59); five
Shared test C files +203/-25 (net +178); existing NXVM BIOS harness +78/-5
(net +73). Total C/H +342/-32 (net +310), plus App test CMake +6/-5
(net +1). Production adds no helper, state, interface, callback, thread or
clock. The one local immutable operand table and preflight/store path cover
the absent-device class at its CPU owner. Three bounded test helpers reuse
existing fixtures; no new executable or framework is introduced. Earlier
286/386 no-device store/pointer differences remain explicitly assigned to
the existing x87 proposal, not silently qualified by this SX repair.

Coordinator review reads every changed production/test file, manifest,
README, App registration and evidence, rather than accepting only aggregate
results. It preserves initial failed attempts and existing warning waivers;
no claim of all-platform or no-waiver qualification is made. Shared P1 `48fe7a544`,
NXVM receiving P and MyRaidenII receiving P are separately scoped. The
whole T remains open for actual Setup, selected storage/guest, display/input,
lifecycle and publication. Desktop, RDP and gameplay validation are not run.

After all S54 native controllers and builders exited, the official cleaner's
preview identifies only owned `build/t580-s54-boot`: 41 generated log files,
399,400 bytes. Required outcomes and hashes above are archived before its
Apply operation, which removes exactly that directory and confirms absence.
Both changed Core manifests and both affected documentation gates pass.
The two persistent cache roots are retained; no unrelated process or scratch
owner is removed.
