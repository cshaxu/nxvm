# T456 S1 PIC Source, Reference And Contract Reconciliation

## Scope And Method

This S1 consumes, and does not renumber or duplicate, T450 S3's frozen
`PIC-R1`--`PIC-T5` universe and T450 S4's current-code audit. The universe has
18 rows: five register/initialization rows, eight functional rows and five
timing/boundary rows.

The Intel 8259A data sheet, order `231468-003` (Dec. 1988), is the normative
chip source. Its local file hash remains
`71F62B2FA1F13F6547DCEE26A2D570BF9BCFCFE6F2F84B24BC01053BB6153533`.
S1 rendered and visually inspected printed pages 3--23, including the
functional description, interrupt sequence, ICW/OCW diagrams, cascade figure,
and AC timing tables/waveforms. It is a scan with an OCR text layer: OCR was
used only to locate content; rendered tables, diagrams and prose governed the
result. IBM *Personal Computer AT Technical Reference* supplies only the
selected two-PIC port and IRQ2 wiring.

The reference sources below were inspected only to discover disagreement or a
missing interpretation. No code, test, dependency or requirement was imported.
Their observations are not Level-1 evidence.

| Reference | Fixed observation source | Relevant observation |
| --- | --- | --- |
| 86Box | `86Box` commit `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`; `src/pic.c`, SHA-256 `A2130FFEA43506DD3EEEFE1C68F01C039A2EB031F87B996B4A2A095F1401C3DA` | Uses ICW3 bits to decide whether a master line has a slave; implements poll, special mask and PC/AT cascade/spurious handling. |
| Bochs | release 2.6 source, `iodev/pic.cc`, SHA-256 `BD6760AA5310CABFF4809751AF3FAB6A0A0A5AA006A2FA00202C9A4B5BB24831` | Models one PC/AT slave fixed to master IRQ2; implements ICW/OCW, poll, AEOI and SFNM within that selected wiring. |
| MAME | `mamedev/mame` master `63e3a8f9009f541a6438a8f6c6c3a52c669b4de6`; `src/devices/machine/pic8259.cpp` | Consumes programmed cascade state and exposes an x86 acknowledge callback; records special-mask support as TODO. |
| PCjs | `jeffpar/pcjs` commit `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`; `machines/pcx86/modules/v2/chipset.js`, SHA-256 `F9F64438E30A6BE9B333FE542AB408478228E59F4FAA185C021CA2D3C830E1A7` | Uses the IBM AT IRQ2 route but explicitly leaves poll and special-mask support TODO. |
| QEMU | `qemu/qemu` master `2be159078ea26feac4c9c9902acf8906f1a05c2a`; `hw/intc/i8259.c` | Implements ICW/OCW, edge/level, poll, special mask, AEOI and spurious handling; its PC integration wires the slave to IRQ2. |

## Source Result And Reference Reconciliation

| Retained rows | Rendered Intel/IBM result | Reference reconciliation | S1 disposition |
| --- | --- | --- | --- |
| `PIC-R1`, `PIC-F1`, `PIC-T1` | IRR/ISR/IMR, edge/level recognition and static/no-clock character are explicit. | All five implement the same conceptual state; framework timers do not add a chip clock fact. | Level 1; current owner remains `pic.c`. |
| `PIC-R2`, `PIC-R3` | ICW1 clears defined state and starts a sequence; ICW3 is required in cascade and maps master slave inputs/slave identity. | 86Box and MAME consume the configuration. Bochs, PCjs and QEMU fix IRQ2 because their selected PC/AT composition does so. | Level 1; current Core accepts/stores ICW3/SNGL but selection ignores it, so S3 must repair the one selection path. |
| `PIC-R4`, `PIC-F2`--`PIC-F4` | OCW1/2/3, priority, EOI/AEOI, rotation, special mask and SFNM are explicit. | PCjs and MAME visibly omit or defer some modes; 86Box, Bochs and QEMU cover them. A majority cannot weaken the manual. | Level 1; retain the existing single Core mechanism and prove it in S4. |
| `PIC-R5`, `PIC-F5` | IRR/ISR/IMR reads and poll acknowledgement are explicit; poll overrides status-read selection. | PCjs leaves poll TODO; the other four implement an acknowledgement form. | Level 1; current Core path is retained and receives regression proof in S4. |
| `PIC-F6`, `PIC-F7`, `PIC-T4` | One master may select slaves through ICW3/CAS; IBM AT selects two controllers, the four ports and the IRQ2 cascade route. | Generic-chip references consume configuration; PC/AT integrations hard-wire IRQ2. These are different scopes, not a vote. | Level 1 chip semantics plus Level 1 selected AT composition. S3 makes default IRQ2 composition data while selection consumes the programmed state. |
| `PIC-F8` | In 8086/88 mode, the first INTA resolves/sets ISR/clears IRR and the second supplies the vector; default IR7 is documented when no request remains at acknowledgement. | MAME exposes acknowledgement explicitly; QEMU returns a vector through a PC IRQ callback; the other references bind it to their CPU framework. No reference supplies NXVM's transaction phase. | Level 1 logical order; Level 2 receiver for the Core command-to-visible/CPU-delivery boundary in S2. Electrical waveform stays Level 3/out of scope. |
| `PIC-T2`, `PIC-T5` | Causal request-to-INT-to-acknowledgement ordering is explicit; no elapsed Core scheduling quantum is supplied. | Each emulator supplies a framework-specific scheduling route, and those routes are not transferable. | Level 2: S2 declares one logical Core phase, never a derived delay. |
| `PIC-T3` | Setup, hold, propagation and INTA waveforms have numerical electrical values. | References approximate or schedule them differently. | Level 3 maintained boundary: no Core-tick conversion or emulator-derived number. |

## Resulting Implementation Boundary

The references uncover one real conflict in current Core state: `pic.c` accepts
and stores `ICW3` and `SNGL`, then `core_machine_pic_select`, SFNM selection
and `core_machine_pic_get_interrupt` treat master IRQ2 as the cascade line.
The manual distinguishes chip configuration from selected IBM AT wiring.
Therefore S3 must make the existing selection path consume its programmed
cascade relation while the machine's standard initialization supplies the AT
IRQ2 default. It must not add a second AT-only dispatch path.

The source also confirms the other receiver: current `scan`/`peek`/`get`
operations collapse the 8086 two-logical-INTA relationship into direct CPU
vector consumption. The manual fixes the causal order but not an NXVM elapsed
delay. S2 consequently belongs at the existing T449 transaction/arbitration
boundary, with PIC retaining state ownership and the CPU retaining consumption.

## S2--S4 Admission Plan

| Subtask | Single outcome | Exit evidence |
| --- | --- | --- |
| S2 | One transaction-owner request-visible-acknowledgement contract that preserves the manual's two logical 8086/88 INTA steps without claiming electrical duration. | Focused command-to-visible trace and CPU/PIC regression show one delivery route and preserved reset/cancellation. |
| S3 | One PIC selection mechanism consumes ICW3/SNGL; the IBM AT IRQ2 route is supplied as initialized composition data, with ICW1 reinitialization resetting the relation. | Focused non-default ICW3/SNGL and AT-default cascade/reinitialization regressions. |
| S4 | Every retained command/lifecycle row has a focused proof and no superseded helper or parallel delivery path remains. | 18-row closure table, command/priority/poll/mask/EOI/AEOI/reset/cascade tests, full required gates and minimalism audit. |

## Explicit Non-Claims

This record makes no claim about electrical INTA width, setup/hold,
propagation, default-IR7 race reproduction, arbitrary board wiring or a
reference emulator's scheduler. Those need a separately admitted physical
transaction/board receiver. The data sheet's unspecified post-acknowledgement
INT reassertion interval is not converted into an NXVM delay.
