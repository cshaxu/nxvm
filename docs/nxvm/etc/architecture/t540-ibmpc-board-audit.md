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

## Source-Level Findings

The current implementation gives a more precise boundary than the broad
"XT versus AT" labels alone:

- `profiles/xt/xt_5160_268.c` constructs a one-DMA, single-PIC, 8253,
  XT-PPI-keyboard machine.  It also owns the 4.772727 MHz board input, 360 KiB
  floppy selection, Xebec attachment and XT absent-memory map.  This is a
  distinct `ibmpc/xt` construction family, not an argument to add flags to an
  AT constructor.
- `profiles/default_profile/pc_at_profile.c` already contains both Default
  PC/AT and IBM 5170 descriptors, and shares one descriptor-to-topology path.
  That proves useful reuse, but the descriptor chooses CPU/time values, video
  apertures, Port-B refresh behavior, CMOS defaults, HDC personality, FDC
  geometry, firmware roles and route leaves.  The whole file therefore cannot
  move to `x86/ibmpc/at`: doing so would make Shared own product/profile policy.
- `profiles/model40/model40.c` currently begins from
  `vm_profile_ibm_5170_values_create()` before replacing Model-40 values.
  This is an existing implementation shortcut, not proof that Model 40 is an
  AT instance.  A later source S must either demonstrate that the retained
  baseline fields are an explicit AT electrical contract, or construct its
  values directly.  The present hidden dependency must not be preserved as an
  `if (model40)` branch in Shared.
- `profiles/model40/composition.c` independently wires its two 1.2 MiB drives,
  Compaq WD-40MB HDC, D4 mapping, EGA/Compaq display and auxiliary PIT.  Those
  remain Model-40 composition responsibilities.
- `devices/dma_bus.c` currently combines XT-compatible primary-DMA/page-latch
  behavior with the AT secondary controller and cascade ports behind a runtime
  controller count.  It is not yet evidence for an AT-only extraction.  Its
  state and reset behavior need a direct XT/AT caller comparison before any
  split; introducing a second DMA adapter first would duplicate ownership.
- `devices/machine_board.c`, `pic_bus.c` and `pit_bus.c` own board wiring
  rather than chip state.  Their potential destination is determined by their
  actual public dependencies and reset order: a module that needs only the
  Core port/transaction contracts and independent chips may become
  `x86/ibmpc/*`; a module that needs product media, firmware or profile data
  must remain beneath the product composition boundary.
- `devices/machine_plan.c` is the unique Core-plan application transaction.
  It currently creates memory mappings and configures display, DMA, RTC, FDC,
  HDC and D4 in one ordered failure boundary.  It must remain a single owner;
  T540 must not clone this transaction into XT and AT helpers.  If it is moved
  later, it moves whole as a neutral Core/board construction service, not as
  an IBM-profile policy object.

## Resulting Destination Rule

The eventual directories are deliberately capability-oriented:

```text
src/x86/ibmpc/common/  one demonstrated IBM-PC wiring mechanism
src/x86/ibmpc/xt/      XT-only PPI/DIP/NMI/refresh and 8-bit board mechanics
src/x86/ibmpc/at/      demonstrated AT electrical mechanisms only
src/app-nxvm/profiles/ one composition per selected machine
```

`common` is not a fourth generic profile and `at` is not a descriptor switch.
For example, an identical 8254-to-IRQ0 connection may live in a board helper;
the choice to fit a 8253 or 8254, the oscillator ratio, and the reset-time
refresh connection stay with the applicable composition unless their complete
contract is identical.  A composition may call zero, one or several helpers.
That preserves a single owner for each line while allowing later independent
applications to reuse proven mechanics.

## Candidate Boundaries For Later S Tasks

1. Record an exact retained-adapter dependency ledger and classify each module
   as neutral Core construction, XT-only, AT-only, Model-40-only, or still
   mixed.  Mixed modules are not moved merely to make the directory tidy.
2. Extract one minimal, already-identical board mechanism at a time, retaining
   its one owner and one rollback path.  The first source package must not
   introduce a generic board object, callback registry or machine enum.
3. Extract XT-specific PPI/keyboard/NMI/refresh mechanics to `x86/ibmpc/xt`
   only after its port, PIT and PIC dependency boundary is explicit.
4. Extract the smallest directly-proven AT wiring slice to `x86/ibmpc/at`;
   reconnect 5170 and Default independently.  Model 40 joins only after a
   direct call/lifetime comparison for that exact slice.
5. Resolve the Model-40-from-5170 values shortcut as an explicit composition
   decision, never as an implicit inheritance leak.
6. Remove each old NXVM path in the same change that reconnects its consumer,
   then establish shared and product test ownership.  App splitting is a later
   T, not a side effect of T540.

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
- `test/app-nxvm/unit/core/machine/vm_xt_5160_268_profile_smoke.c`, the
  5170/direct-plan and Default-PC/AT composition smokes, and the Model-40
  D4/FDC/HDC/CMOS smokes already form the product regression boundary.  A
  later extraction must add a focused `test/x86/ibmpc` mechanism test only
  when a new public board module exists; it must retain these per-machine
  tests rather than replace them with one generic profile matrix.

S1 does not claim that any candidate is already identical. Later source work
must make those comparisons, preserve all product tests and stop if a proposed
shared path would create a machine switch or duplicate owner.
