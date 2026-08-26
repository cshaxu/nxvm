# T484 S6 XT B3 Source And Owner Matrix

`M5:T484:S6:XT-B3-SOURCE-MATRIX:OK`

`M5:T484:S6:XT-NO-AT-ALIAS:OK`

## Method

This audit re-reads the accepted IBM *5155/5160 Technical Reference* in its
rendered form where OCR is ambiguous, then compares each selected relation with
the current Core and read-only 86Box/PCjs implementations.  IBM remains the
normative source.  Emulator observations are corroboration only; no external
code, ROM, media, path or timing value is imported.

## Keyboard/PPI

| Relation | Normative source fact | Current owner and gap | Minimal receiver |
| --- | --- | --- | --- |
| Guest register set | IBM BIOS/source listing labels 8255 ports `60h`--`63h`: scan/control A, diagnostic B, C and command. | Core `kbc.c` installs an AT 8042 at `60h`/`64h`, including command-byte, A20, reset and optional AUX state. `kbc_aux_absent` removes only AUX; it does not become an XT PPI. | One Core-owned XT PPI/keyboard attachment owns the four ports and its latches. It may reuse no 8042 command state or port provider. |
| Keyboard byte and IRQ1 | IBM identifies the scan-code port and keyboard IRQ1. | The existing KBC owns a FIFO and the PIC IRQ1 source, but its 8042 output-buffer condition is not the XT shift-register/PPI contract. | The new attachment publishes its own source-qualified byte-ready condition through the existing Core PIC owner; VM continues to submit typed host input and holds no guest keyboard state. |
| PPI B controls and status | IBM BIOS listing identifies speaker gate, refresh indication, I/O-check and parity status/control masks at `61h`/`62h`; POST switches port-A direction around diagnostic checkpoints. | PC/AT planar parity at `61h` is explicitly not the XT controller. Core has no 8255 owner or XT PPI direction/latch state. | The same one PPI attachment owns A/B/C/control-mode behavior, speaker gate and parity/NMI enable inputs. Exact unsourced timing stays out of this functional batch. |
| Keyboard reset/clock inhibition | IBM POST manipulates PPI-B while the keyboard shift register is disabled/enabled. | The AT KBC reset commands and `64h` protocol are incompatible. | Include only the manually evidenced PPI line behavior in the XT attachment; keyboard protocol bytes and timing require the selected keyboard source packet before implementation. |

86Box separately labels an XT keyboard device and PCjs keeps keyboard models
separate, corroborating that an XT keyboard must not be treated as an 8042
variant.  Their queues, host mappings and timing are not a specification.

## Diskette Attachment

| Relation | Normative source fact | Current owner and gap | Minimal receiver |
| --- | --- | --- | --- |
| Selected unit | The frozen 5160-268 catalogue selects one half-height diskette drive, but not its mechanism, capacity, geometry or media. | Core FDC/media owns the 8272A command state, IRQ/DRQ and logical image path. Its current PC/AT materializer selects a different drive/media profile. | No drive binding is admissible until an immutable XT drive/media declaration is sourced; do not select 1.44 MB or a phantom media image. |
| Adapter route | IBM BIOS uses adapter control `3F2h`; the IBM I/O-channel table supplies external `IRQ6` and `DRQ2` relations. | `core_machine_fdc_config` already supplies ports, IRQ and DMA channel through the copied Core plan. Current PC/AT configuration also exposes `3F7h` direction/control behavior. | The later XT FDC plan may bind only proved XT adapter ports plus IRQ6/DMA2. `3F7h` is not inherited: PCjs explicitly records it as Model-5170-only, a useful negative corroboration. |
| Reset/IRQ/DRQ | IBM BIOS resets through the adapter control and enables its diskette interrupts; Core FDC already owns DOR reset, result IRQ and bound DMA requests. | The Core state machine is reusable, but route selection must not be read from PC/AT descriptor leaves. | Profile supplies frozen port/IRQ/DMA values once sourced; Core retains reset, command, DRQ/IRQ and media state. |

## Result And Next Receiver

There is no compatible AT alias.  The future B3 implementation has two
independent Core owners: an XT PPI/keyboard attachment and the existing FDC
with a source-qualified copied configuration.  They meet only at existing PIC,
DMA and typed host-input/media boundaries; neither profile nor VM may own a
mirror of their guest state.

The selected drive type/media, keyboard protocol detail and every physical
delay remain source-gated.  The next implementation S must first establish
those immutable inputs, then add the PPI attachment and the exact FDC route in
one copied 5160 topology.  It must delete no independent AT behavior and add
no generic XT framework.
