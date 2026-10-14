# T556 S11 Default Text Raster

## Baseline and ownership

Admission baseline: `7bfefb2fc`. MyNES S10's pending diagnostic and documents
are preserved, not part of this repair. The initial scope was the default App's
guest firmware, tests/diagnostic and release pair. After native observation
exposed the TIME fault, the owner approved its Core root-cause repair below;
that extends qualification/publication to all four PC release pairs. Shared
six, other App source, MyNES artifacts, INIs and external inputs stay unchanged.

The ROM builder places both eight-row font halves in the first sixteen rows
of each 32-byte glyph slot. The BDA already declared height 16, but mode03
programmed CRTC09=07 and display-end 199. Thus the actual scanout was eight
rows per character. T554 `b25e2ca8d` added that eight-row cursor setup; T555
`b2ee99b1c` correctly stopped forcing sixteen rows in Machine conversion,
exposing the inconsistent firmware. Later mode-return programming retained
the 200-line register table. The synthetic probe alone did not measure an EXE
or establish anything about RDP DPI.

## Mode and source grade

The default uses the generic EGA personality; this is a project-defined machine,
not a claim of a physical EGA monitor supporting a standard 400-line mode.
Keep the existing horizontal and clock setup. Shift each old vertical endpoint
by 200 lines, retaining the old porches and pulse widths:

| Quantity | Before | After |
| --- | ---: | ---: |
| Active lines | 200 | 400 |
| Modeled total | 262 | 462 |
| Blank start/end | 224/240 | 424/440 |
| Retrace start/end-exclusive | 225/228 | 425/428 |
| Character height | 8 | 16 |
| Visible cursor rows | 7..7 | 14..15 |

CRTC06=CC, 07=1F, 09=0F, 10=A9, 11=2C, 12=8F, 15=A8, 16=18.
All vertical values fit EGA's nine-bit fields. The five-bit blank endpoint
resolves to 440 after start 424; the four-bit retrace endpoint resolves to 428
after start 425. Line compare remains 511. Underline remains outside the cell.
Mode03 also restores BDA columns, cursor shape, rows and font height.

Register interpretation is based on the IBM EGA Technical Reference, printed
64/65, retained in the Options and Adapters Technical Reference volume 2 scan
SHA-256 `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
Core's existing vertical terminal-count +2 and oscillator/raster model remain
L2. This change does not establish a hardware refresh-rate or physical-monitor
qualification and does not borrow a VGA mode table.

## DeskPro disposition

`app-mydeskpro386/machine_construction.c` selects Compaq Enhanced Color.
The Compaq Enhanced Color Graphics Board and Color Monitor Technical Reference,
December 1986, book 106451-001, printed 2-13 and 2-19, describes the high-resolution
80x25/350-scanline mode and an 8x14 font held in 32-row slots. Scan SHA-256:
`278C839C18962ABD57B661315A80C519E343005C9FA1708FD1BC7461A92ADB34`.
The selected Machine contract test explicitly programs and verifies that mode;
it is not, by itself, evidence of what an unmodified BIOS executed.

640x350 is a valid native pixel grid, not evidence of a missing sixteenth font
row. Physical monitor aspect and square host pixels are different questions.
No DeskPro defect has been established by this report; do not alter its ROM,
font height or host scaling to match the default machine.

## Initial firmware-only qualification (before the Core repair)

The existing actual-CPU firmware receiver covers boot,
graphics modes D/E/10 and return to mode03, both font halves and unused rows,
bottom-row text, BDA geometry, cursor shape, copied Machine font and dimensions,
and ordered active/retrace/exit observations. No synthetic input is substituted
for guest execution in that receiver.

The App diagnostic observes the deployed executable and unchanged adjacent INI.
Its geometry record distinguishes native client size from outer Window size.
Actual RDP phone-client DPI/scaling acceptance remains separate from these tests.

Both original Ninja/ccache graphs build. Final repository units pass 592/592
per width, and default integrations pass 22/22 per width, including DOS and
Windows31. Logs: `build/t556-s11-{x64,x86}-unit-final.log` and corresponding
`-integration.log`. The first concurrent dual-width unit runs timed out in
Lib test-types-boundary-selftest and Emulator verifier-negative; isolated
per-width runs at four jobs pass without source or timeout changes. These
initial timeouts remain recorded, not counted as passes.

Native observation is **not qualified**. In the available remote Windows
session (`SM_REMOTESESSION=1`), the baseline EXE's client is 304x95, outer
320x134. The repaired EXE's client is 304x190, outer 320x229: the requested
pixel aspect changes correctly. This is not a measurement of an unscaled
640x400 client, nor verification of the owner's phone-client DPI settings.
The diagnostic's outdated prompt/title assumptions were updated to find the
existing Lib Window class and current monitor prompt; capture remains only
an observation, never an automatic boot-success assertion.

The repaired EXE subsequently reports Machine error and closes its Window
after approximately 7--9 seconds in both architectures. The baseline survives
a separate 40-second observation. A read-only debugger breakpoint identifies
`vm_machine_fault_capture` from `vm_machine_runner_run`, with Core stop reason
FAULT, PC `0000D02C`, detail `54494D45` (TIME), elapsed ticks `00D6F046`.
The Lib component-failure breakpoint did not fire. A second breakpoint reaches
`core_machine_report_fault.part.0` from `core_machine_run`; the optimized
branch coalesces several timing-failure checks, so the exact failing timing
predicate is **not yet established**. Do not attribute it to a specific CPU
opcode, overflow or video deadline without further evidence.

Retained diagnostics: `t556-s11-native-cpu-gdb.log`,
`t556-s11-native-time-gdb.log`, `t556-s11-short.log`, and
`t556-s11-baseline-long.log` under build. This exposed Core execution failure
requires an explicit scope decision before a CPU timing repair. No Core or
Shared production changes have been made. The provisional default EXEs were
removed from the release positions by restoring the exact admission binaries;
the candidate builds remain in the original build trees. S11 is not delivered,
committed, pushed or closed; T556 remains open.

## Owner-approved Core investigation

The owner explicitly requested root-cause isolation followed by repair.
An optimized diagnostic link with DWARF for the existing Machine/timing objects
reproduced the fault without changing production source. At the common TIME
branch, the result was zero ticks from the 80386 fallback. The decoder held only
`2E`, `oplen=1`, `flagIgnore=true`, after asynchronous interrupt entry. The
entry's saved location was 0D7A:0051; the prior prefetched span ended at that
single segment-override byte. The CPU was executing the interrupt handler,
not a completed one-byte segment-prefix instruction.

The causal order was `ExecIns -> ExecInt/ExecInit -> Core timing_select`.
`ExecInit` during INTR/NMI/debug-trap delivery reuses the instruction record,
including old registers, operands and prefetched bytes. Selecting timing after
that operation can classify the wrong instruction, not merely lose a font row.
The prefix-only case selects zero ticks; the existing nonzero elapsed-time
publication contract correctly rejects it. No fallback tick is appropriate.

Repair: select timing once inside CPU refresh immediately after successful
instruction execution and before asynchronous entry. Core reads the existing
copied timing result. The selector is now private to CPU; the existing capture
API remains the Core boundary. No timing table, formula, precision grade,
Shared code, persisted state, retry or second execution path is added. A refresh
that only wakes an already halted CPU uses the existing no-retirement delivery
outcome rather than charging an unexecuted instruction.

This was latent in the admission CPU implementation. The firmware change adds
four BDA stores and changes the raster deadlines, shifting guest execution
relative to interrupts. The baseline's 40-second run did not encounter this
particular prefix boundary; that is not proof its late timing selection was
safe. Separately, the original 640x200 geometry was hidden by the pre-T555-S6
fixed-16 Window conversion. These are two distinct historical causes.

Regression coverage adds INTR, NMI and TF-trap cases across all five CPU
profiles, plus halted-NMI no-retirement and an actual Core-run 80386 boundary
test. Existing repeat-phase and transfer timing tests now read the recorded
result rather than selecting a second time. The timing seam gates enforce the
new order and prohibit Core from reselecting after interrupt delivery.

Initial repaired native x64 observation remains running for 40 seconds and
retains a 304x190 client, without the prior Machine error. Its desktop bitmap
is black and its legacy diagnostic pause acknowledgement fails, so this run
does not certify visible DOS content or interactive RDP behavior. Full final
qualification and artifact delivery are recorded below when completed.

## Final regression proof and review

The new actual-Core interrupt-retirement test was linked against admission
CPU/Core objects and exited 11 at the new regression, after its earlier cases
passed. The same test linked against current objects exits zero. This is an
old/new implementation comparison, not just a changed expected value. The
repaired x86 native executable also survives 40 seconds, with a 373x233 client
and 389x272 outer window in the remote session. Its captured bitmap visibly
contains the DOS A: prompt. The legacy diagnostic pause acknowledgement still
fails, so neither native observation is reported as a passing integration test.
Logs: `build/t556-s11-fixed-native.log` and
`build/t556-s11-fixed-x86-native.log`.

The existing FPU WAIT receiver expected interrupt entry to count as an
instruction. It now separately asserts zero retired instructions/ticks for
entry, the handler address and saved return address, then executes and counts
the handler's HLT. No FPU algorithm or timing table changed. The first final
x64 unit attempt exposed this assertion and a stale test manifest; both were
corrected before the clean final complete runs.

Implementer and coordinator reviews are sequential in this task. The actual
production diff preserves one timing selector and the existing copied-result
owner. It adds one stack-local boolean, no persistent fields, no public API,
no fallback tick, and no new renderer or execution path. The former externally
declared selector moves into the existing CPU-private header. The actual
timing formulas and precision grades are unchanged; this is not a claim of
new hardware-exact interrupt-entry timing. Existing shutdown/fault/WAIT paths
retain their no-retirement semantics. Source/test Core manifests cover the
changed files. Public six, MyNES production/artifacts, all INIs, external ROM
masters and media remain unchanged.

Production C/H/ASM changes total +43/-22 (net +21), existing tests/diagnostic
+168/-45 (net +123), and the two timing gates +16/-4 (net +12). These counts
exclude documentation, manifest hashes and binary artifacts. The extra tests
exercise the actual execution/capture boundary; no test framework is added.

## Final qualification and artifacts

Both original Release Ninja/ccache graphs were reused and restored to the
default profile after qualifying the other PC receivers. Common Core objects
are reused between profile builds; no substitute build directory is introduced.

| Final executed selection | x64 | x86 |
| --- | ---: | ---: |
| Complete repository units | 592/592 | 592/592 |
| Default DOS/Windows/profile integration | 22/22 | 22/22 |
| My5160 integration | 2/2 | 2/2 |
| My5170 integration | 4/4 | 4/4 |
| DeskPro integration | 4/4 | 4/4 |

The 32 integration executions per width include common routes invoked against
different profile bindings, not 32 distinct test sources. MyNES's 45 units are
included in 592; its integration suite is not rerun or claimed under this PC
repair. DeskPro unattended boot passes within 134.33 seconds (x64 suite) and
171.80 seconds (x86 suite), without changing its BIOS or injecting F1.

Background qualification comprises the builds, complete units and external
DOS/Windows/profile integrations. The three registered native Lib desktop
units also pass in each width. The separate actual-EXE remote observations
above are not passing desktop integration results: pause acknowledgement and
phone-client DPI/input remain unverified. No claim is made that every possible
guest or interactive scenario has been tested.

Logs: `build/t556-s11-time-x64-unit-final.log`,
`build/t556-s11-time-x86-unit.log`,
`build/t556-s11-time-{x64,x86}-integration.log`, and
`build/t556-s11-time-{x64,x86}-{xt,at,deskpro}-integration.log`, with the
corresponding build/configure/restore logs. Core manifests, timing gates,
documentation governance and whitespace review pass. Both specialized gate
targets retain the three historical deferred strict-compilation rows; they
are not represented as an exception-free full-tree build.

Eight rebuilt 0.5.0556 PC executables are deployed, with PE machine values
8664 (x64) and 014C (x86), version/deployment checks from the release targets.
MyNES artifacts remain byte-identical to admission.

| Artifact under assets | SHA-256 |
| --- | --- |
| `my5160/nxvm_xt_0_5_0556_x64.exe` | `93c3045746792f19a08c47356434768d105072977af0dbe37f69b62542d8bb3e` |
| `my5160/nxvm_xt_0_5_0556_x86.exe` | `fe1d37bb6d0c104e3bc9655f4f5cec8234a33db0449520e726a537fe055dd7af` |
| `my5170/nxvm_at_0_5_0556_x64.exe` | `794e78724c9addd287dd5effe5c11b921d7946767ba2cb6a39947c92364615a6` |
| `my5170/nxvm_at_0_5_0556_x86.exe` | `537efc32fa9a8bdfa0e8e72f488ab048c54184fc873e85e4a5d117a82feeaba0` |
| `mydeskpro386/nxvm_model40_0_5_0556_x64.exe` | `e25b2c88d54baa3ac00df0ba6a4526605adbbd9c9e3df9dbf117cee0aa640b9f` |
| `mydeskpro386/nxvm_model40_0_5_0556_x86.exe` | `58c8e8085cbafe1b6603a4ab4ad0896d3ab112ca71c742cfbcb0a76f4443c519` |
| `nxvm/nxvm_default_0_5_0556_x64.exe` | `49fe5e49946c0afb18390099e43c3bc050d9d1f30d3d2151721731472805e7e4` |
| `nxvm/nxvm_default_0_5_0556_x86.exe` | `e05652cd34525396ce62d05e8853bad37c1ca3810aab0b6e5739692dea0c4aa6` |

The NXVM-target P containing this report identifies the source/artifact
revision. S11 is delivered for owner acceptance, not closed; T556 stays open.
Unrelated MyNES S10 working changes are deliberately left uncommitted by this P.
