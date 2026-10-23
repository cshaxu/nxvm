# M5 T564 S12 — X86 NXVM peripheral and timing-boundary audit

## Scope and method

This record audits the four selected PC App compositions, their X86 NXVM Board
plans, and the device/deadline path they already use.  It is not a claim that
every analogue property of historical hardware has been qualified.

The reviewed path is:

```text
App profile descriptor/configuration
  -> x86_nxvm_board_plan (complete timing declaration table)
  -> Board copied/frozen declaration and device clocks
  -> Core Machine's sole elapsed-time/deadline owner
```

The review included `src/app-my5160`, `src/app-my5170`,
`src/app-mydeskpro386`, `src/app-nxvm`, `src/x86-nxvm/chips`, and
`src/x86-nxvm/board-*`, with their direct tests.  It did not import an external
model or change a source, firmware, media, profile, or public interface.

## Ownership and selected configuration

`x86_nxvm_board_timing_capability` has a complete 30-entry declaration
universe.  The Board rejects a missing/duplicate/invalid declaration, maps each
capability to one fixed seam, copies the validated table only while Core
configuration is open, and rejects reinstallation after publication or reset.
The caller's source array is not retained.  This is the right owner for
composition evidence; it does not make Board a second guest clock.

`x86_core_machine` remains the only elapsed-time and deadline advance owner.
Board clock domains only translate a selected profile's ratio into that Core
axis.  Host display, host input, debug, platform mailbox/resource/wait, Session
commands, and Product Debug are mechanically required to be
`NON_GUEST_TIME`.

Selected profiles presently provide the following bounded device sets:

| App | Selected board family | Timed device facts | Evidence disposition |
| --- | --- | --- | --- |
| My5160 | XT: 8088, single PIC/DMA, 8253, XT PPI keyboard, FDC and optional Xebec HDC/CGA | PIT ratio is explicit; DMA/PIC remain fallback service behavior. | Macro-proportional/L2; no physical cycle claim. |
| My5170 | AT: 80286, cascaded PIC/DMA, 8254, RTC, 8042, FDC, WD1003 and display | PIT, DMA and RTC have selected ratios; PIC visibility remains L2. | Source-qualified controller clocks where declared; remaining controller behavior L2. |
| MyDeskPro386 | Compaq AT derivative: 80386, cascaded PIC/DMA, RTC, 8042, FDC/HDC, auxiliary PIT and CECG video | Config records the 16 MHz macro axis and clock ratios; comments explicitly retain non-physical/L2 status where the source does not establish a cycle-accurate bus model. | Mixed source-ratio/L2, not a physical board-timing claim. |
| NXVM | Generic 80386 AT/EGA profile | No selected source controller ratios; all controller timing rules remain L2 fallback. | Deliberately unqualified service axis. |

The device packages keep their own digital behavior: PIT, RTC, FDC, HDC,
video/CECG, PIC, DMA, KBC, keyboard, PPI, mouse and controller wiring are not
reimplemented in an App.  Unselected/absent topology is rejected by the Board
plan instead of receiving a hidden fallback device.

## Audit disposition

No reproducible timing-boundary defect was found.

In particular, this review found no second scheduler, no App-owned advancement
of guest time, no conversion of host presentation/input time into guest time,
and no profile claiming physical time where its configuration declares a macro
or unqualified axis.  The existing `L2_FALLBACK`, `NON_GUEST_TIME`, and
source-ratio classifications are already the minimal distinctions needed for
the selected profiles.

Analogue display fidelity, mechanical media timing, unselected peripherals,
and a physical bus-cycle model remain outside this audit.  They must be admitted
as separate work with a selected-profile source/evidence boundary; none is
silently upgraded by this result.

## Focused verification

The existing owner-local tests were run on both stable Ninja trees:

```text
build/.ninja/gcc-x64-release: 11/11 passed (0.49 s real)
build/.ninja/gcc-x86-release: 11/11 passed (0.62 s real)
```

The selection covered PIT, RTC, FDC, HDC, video and Compaq CECG chip contracts;
the shared profile contract; and Board timing qualification, controller
authority, plan publication, and timing checkpoint tests.  These tests prove
the composition/deadline contract, not runtime qualification of every physical
device condition.

## Result

No source, test, manifest, artifact, or ABI change is warranted for S12.  The
appropriate next action is owner review of this no-repair disposition; a future
selected-device defect may enter a later S with its own reproducer and evidence
grade.
