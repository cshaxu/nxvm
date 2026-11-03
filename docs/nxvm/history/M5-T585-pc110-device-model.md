# M5 T585 — MyPC110 remaining device and model qualification

Status: closed after S1–S23. The owner directed completion of the first four PC110
queue tasks. T583 and T584 are closed; the third receiver is the
[device and model proposal](../proposals/m5-pc110-device-model-qualification.md).

S1 audits the current owner paths, tests and source-backed evidence before
planning repairs. Existing boot and interaction evidence does not by itself
qualify PC Card, power, audio or physical display hardware.

The [S1 inventory](../etc/evidence/t585-device-model-inventory.md) records
the present positive evidence, gaps and planned sequential receivers.

S2 corrects card-detect status acknowledgement, shared global-register
addressing and invalid-index bounds. Dual-width qualification and the
remaining IRQ/window/card work are in the [S2 evidence](../etc/evidence/t585-s2-pcic-status.md).

S3 connects the modeled card-detect ISA status IRQ through PC110's existing
PIC and tests two sockets, acknowledgement and reset. INTR#/wake, other card
status inputs, windows and media remain explicit subsequent work; see
[S3 evidence](../etc/evidence/t585-s3-pcic-irq.md).

S4 adds the independent Ricoh memory-window decoder. Production Core routing,
card media and the remaining timing bit are explicitly retained for the next
receiver; see [S4 evidence](../etc/evidence/t585-s4-pcic-memory-decode.md).

S5 adds a runtime-selectable range to one existing Core overlay and validates
cache invalidation through actual 486 execution. The PC110 card backing and
guest route remain S6; see [S5 evidence](../etc/evidence/t585-s5-core-device-range.md).

S6–S11 connect memory-card media, both sockets, status/IRQ/wake and product
insertion/ejection; S12/S13 audit and correct the attribution of internal ATA.
S14–S19 implement bounded ES488 playback, OPL2 timer/melodic synthesis and
single-stream mixing, including auto-init DMA. S20 reconciles the remaining
receivers; S21 implements a 16550-compatible COM1 chip and PC110 IRQ4 route;
S22 verifies actual BIOS APM; and S23 completes the
[current device coverage ledger](../etc/evidence/t585-s23-device-coverage.md).

The final S21 cached unit suites passed 714/714 on each architecture. Registered
integrations passed 47/48 on each: native MIDI output could not be opened in
the isolated desktop, an owner-approved device-availability exception, and is
not represented as a pass. Selected XT, AT and DeskPro profile integrations
passed 3/3, 5/5 and 6/6 on each width. All fourteen artifacts were built and
checked for PE width. S22 changed only the existing firmware test and passed
its extended TFT cold boot on both widths; independent probes also checked
the derived DSTN firmware on both widths.

T585 closes the finite device/model reconciliation, not full physical hardware
emulation. The [coverage ledger](../etc/evidence/t585-s23-device-coverage.md)
and [TODO](../states/TODO.md) explicitly retain missing I/O-card transport,
host serial/IR/modem connection, dynamic battery/AC/lid, fuller sound synthesis
and panel fidelity. RAM and performance debt are separately retained. Final
current-product, input and RDP qualification belongs to the next T; the
derived DSTN firmware remains distinct from a verified factory-original image.
