# T472 S1: L2 Macro Pacing Contract

`M5:T472:S1:MACRO-PACING-CONTRACT:OK`

## Source Crosswalk

| Scope | Primary/manual fact | Independent implementation corroboration | T472 disposition |
| --- | --- | --- | --- |
| IBM 5170, March 1984 board | IBM Technical Reference 1502243, System Board 1-6: 80286 is 6 MHz (167 ns); its 8254 input is 1.19318 MHz. | MAME distinguishes its 6 MHz and 8 MHz 5170 BIOS families. | Retained as a distinct 6 MHz board fact; not assigned to the selected Rev-3 profile. |
| Current `ibm-5170-model-339` profile | Its already-selected abstract firmware slot is Rev-3 and its existing device ratios are defined against an 8 MHz nominal CPU source. | MAME identifies 8 MHz 5170 firmware families; PCjs Rev-3 5170 machine configurations label 8 MHz. | `MACRO_PROPORTIONAL`, 8,000,000 Core ticks/s, L2 only. |
| `default-pc-at` and DeskPro Model-40 | No selected board-wide source chain maps their present Core elapsed axis to a nominal machine rate. | External models do not establish that missing NXVM Core-axis mapping. | `UNQUALIFIED`; no host pacing and no fixed HLT wait. |

86Box, MAME, PCjs, Bochs and QEMU were inspected solely as read-only research
inputs. No source, firmware, test fixture, build input or dependency is copied
into NXVM. Bochs and QEMU do not provide an applicable selected-5170 board-rate
contract for this profile decision.

## Contract

Core copies a `time_axis` once during construction. Its three qualifications
are unqualified, macro-proportional and verified physical. Macro-proportional
publishes a copied rate to the time observation's pacing fields, but its
physical fields remain unavailable. Therefore it cannot satisfy the physical
retirement rules or close `TODO(Medium) Profile physical-timebase closure`.

The next S reuses the existing host-origin/Core-origin comparison. It does not
introduce a host tick accumulator, a Core time writer, profile callback, or
second scheduler.

## Focused Proof

- `core-machine-time-smoke` accepts macro qualification only with a positive
  rate and rejects physical retirement on it.
- `vm-model-339-clock-contract-smoke` observes 8,000,000 pacing ticks/s while
  physical time remains unavailable; it also corrects the stale post-gate PIT
  expectation to the profile's existing rational result (27 Core ticks).
- `vm-model40-byob-s20-smoke` observes neither pacing nor physical time.
