# VM Profile Resolution And Core Machine-Plan Design

M5 Td S110 supporting design. This document elaborates the principal System Architecture and the Core L3 timing design. It is not an active task, Queue change, ABI change, or assertion that the target already exists.

## Scope

The architecture retains IBM PC/XT 5160 and PC/AT 5170 as its two eventual roots. The current implementation program is deliberately restricted to the existing AT tree: pc-at-5170, default-at, and the DeskPro 386 delta. XT admission remains later Queue work; 80486 and P5 are outside this design.

## Decisions

Project A owns reusable Core execution, transaction, device-state and virtual-time mechanisms. L3 is a deterministic documented constant, formula, bounded-range selection, or labelled reference-derived contract; L2 is an explicit proportional fallback; L4 electrical/measurement reproduction is prohibited.

Project B owns VM machine identity, topology, firmware/media policy, provenance, profile inheritance, resolution, and user-session policy. VM selects Core contracts but never implements a CPU timing algorithm or controller state machine.

```text
ibm-5160-xt (deferred root)

pc-at-5170
|- default-at
- compaq-deskpro-386-model-40
```

The two architecture roots are ibm-5160-xt and pc-at-5170. They are independent; XT is never an AT parent. The current VM program admits only the pc-at-5170 tree; 5160 admission remains later Queue work. Every other built-in profile has exactly one profile parent ultimately ending at one of those roots. A profile is the only machine-selection concept: no variant, no second profile-variable layer, and no multiple inheritance. Private reusable fragments may share typed data, but are not selectable profiles and cannot create a diamond.

## Profile Contract And Resolution

A profile is immutable declarative data, never live machine pointers, host callbacks, media bytes, mutable controller state, or user paths. Every effective field records one owner, evidence tier (original, reference-derived, generic-xt, or generic-at), timing disposition (selected L3 contract, explicit L2 fallback, or not applicable), and neutral Core contract ID where consumed.

Before migration, freeze a finite field/dependency universe: CPU and clocks; RAM, ROM, reset and A20; ports and memory decode; PIC, DMA, PIT, RTC, KBC, NMI and speaker; FDC/HDC/media; display; firmware policy; and every IRQ, DRQ, HOLD, BUSRDY, wait and reset dependency. Children may inherit, replace, disable, or make a predeclared allowed add only.

The resolver recursively resolves the parent and applies typed patches. It rejects duplicate ownership, disabled-but-bound devices, port/memory overlap, IRQ/DRQ conflict, invalid ROM/memory windows, unavailable Core contracts, broken dependencies, and disallowed media or firmware policy. Its output is a copied immutable vm_resolved_profile with VM-only identity, provenance, evidence links, and session-policy limits. Resolution never constructs a partial machine to discover validity.

## Core Boundary

Core remains machine-neutral: no machine name, firmware name, or VM profile enum enters Core. VM is the composition root and validates profile identity, inheritance, provenance, and user policy. Core validates neutral types, ranges, routes, timing selections, and topology.

The target input is a single copied, validated, frozen core_machine_plan. It replaces VM-visible sequences of partial configuration as the long-term composition boundary. Existing core_machine_config and core_machine_configure_* calls are migration source material, not a second permanent production path. The plan contains neutral clock/reset and CPU selections; instruction and transaction/arbitration contracts; memory/ROM windows; device topology, decode and IRQ/DRQ/HOLD routes; controller phase/reset/cancellation/wait selections; and observation declarations. Core copies and validates the complete graph before publication, freezes it, and owns later state transitions. VM never drives controller state through profile callbacks.

Core exposes copied capability/configuration snapshots for profile resolution and parity testing, without exposing mutable Core layout.

## Session YAML

YAML is session configuration, not profile authoring. Its custom grammar is nxvm-session; there is no v1/v2 compatibility contract. A future grammar replacement is direct and governed, because there are no external compatibility users.

```yaml
schema: nxvm-session
profile: compaq-deskpro-386-model-40
memory_kib: 4096
boot: fixed-disk
firmware:
  byob_manifest: session-local-approved-manifest
media:
  fixed_disk: session-local-approved-image
```

The single profile field selects one built-in profile. YAML may select only choices declared by that profile, such as allowed memory, boot order, approved BYOB manifest, and session-local media. It cannot declare a parent, topology, CPU, ports, IRQs, DRQs, ROM windows, timing constants, chip graph, or variant. Parsing validates YAML against the selected profile's allowed session schema and yields an immutable session request; profile resolution and Core-plan construction then validate the rest.

## Incremental Migration

1. Freeze profile fields, dependencies, capability IDs, current-path probes, and explicit L2 exceptions.
2. Add immutable typed profile data, resolver validation, resolved-profile snapshots, and a plan adapter; preserve the current composer behind it.
3. Add the complete copied core_machine_plan validation/freeze boundary and compare its Core snapshot to current 5170 composition.
4. Migrate pc-at-5170, prove reset/ROM/routes/devices/timing-contract parity, cut over, and delete its legacy production path.
5. Migrate DeskPro 386 directly as a named pc-at-5170 child. Retain reference-derived and generic-at labels; do not turn generic behavior into false Compaq evidence.
6. Complete the same resolver and migrate default-at as the second explicit pc-at-5170 child.
7. Replace the hard-coded session parser with nxvm-session after all three AT profiles have parity, then delete superseded profile-specific composition branches.

At every cutover, the old and new paths are compared through immutable Core snapshots and focused reset, mapping, route, device, firmware-policy and timing-contract regressions. New becomes the only production path before the next migration. A mismatch stops that increment; no profile-local callback or silent generic fallback is permitted.

## Acceptance And Governance

A profile migration needs a complete field ledger, available selected Core contracts, owner-visible L2 exceptions, and resolver conflict/provenance/immutability tests plus existing composition regressions. This is an L3 composition framework, not an assertion that every selected device already has L3 coverage.

Promotion to principal architecture, root-profile admission, or Core/VM ABI change requires a separately approved governance or implementation task. This supporting document does not itself change runtime behavior or the Core/VM ABI.