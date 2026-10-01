# T540 S1 IBM PC Board-Reuse Audit

## Decision

Use a three-level board design, but create no source directory until a later
source S proves a concrete shared implementation:

```text
x86/ibmpc/common     proven PC wiring/mechanics shared without board policy
x86/ibmpc/xt         IBM 5160-only board wiring and 8-bit-era mechanisms
x86/ibmpc/at         proven AT-family wiring, never a model switch container
app composition       one thin machine-specific construction/asset/topology owner
```

Every machine retains a thin composition. This is not duplication: it owns the
machine's immutable topology, ROM roles, memory map, oscillator inputs and
which public-chip pins are connected. Shared code may own only a mechanism
whose state, reset lifetime and failure cleanup are identical for all of its
callers.

## Current Graph Findings

| Board | Must remain machine composition | Candidate shared board layer | Must not be generalized in T540 |
| --- | --- | --- | --- |
| IBM 5160 XT | XT ROM roles, 8-bit map, PPI/DIP/NMI/speaker inputs, XT keyboard topology, Xebec selection and XT memory constraints | `ibmpc/xt`: PIT0→IRQ0, PIT1→DMA-refresh where the existing board adapter proves the same lifecycle; XT port attachment helpers only if no AT caller needs different state | AT 8042/RTC/dual-PIC/DMA topology, EGA/Compaq rules |
| IBM 5170 AT | IBM ROM/CMOS roles, 1.2 MB drive topology, WD1003/ST-506 personality, exact assets and board clock values | `ibmpc/at`: public-chip construction/reset sequence and proven AT interrupt/DMA/RTC/KBC wiring | Model-40 D4 mapping, Compaq CMOS/ROM behavior, Model-40 FDC/HDC personalities |
| DeskPro Model 40 | Compaq ROM/CMOS seed, D4 memory map/parity, EGA/Compaq video, model-specific FDC status and WD/Compaq HDD personality | May consume a later small AT substrate only after direct call/lifetime comparison proves identical 8042, dual-PIC, DMA and RTC wiring | Any `if (model40)` switch inside Shared or an invented generic Compaq profile |
| Default PC/AT | Project BIOS, selected default media/firmware roles, default CPU/memory/topology choices | May consume the same AT substrate only for proven identical wiring, not because its name includes PC/AT | IBM or Compaq firmware policy, machine-name dispatch in Shared |

## Answer To The Architecture Question

XT needs a dedicated board layer. Its PPI/XT keyboard, NMI/DIP/speaker inputs,
8-bit-era topology and refresh/storage choices are not AT variations.

5170 and default PC/AT are candidates for an AT board substrate, not automatic
users of one. The substrate can contain only code with one public-chip
connection graph, one reset/finalize order and one failure boundary. Both
machines still keep composition files for their different ROMs, media and
immutable board values.

Model 40 is AT-class in some chips, but is not an ordinary 5170. It should use
the AT substrate only one proven mechanism at a time; its D4 memory, Compaq
firmware/CMOS, video and storage/FDC paths remain model-owned unless code
comparison establishes an identical contract.

## Candidate Boundaries For Later S Tasks

1. Inventory and extract a minimal shared construction/reset transaction only
   where XT and AT callers already have identical state ownership and rollback.
2. Extract XT-specific PPI/keyboard/NMI/refresh board attachment as
   `x86/ibmpc/xt`, with no AT conditionals.
3. Extract the smallest directly-proven AT wiring slice (dual PIC cascade,
   RTC/8042/DMA public bindings, if identical) as `x86/ibmpc/at`.
4. Reconnect 5170 and default independently; admit Model 40 only for the
   subset it proves identical. Retain each machine's composition and test it.
5. Only then remove duplicate NXVM board paths and establish shared/product
   test ownership. App splitting is a later T, not a side effect of T540.

## Evidence Anchors

- `src/app-nxvm/profiles/machine_plan.c` selects and materializes the current
  four immutable profile plans.
- `src/app-nxvm/profiles/{xt,model40,default_profile}` contain existing
  topology/ROM-specific construction records.
- `src/app-nxvm/devices/{cpu_bus,pic_bus,pit_bus,dma_bus,kbc,fdc,hdc,vadp}.c`
  are retained board adapters around the now-independent chips.
- `src/app-nxvm/devices/xt_ppi_keyboard.c` is explicitly XT wiring around
  `x86/ppi8255` and `x86/xtkeyboard`.
- `src/x86/CMakeLists.txt` already builds independent chip targets; no
  `x86/ibmpc` target currently exists.

S1 does not claim that any candidate is already identical. Later source work
must make those comparisons, preserve all product tests and stop if a proposed
shared path would create a machine switch or duplicate owner.
