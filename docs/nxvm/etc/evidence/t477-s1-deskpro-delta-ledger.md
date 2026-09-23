# T477 S1 DeskPro 386 Derived-Profile Ledger

`M5:T477:S1:DESKPRO-DELTA-LEDGER:OK`

`M5:T477:S1:DESKPRO-PARITY-UNIVERSE:OK`

## Frozen Construction Boundary

The current Model-40 route is a separate `vm_session_create_model40_byob`
branch. It sets `model40_private`, leaves `session->profile` null, creates a
Core plan directly, and owns dynamic BYOB/media state in the session. That is
the exact duplicate composition route T477 must replace. It is not a second
Core implementation: Core remains the machine/device state owner once its plan
is built.

The future route is one copied `pc-at-5170` root followed by one immutable
DeskPro resolver declaration and copied Core input. It may retain a small
DeskPro-specific session materializer only for real dynamic resources that
cannot enter immutable declaration data. It must not retain an alternate
Model-40 Core-config, topology or controller constructor.

## Complete Field Disposition

| Field group | Current source | T477 disposition and future owner |
| --- | --- | --- |
| Profile identity | Session kind `compaq-deskpro-386-model-40`; current `profile` is null. | Explicit VM child identity; the resolved child descriptor becomes the sole session profile. |
| CPU/FPU and core configuration | `vm_profile_model40_core_config_initialize`: 80386/no FPU, 1 MiB, A20 first-to-second MiB, deterministic retirement, instruction/timing contracts and auxiliary PIT. | Explicit DeskPro Core delta. Resolver copies the existing Core input; Core owns the resulting mutable CPU/time state. |
| CPU and board timing qualification | Existing deterministic Model-40 contracts, including external-cycle and wait-window values. | Retained exact selected contracts; no physical-clock or whole-machine L3 upgrade. Any unresolved board timing remains its named L2/accepted receiver. |
| Shared PC/AT controller topology | PIC, PIT, DMA wiring, KBC/A20, RTC port/IRQ and FDC IRQ6/DMA2. | Inherit from `pc-at-5170` only where the actual Model-40 values match; each differing value below is child-owned. Core still owns controller state. |
| D4 memory/control | `model40.c` D4 backing memory and `model40_composition.c` D4 platform `{61h,0,2}` plus ROM compatibility loading. | DeskPro dynamic/session materializer: mutable backing bytes and ROM-derived contents cannot be resolver data. The D4 platform constants are an explicit child topology delta. |
| Firmware and ROM | Required BYOB even/odd ROM manifest, SHA validation, copied bytes and Model-40 firmware provider. | VM policy plus session-owned validated bytes. Immutable declaration records BYOB policy only; neither paths, hashes nor bytes enter root/child static state. |
| Display | EGA present, CGA absent, Compaq Enhanced Color personality, CECG, sequencer/controller defaults and 3C0/3D0 port topology. | Explicit child topology delta. VADP remains the sole guest video state owner; VM only supplies one copied configuration and presentation binding. |
| Diskette | One 1.2 MB 5.25-inch drive, FDC 3F2/3F4/3F5/3F7, IRQ6/DMA2 and DeskPro unready-read rule. | Explicit child delta. The mounted FDD is session media state, while controller state remains Core-owned. |
| Fixed disk | Compaq WD 40 MB HDC at 1F0--1F7/3F6/3F7, IRQ14, no LBA, optional validated second 40 MB session image. | Explicit child delta plus session-owned media bindings. The invariant 925/5/17 geometry is enforced when media is inserted, not stored as mutable profile state. |
| CMOS | Current Model-40 RTC defaults: floppy `20h`, fixed disk `2Fh`, base memory 1 MiB, equipment `21h`. | Explicit child delta; Core owns CMOS runtime bytes after plan construction. |
| Input and host transports | Common keyboard mapping/input queue, but Model-40 disables AUX mouse. | Inherit common VM transport; explicit child KBC capability delta. No session-side second KBC state. |
| Product resources | Core plan/machine, registry, display slot, mailbox, debugger, run handle and FDC terminal observation. | Session/Core lifetime owners only; never resolver fields. The terminal observation remains a scoped session callback, not a machine fact. |

No group is unclassified. The `pc-at-5170` root is not assumed to supply any
value simply because it is PC/AT: S2 must compare each copied root field with
the frozen value above and patch every difference explicitly.

## Finite Parity Universe

- `vm-model40-private-composition-s7-smoke`: external-ROM guard, 80386/1 MiB,
  timing contract, D4, speaker and AUX-disabled selection.
- `vm-model40-integration-s8-smoke`: reset, ROM mapping, D4 controls, FDC,
  HDC, RTC, display ports and host-input behavior.
- `vm-model40-cecg-s9` through `s13` and `s28` smokes: selected Compaq EGA
  personality and CECG register/port outcomes.
- `vm-model40-d4-compatibility-s25`, `d4-map-s16`, `d4-parity-s22` and
  `d4-skey-s23` smokes: D4 ROM backing, mapping/parity and control behavior.
- `vm-model40-fdc-s24`, `dma-s17`, `fdd-s18`, `hdc-s26` and BYOB smokes:
  selected storage/controller behavior and media boundaries.
- `vm-model40-console-s20-smoke`, session catalog and profile tests: visible
  profile identity, BYOB request validation and console lifecycle.

## S2 Boundary

S2 may add a single DeskPro child declaration and copied session root result.
It must first extend the resolver only if a frozen immutable field cannot be
represented by an existing typed field; an extension must remove a duplicate
constructor, not merely mirror Model-40 structures. It must preserve the
dynamic-ROM, media and D4 backing lifetime in the session, and must not expose
Core internals or add a DeskPro-named Core branch.
