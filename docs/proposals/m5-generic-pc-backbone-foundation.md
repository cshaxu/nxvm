# M5 Generic PC Backbone Foundation

## Purpose

After a comprehensive DeskPro 386 Model 40 L3 closure accepts the selected
profile, establish two reusable VM machine backbones: `generic-pc-xt` and
`generic-pc-at`. They let future named machines inherit declared standard
platform behavior without presenting a generic fallback as original machine
evidence.

CPU choice is orthogonal to a machine backbone. `generic-pc-xt` selects only
8086 or 8088 CPU contracts. `generic-pc-at` selects 80286, 80386, 80486 or
Pentium CPU contracts, but gains no later platform feature merely from that
selection.

```text
Core reusable mechanisms
  + VM generic-pc-xt or generic-pc-at
  + VM named-machine delta + selected device/media contracts
  = immutable resolved VM profile
```

Named profiles include IBM PC/XT 5160, IBM PC/AT 5170, Compaq DeskPro 386 and
IBM PC 110. A named profile states its selected backbone and explicit
`inherit`, `replace`, `disable` or allowed-addition operations only.

## Admission Preconditions

- A fresh DeskPro 386 Model 40 comprehensive L3 closure audit accepts its
  selected profile; an earlier not-ready audit is insufficient.
- The four-profile CPU correctness program supplies the CPU semantic and
  timing dispositions consumed by each selected CPU contract.
- The current-product device/profile closure freezes reusable device contracts,
  profile resolution and public-capability inventory.
- This candidate remains before the 8088 CPU-profile and PC/XT program; XT
  behavior cannot supply AT or 80386 evidence.

## Ownership And Composition

| Surface | Core owns | VM owns |
| --- | --- | --- |
| CPU and reusable hardware | CPU execution, memory/ROM primitives, generic PIC/DMA/PIT/RTC/FDC/HDC/display mechanisms, bus routes and checked binding interfaces. | Selection of a CPU contract or hardware combination. |
| Generic backbone | No machine name, model, default device set or firmware policy. | `generic-pc-xt` and `generic-pc-at`: fitted generic devices, topology, declared defaults, permitted variants, standalone startup contract and provenance. |
| Named machine | Reusable mechanism implementations only. | Backbone selection; machine delta; board wiring; ROM/firmware/media requirements; machine-local limits and timing. |
| Resolved profile | Checked capability inputs only. | Profile names, resolution policy, allowed combinations, provenance publication and immutable session configuration. |

Core must never select `generic-pc-at`, `DeskPro`, `5160`, `5170` or `PC 110`.
VM must not duplicate CPU execution or device-controller state machines.

The resolution ledger assigns every effective CPU attachment, memory/ROM
window, controller binding, port range, IRQ/DRQ route, reset fact, media rule
and timing-relevant default one owner and one provenance tier:

1. `original` — primary manufacturer, component or processor material;
2. `reference-derived` — named read-only emulator/reference cross-check, never
   imported; or
3. `generic-xt` / `generic-at` — declared standard-backbone fallback.

An outer layer may not silently fall back to a generic value. Resolution rejects
duplicate owners, undeclared replacement, disabled-but-bound devices,
conflicting port/IRQ/DRQ claims, invalid ROM mappings and disallowed media.

## Required Scope

First build a finite field and dependency inventory. The first implementation
may admit `generic-pc-at` only; `generic-pc-xt` has its own inventory and must
not be an alias. A 386, 486 or Pentium CPU selection inherits only the generic
AT platform. PCI, VLB, APIC, SMM, cache/chipset, integrated IDE, shadow/PAM RAM
and other later-platform facts require an explicitly admitted chipset/platform
or named-machine delta with its own evidence.

A minimal CPU execution fixture remains a Core test fixture, not a public VM
profile or a third machine-backbone layer.

## Verification And Completion Standard

Before closure require:

- a finite two-backbone resolution ledger with each selected field accepted,
  explicitly inapplicable or transferred to a named receiver;
- tests for inheritance, replacement, disable, conflict rejection, immutable
  post-publication state and provenance visibility;
- standalone composition regressions for each admitted generic backbone;
- named-machine regressions proving 5170 and DeskPro retain their frozen
  contracts and no generic default silently changes them;
- Core/VM boundary checks proving no named profile selection entered Core and
  no VM duplicate executor/controller implementation was introduced;
- source/profile/device/ROM/port/IRQ/DMA/reset/timing-owner sweeps, current
  gate and independent closure audit.

Closure claims only the declared composition contract and explicit fallback
tiers. It makes no claim of universal clone compatibility, physical board
timing, original DeskPro evidence, complete CPU correctness or PC/XT readiness.

## Non-goals And Stop Conditions

Do not add arbitrary YAML chip graphs, a universal chipset, per-machine CPU
executors, firmware or guest-media import, PS/2/MCA, PCI/VLB, ISA PnP, cache or
prefetch emulation, or inferred DeskPro timing. Do not migrate a named machine
only to make inheritance appear exercised.

Stop and transfer a field if it has no bounded original, reference-derived or
generic-backbone contract; resolution would change a frozen machine profile; or
implementation would widen beyond the Core/VM composition boundary. The
transfer names the earliest owner and evidence needed for admission.