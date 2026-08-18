# T402 S1: DeskPro 386 D4 Memory-Controller Matrix

`M5:T402:S1:D4-MATRIX:OK`

`M5:T402:S1:D4-CONTROL-WINDOW:OK`

## Scope and source boundary

This is the functional D4 reconciliation batch for the selected private 1 MiB
DeskPro 386 Model-40 composition.  It uses retained original 1986 Compaq D3PE
Processor Board material first, then read-only local 86Box and PCjs behavior
only where D3PE does not specify a software-visible register subform.  No
source text, ROM, firmware, guest media, local asset path, or third-party code
is imported into NXVM.

D3PE names the D4 diagnostic aperture at `80C00000h`, its diagnostic/parity
logic, the `E0000h`--`FFFFFh` replacement RAM selection and the
`FE0000h`--`FFFFFFh` compatibility RAM.  Its D4-RM32 and D4-RRAS decode
material establishes the selected address ownership but not the complete
byte-level diagnostic/setup register view.  The latter is therefore Tier 2;
it is an explicitly bounded compatibility decision, not a physical D4 claim.

## Functional matrix

| D4 member | Evidence tier and disposition | Current owner and proof |
| --- | --- | --- |
| `E0000h`--`FFFFFh` replacement window | Primary D3PE; conforming from T386 S16.  Control bit 0 selects D4 replacement RAM or declines to the immutable lower route. | Model-40 D4 profile; `vm-model40-d4-map-s16-smoke`. |
| `FE0000h`--`FFFFFFh` compatibility RAM and `FFEE0000h` high alias | Primary D3PE; conforming from T386 S16.  The project-owned backing is distinct from ROM loading. | Model-40 D4 profile; map smoke and T386 S16 evidence. |
| Control byte `80C00000h + 0` | Primary D3PE; conforming.  Read exposes selected diagnostics/parity; write controls replacement/protection and clears the selected parity latch. | Model-40 D4 profile; map and parity smokes. |
| Diagnostic high byte `+1` | Tier 2, reference-derived.  Local 86Box and PCjs expose the second diagnostic byte; for NXVM's fixed 1 MiB private configuration it resets to `FDh`. | Model-40 D4 profile; new map-smoke reads. |
| RAM setup bytes `+2`/`+3` | Tier 2, reference-derived.  Both local references expose the setup word.  The low byte is stored; high byte remains the selected configuration value (`FCh`).  It changes no timing, cache or memory-size behavior. | Model-40 D4 profile; new map-smoke read/write/reset proof. |
| Same control page outside the four selected bytes | Tier 2, reference-derived.  Both local references register a 4 KiB aperture and treat unimplemented positions as nonresponsive.  NXVM reads `FFh` and ignores writes in that page. | Model-40 D4 profile; new map-smoke `+4` proof. |
| D4 parity diagnostic and IOCHK clearing | Primary D3PE; conforming from T386 S22. | Existing Model-40 D4 consumer and `vm-model40-d4-parity-s22-smoke`. |
| RAM size/setup hardware effect, cache-control effect, DRAM RAS/CAS/READY/DMA phases, ISA waits and physical decode latency | Not determined by this functional batch; transferred. | Earliest owner is the later Model-40 board/device timing receiver; no Core or VM public interface change is admitted here. |

## Repair and similar-issue sweep

The observed defect class was an undersized D4 control registration: NXVM
registered only `80C00000h`, while the selected reference behavior has the
four software-visible control/diagnostic bytes and a nonresponsive page.
The sweep covered all Model-40 D4 production registrations, callbacks, reset
state, replacement/compatibility aliases, parity paths and focused D4 tests:

- `src/vm/profile/model40/model40.c` and `.h`: repaired the sole D4 control
  device from one byte to a Tier-2-labelled 4 KiB window; only offsets 0--3
  carry the documented functional state.
- `tests/machine/vm_model40_d4_map_s16_smoke.c`: covers all four bytes, low
  setup write/reset and an unimplemented-page byte.
- `tests/machine/vm_model40_d4_parity_s22_smoke.c`: remains the sole selected
  parity/IOCHK regression and passes unchanged.
- No shared-Core D4-specific hit exists.  Core retains generic callback-based
  memory routing and receives no profile-specific path or ABI.

## Verification and transfer

Focused build and CTest passed for
`current.vm-model40-d4-map-s16-smoke` and
`current.vm-model40-d4-parity-s22-smoke`.  The P1 acceptance record adds the
full current gate, documentation-governance result, actual-diff review and
pushed commit.

This evidence accepts functional mapping/control visibility only.  It does not
claim an original complete D4 setup register definition, RAM expansion model,
external-cache implementation, bus timing, DRAM timing, firmware behavior,
reference-machine identity, physical DeskPro fidelity, or L3 readiness.  Those
items remain explicit transfers to the later board/device timing receiver.
