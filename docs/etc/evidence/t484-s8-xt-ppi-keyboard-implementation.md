# T484 S8 XT PPI Keyboard Implementation

`M5:T484:S8:XT-PPI-KEYBOARD:OK`

`M5:T484:S8:XT-IRQ1-RESET:OK`

`M5:T484:S8:NO-8042-ALIAS:OK`

## Retained Owner And Data Flow

`core_machine_xt_ppi_keyboard` is the one Core owner for the selected IBM
5160 PPI keyboard attachment.  The immutable resolved profile supplies only
`60h`--`63h` and IRQ1.  Core owns the PPI latches, source-backed sixteen-byte
keyboard-side pending capacity, currently held port-A byte and PIC IRQ source.
The existing typed host-input boundary submits completed native scan-set-1
bytes to that owner; it has no guest FIFO or port-state copy.

The default topology retains the existing Intel 8042 unchanged.  Selecting
XT PPI prevents 8042 creation and therefore leaves `64h`, AUX, command byte,
A20/reset output and 8042 FIFO absent.  XT pointing-device reports return
`TYPE_STATUS_UNSUPPORTED` rather than reaching an absent controller.

The adapter implements Intel 8255A mode-set/BSR register behavior needed by
the selected board.  The selected IBM `99h` mode makes port A an input and
port B an output.  PPI-B bit 6 holds the keyboard clock low; bit 7 clears the
held keyboard byte.  Reading port A observes the held byte; it does not
silently invent a second consume rule.  The documented PPI-B clear pulse
deasserts IRQ1 and permits the next pending byte to publish.  A completed
keyboard BAT result follows the same typed-input, port-A and IRQ1 route; Core
does not fabricate a keyboard response or convert the source's physical delay
until the later XT board-time receiver qualifies that conversion.

## Focused Proof

`core-machine-xt-ppi-keyboard-smoke` proves:

- immutable XT `60h`--`63h`/IRQ1 construction, no `64h` port and scan-set-1;
- mode-set and port-C BSR latch semantics;
- make `1Eh`, break `9Eh`, PPI-B clear pulse, IRQ1 assertion/deassertion and
  clock-low inhibition;
- keyboard BAT `AAh` delivery after line release; and
- unchanged default PC/AT `64h`/scan-set-2 behavior.

The focused sweep also passed the existing 8042 controller, serial cadence and
AUX-port regressions plus the fixed XT resolver/profile smoke.  The complete
current-gate passed all 298 registered tests.  Documentation governance passed
for `vm-0-5-0484`.

## Similar-Issue Sweep And Simplicity Review

`rg -n -C 2 "shared_kbc|keyboard_receive|kbc_aux_absent|core_machine_kbc"
src/core/machine/machine.c src/core/machine/machine.h
src/core/machine/machine_interface.h src/core/machine/machine_plan.c
src/vm/profile/xt/xt_5160_268.c` found every former unconditional KBC lifecycle
site.  Each now dispatches once by immutable keyboard topology; no other XT
profile or session construction route exists.  The retained 8042 path remains
the default, with a distinct hardware state/port contract, rather than an XT
compatibility branch.

The implementation adds 376 source lines and removes 20, plus 117 test lines.
The positive size is one new stateful controller owner and its focused proof;
it replaces the previous false unconditional-8042 construction for XT rather
than layering an adapter, duplicate input queue or VM guest-state mirror.

## Developer Artifact

The optimized stripped Release `build/output/nxvm_0_5_0484.exe` was rebuilt
from this source at S8.  SHA-256:
`06A1252B3928E42C0DEF6504398EF0795BC72A8CA641FD26B216FE9FAA2174B9`.
