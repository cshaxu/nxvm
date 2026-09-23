# T478 S1 Default-AT Derived-Profile Ledger

`M5:T478:S1:DEFAULT-AT-DELTA-LEDGER:OK`

`M5:T478:S1:DEFAULT-AT-PARITY-UNIVERSE:OK`

## Frozen Construction Boundary

`default-pc-at` is currently the direct branch in `vm_session_create`: it
selects the static descriptor, selects an optional CPU/FPU contract, copies a
Core configuration, and then independently applies a memory override. IBM 5170
instead resolves `pc-at-5170` first and copies its frozen Core input. This is
one real duplicate Core-input route, not a second Core implementation.

T478 replaces the Default-AT direct route with one immutable child declaration
of the already copied 5170 root. The child will be resolved before session
construction. A request may only select a profile-declared option before that
freeze; a session may retain dynamic media, presentation, input, debugger and
run-lifetime resources, but cannot rewrite profile topology or a frozen Core
plan afterwards.

## Complete Field Disposition

| Field group | Current Default-AT fact | T478 disposition and future owner |
| --- | --- | --- |
| Identity and parent | Static descriptor identity `default-pc-at`; direct session branch. | Immutable child identity `default-at`, parent `pc-at-5170`; VM resolver owns identity/provenance. |
| CPU/FPU and request policy | Default 80386/no FPU; direct session inputs presently select any validated legacy CPU/FPU contract, while the product parser only admits no FPU. | Child owns default Core selection. Existing resolver policy field carries the finite allowed request mask; S2 must reject every option not declared by this child and retain no direct session CPU/FPU selection. |
| Memory and reset | 16 MiB default, populated extended memory and 9FC0h FDC bounce. Direct session branch presently overwrites `memory_bytes`. | Child Core delta; the existing resolver policy field is the only admission of a bounded memory request before resolution. No post-freeze configuration mutation. |
| Instruction, transaction and time contracts | Default deterministic 80386 contract, unqualified axis and all controller rules L2. | Child Core delta, preserving every selected L2 disposition and making no physical/L3 claim. |
| Clock and KBC values | Unit/default rational ratios and zero KBC delays. | Child Core delta because they differ from the 5170 root's Model-339 source-rational contracts and typematic values. |
| Shared AT ports and routes | PIC/PIT/DMA/KBC/VADP/CMOS/FDC leaves and PIT/IRQ0, KBC/IRQ1+12, CMOS/IRQ8, FDC/IRQ6+DRQ2 routes match the descriptor arrays shared by the root. | Inherit. Resolver still owns one copied leaf/route graph; neither session nor Core receives a Default-AT branch. |
| Memory decode and display | CGA VRAM enabled, EGA enabled, no planar parity; same declared EGA port configuration but unlike the root's no-CGA window. | Child owns device/memory graph delta; VADP remains sole guest video/memory state owner. |
| Storage topology | ATA HDC enabled at 1F0--1F7/3F6, IRQ14, 16-bit PIO/LBA28; FDC is shared. | Child owns HDC device/port delta and builtin firmware service. Mounted FDD/HDD bytes and paths remain session media state. |
| Firmware, ROM and CMOS | Builtin generic firmware; same reset ROM location, but 639 KiB CMOS base memory and fixed-disk `F0h/2Fh` defaults. | Child policy/configuration delta; Core owns mutable CMOS after plan materialization and session owns only builtin-firmware lifecycle binding. |
| Floppy, AUX and product resources | No field-upgrade flag; common AUX route and shared 1.2 MB FDD default. Registry, display slot/mailbox, input, debugger, execution handle and boot preference are session/Core lifetimes. | Inherit the topology fact where unchanged; retain every listed dynamic owner outside resolver data. |

The ledger has no unclassified field. The intentional differences are not
Model-339 defects: 80386/default RAM, selectable pre-freeze session options,
unqualified/L2 timing, CGA/HDC/firmware/CMOS, and memory-display topology must
be explicit child values. Everything else is inherited only after byte-for-byte
comparison in S2.

## Source Sweep

The admitted sweep was:

```text
rg -l "VM_SESSION_PROFILE_DEFAULT_PC_AT|vm_profile_default_pc_at_descriptor_get\\(|vm_session_profile_select|vm_session_materialize_profile_core_config|vm_session_apply_core_config_overrides" src tests CMakeLists.txt
```

Production hits are `pc_at_profile.c` (static descriptor, Core materializer and
5170 root), `session.c` (the one direct Default-AT selection/materialization
branch), `session_interface.h` (profile request vocabulary), and
`session_factory.c` (product parser). The profile private header declares the
shared immutable descriptor types. No other production Default-AT constructor
exists. Test hits are the profile, PC/AT topology/composition, HDC, 5170 and
session-atomicity owner smokes listed below; each remains a parity consumer,
not a second production route.

## Finite Parity Universe

- `default-pc-at-profile-smoke`: descriptor values, contract selection and
  leaf/route facts.
- `vm-ibm-5170-model-339-composition-smoke`: Default-AT ATA preservation
  alongside the root's HDC rejection.
- `vm-pcat-topology-s2-smoke` and `vm-pcat-composition-s4-smoke`: common
  leaves/routes, topology and materialization behavior.
- `vm-hdc-port-smoke`: ATA port/PIO topology.
- `vm-session-initialization-atomicity-smoke`: default, requested memory/CPU
  construction and rollback behavior.

## S2 Boundary

S2 may add one Default-AT child declaration and resolver result using the
existing typed resolver fields, then add focused positive/negative resolver
proof. It must use the existing policy `allowed_session_options` representation
instead of widening the resolver kernel; it must delete the direct Default-AT
Core materialization and post-copy override path rather than wrap them. If the
existing policy field cannot express the finite request set without an
ambiguous local convention, stop for a renewed approved scope rather than add
parallel configuration state.
