# T386 S4: DeskPro D4 Platform Control

`M5:T386:S4:D4-PLATFORM-PORT:OK`

`M5:T386:S4:D4-NMI-MASK:OK`

`M5:T386:S4:D4-FAILSAFE-ROUTE:OK`

`M5:T386:S4:D4-RESET-ISOLATION:OK`

## Scope And Source Boundary

T384's retained primary-source audit establishes the original DeskPro 386 D4
NMI topology: CMOS index-port bit 7 masks NMI, port `61h` bit 3/control and
bit 6/status serve expansion-bus IOCHK, and bits 2/control and 7/status serve
the failsafe timer. It also establishes the D4 8042/8742 plus D4-SKEY A20 and
reset paths and second 8254 at `48h-4Bh`.

The manufacturer maintenance guide is a supporting diagnostic cross-check: its
Port-61 tests require cold-visible bits 0, 1 and 3 high, bits 2 and 7 low, and
separately test the failsafe interrupt and NMI. The owner-managed external
DeskPro 386/16 Rev-E ROM was inspected transiently only. Its diagnostic code
reads port `61h`, sets and clears bits 2/3, polls status bits 6/7, and programs
each `4Bh` counter-select control word. Neither ROM nor a derivative is in
this repository.

The ROM sequence supports the logical enable-clear behavior below, but not a
physical pulse width, latch propagation delay, PAL truth table, or a board
clock conversion. Those remain timing work.

## Implemented Owner Contract

`core_machine_configure_d4_platform()` is an optional configuration-phase
receiver. It requires the already configured auxiliary PIT, claims only
`61h`, and rejects a preexisting port owner or occupied selected PIT output.
The Model-40 carrier does not call it yet, so default PC/AT and Model-339
composition remain unchanged and no DeskPro profile becomes selectable.

| Concern | S4 behavior | Boundary |
| --- | --- | --- |
| IOCHK | Report API latches bit 6; bit 3 gates NMI publication. | An actual DeskPro expansion-bus producer remains board composition/timing work. |
| Failsafe | Config-selected auxiliary-PIT output latches bit 7; bit 2 gates its NMI publication. | The receiver does not claim an electrical waveform or system speed control. |
| NMI mask | Both sources use existing CMOS-owned `70h` bit-7 mask; unmask refreshes the selected pending source. | CPU delivery/priority remains the accepted shared CPU owner. |
| Port `61h` | Cold value is `0Bh`; writes retain low controls and an observed disabled control clears its matching logical latch. | Bits 0/1 and non-NMI speaker semantics are visible only; audio is unimplemented here. |
| KBC A20/reset | Existing KBC remains sole owner of output-port A20/reset mutation. The D4 port path neither changes the output byte nor requests reset. | D4-SKEY PAL transformation and reset duration remain board work. |
| Reset/finalize | Cold reset clears D4 latches/signalled state and restores `0Bh`; PIT reset deasserts its callback. | Physical reset sequencing/settling remains timing work. |

## Focused Proof

`core-machine-d4-platform-s4-smoke` creates a core machine with an explicit
`48h-4Bh` auxiliary PIT, configures the D4 receiver at `61h`, and proves:

- `61h` cold state is `0Bh`, while default D4 failsafe enable/status are low;
- a masked IOCHK source latches without publication, then publishes after CMOS
  NMI unmask;
- auxiliary PIT counter 0 mode-0 completion latches the selected failsafe
  source and publishes NMI only through the enabled route;
- `61h` write does not mutate the 8042 reset/A20 output byte; and
- reset restores D4 state and clears NMI/latches.

The same Git-Bash/WinLibs GCC 16.1 build ran the focused executable and emitted
all four markers. A fresh `build/t386-s4-gitbash` then built
`run-current-smokes`; that aggregate target again omitted the preexisting
current-gate dependency set, so the six targets
`vm-fdc-dma-boundary-smoke`, `vm-two-session-isolation-smoke`,
`vm-core-executor-storage-smoke`, `core-machine-executor-run-smoke`,
`core-machine-ram-create-smoke`, and `core-machine-port-assembly-smoke` were
explicitly built before the final current-gate run. `ctest -L current-gate
--parallel 4` passed 253/253. `verify-documentation-governance` then passed;
to make that Windows PowerShell gate readable, this P mechanically replaces the
six pre-existing non-ASCII em dashes in the already-referenced T386 S1 ledger
with ASCII `--`, without changing its requirements or conclusions.

## Similar-Issue Sweep

The recorded search was:

```text
rg -n -i "port.?61|0x0061|planar_parity|set_nmi_mask|auxiliary_pit|kbc.*output|request_reset" src tests CMakeLists.txt docs/etc/evidence
```

- Existing `planar_parity` hits are the selected IBM Model-339 memory-parity
  owner and remain untouched; its current-gate smoke passed.
- Existing `set_nmi_mask` is the shared CMOS/CPU publication boundary; S4 adds
  a second source refresh there without changing mask ownership.
- Existing auxiliary-PIT hits are S3's optional receiver; S4 is its first
  explicitly selected output consumer and rejects output reuse.
- Existing KBC reset/A20 hits remain KBC-owned. No second mutation route was
  added.
- Default-profile/session hits only compose IBM parity and remain a negative
  control; S4 makes no Model-40 carrier or catalog change.

## Transfer

S5 retains the selected Compaq storage/FDC work. S6 retains the Compaq EGA
personality. S7 may compose these owners only after S3--S6 are accepted. The
DeskPro board/timing candidate retains D4 PIT source cadence, IOCHK bus
producer, PAL logic, reset duration and physical availability. This evidence
makes no runnable DeskPro, firmware, device-completeness, timing or L3 claim.