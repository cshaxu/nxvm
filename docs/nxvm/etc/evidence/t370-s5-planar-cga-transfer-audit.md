# T370 S5: Planar-Parity, CGA, And Transfer Audit

## Selected planar-parity NMI result

The selected Model-339 planar-RAM parity route is already one bounded source
path.  The shared physical RAM read/write owner maintains parity backing for
the configured 512 KB range; a mismatch calls the machine-owned planar-parity
producer.  That producer latches the fault, exposes it through PC/AT port
`61h`, and can publish the CPU NMI input only when parity is enabled and CMOS
port `70h` has not masked NMI.  Clearing parity through the selected port-B
control clears the latch/publication state; cold reset restores enabled,
unlatched state.  I/O-channel check and adapter parity are not selected.

IBM PC/AT material establishes the parity/I/O-channel-check topology and
port/mask context.  T366 S3/S4 supplies the selected lifecycle and focused
proof.  It does not supply RAM fault-detection propagation, port settling,
NMI latency, or a physical clock conversion.  `flagNMI` is a delivery consumer;
CMOS bit 7 is a mask; neither becomes a second source.  Thus this is a retained
logical source/latch/reset contract, not planar service timing.

## Selected CGA/VADP result

For the selected Model-339 CGA topology, `vadp.c` owns CGA CRTC/mode/color/
status state, CGA VRAM capture and selected text, `320x200x4` and
`640x200x2` decoding.  The model profile selects CGA-only ports and a
VADP-owned `B8000h` VRAM provider; EGA topology is absent.  VADP captures a
value snapshot, session code copies it to a presentation mailbox, and the host
consumer reads that copy only.  The sole peripheral callback advances KBC then
VADP with its deterministic project-domain ratio.

IBM CGA/PC/AT references and T352/T366 evidence determine selected topology,
register/aperture and raster-layout behavior.  They do not map the project
VADP `1/1` callback ratio to a CGA dot clock, scanline cadence, display-enable
window, memory contention, retrace duration, composite phase, or host refresh.
The callback trace is therefore ordering evidence only.  Existing CGA/VADP
status, graphics, topology and copied-frame tests retain their behavioral and
lifecycle proof without becoming a duration result.

## T370 selected-device completion and exact transfers

| Selected S1 group | T370 result | Remains open / ordered receiver |
| --- | --- | --- |
| PIT, PIC, RTC/CMOS | S2 retains protocol/event ordering and rejects project-ratio/RTC-scalar conversion. | Oscillator, propagation and INTA phase: selected-profile phase refinement. |
| Dual 8237A and FDC | S3 retains one DRQ/DMA2/IRQ6 lifecycle and rejects uPD765-to-core duration. | DREQ/DACK/AEN/HLDA, FDC controller and TEAC mechanics: selected-profile phase refinement. |
| 8042 and keyboard | S4 retains one command/FIFO/IRQ/reset owner; Model-339 delay fields are zero. | Controller/keyboard clock, serial exchange and response/typematic latency: selected-profile phase refinement. |
| Planar parity and CGA/VADP | S5 retains selected source/latch and display topology/copy behavior, with no timing scalar. | NMI propagation, CGA clocks/retrace/contention and device microstates: selected-profile phase refinement. |

Current-product ATA/HDC, bounded AUX breadth and all other supported device
capabilities remain the later current-product device-capability L3 closure;
they are not selected 5170 Model-339 evidence.  IBM MFM/ST-506 remains its
separate TODO and is not ATA.  The next Queue candidate is therefore
selected-profile model-L3 phase refinement, followed by the IBM 5170 model-L3
closure audit.  **T370 completes its bounded device-service reconciliation but
does not complete 5170 Model-339 L3.**

## Sweep and verification

The audit swept planar-parity/NMI/port-`61h`/CMOS-mask/reset/finalize routes,
Model-339 parity composition, VADP CGA port/aperture/raster/status/capture/
reset routes, profile topology, timeline trace and focused parity/CGA/VADP/
composition tests.  One owner remains at every selected boundary; no secondary
scheduler, NMI producer, display writer or mutable host-consumer route was
found.  Documentation governance and `git diff --check` are the S5 checks;
this reconciliation creates no runtime artifact or marker.
