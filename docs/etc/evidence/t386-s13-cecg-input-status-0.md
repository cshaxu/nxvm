# T386 S13: CECG Input Status 0 And SW1 Selector

`M5:T386:S13:CECG-INPUT-STATUS-0:OK`

## Primary Contract

The primary source is Compaq, *COMPAQ Enhanced Color Graphics Board / COMPAQ
Color Monitor Technical Reference Guide*, first edition (December 1986),
sections 3.2 and 4.1--4.2. It was consulted transiently under the source
policy; no scan, ROM, firmware, guest media, local path, hash, binary,
third-party source, or source text is retained in Git. The source is available
for review at the [Internet Archive record](https://archive.org/details/trg-enhanced-color-graphics-board-1986-12).

| Primary-determined observable | S13 owner and disposition |
| --- | --- |
| Miscellaneous Output `3C2h` bits 2--3 select master-clock source and multiplex one of SW1 positions 4, 3, 2, then 1 into Input Status 0 bit 4. | Shared VADP owns the mutable selector, input-status read, validation and reset recovery. |
| Input Status 0 bits 5--6 report high when Special Features inputs are high or open; bit 7 is set when vertical-retrace IRQ is disabled. | The Model-40 declaration selects no Special Features board and disabled vertical-retrace IRQ. VADP therefore exposes the source-determined static `1` values without inventing a pin, IRQ producer or physical monitor. |
| Model-40 physical switch/reset facts are machine-specific. | Private VM composition alone declares the selected 640x350 Color Monitor-compatible SW1 state (`OFF/ON/ON/OFF`, mask `06h`) and selector reset `01b`; it owns neither port dispatch nor mutable hardware state. |

## Owner Boundary And Proof

`core/machine/vadp` remains the only CECG port/register/state owner. Its
extended generic CECG declaration carries static board facts; its runtime state
holds only the selected `3C2h` clock/SW1 selector. `vm` supplies the selected
Model-40 declaration through private composition. Generic EGA does not receive
Input Status 0 registration or CECG state.

- `core-machine-compaq-cecg-s13-smoke` proves all four selector-to-SW1 mappings,
  static no-Special-Features/no-IRQ bits, reset restoration and generic-EGA
  isolation.
- `vm-model40-cecg-s13-smoke` proves the selected Model-40 composition reaches
  the same state through the session lifecycle and reset using a project-owned
  synthetic ROM fixture.
- The retained S9--S12 controls were rebuilt and pass with the expanded
  declaration, preserving their CECG control, Feature/Environment, CPU-memory
  gate and I/O-base contracts.

## Similar-Issue Sweep And Transfers

The S13 sweep covered CECG declaration initializers, `3C2h` registrations and
read/write handlers, reset, Model-40 composition, generic-EGA setup, all S9--S13
controls, current-gate membership, strict-owner counts, evidence indexing and
CECG TODO wording. It found one shared declaration expansion, so every direct
initializer is explicit and both T345 inventory assertions advance by two
owned current-gate tests.

S13 deliberately does not make the selected master clock change raster
cadence, monitor output, blanking, page selection, sync polarity or ISA
availability. It also does not model Special Features signal transitions,
vertical-retrace IRQ delivery, option-ROM/BIOS programming, physical monitor
signalling, firmware publication, board waits, or L3 timing. Those receivers
remain in `TODO.md` and the T386 functional/board proposals.

## Verification

- Focused retained S9--S13 CECG controls: 10/10 passed.
- `verify-t345-deferred-direct-ownership`: passed with 215 rows, 155 owner
  tests, 7 embedded production tests, 1 type foundation, 2 safely separable
  production entries, 50 mixed/inherited entries and 57 exact residual entries.
- Canonical `run-current-smokes` aggregate completed without a test failure.
- Packet-required serial CTest replay: 268/268 current-gate tests passed, with
  zero `Test Failed` records in `LastTest.log`.
- Documentation governance is rerun at P1 closure.

This is selected-device functional progress only. It makes no runnable public
DeskPro profile, firmware, physical-board, board-timing or L3 claim.

## P2 Corrective Formatting

Coordinator review rejected P1 solely because the two new C controls and this
evidence file lacked their required final newline. P2 restores that source and
document format without changing code, test behavior, ownership, verification
scope or transfer boundary. The focused S13 controls and the completed serial
268/268 replay remain the behavioral evidence.
