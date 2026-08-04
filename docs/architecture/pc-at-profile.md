# Default PC/AT Profile

## T208 S1 Decision

The default PC/AT profile is immutable VM-owned declaration. It describes the
machine selected by NXVM; it does not own a second machine, mutable device
state, host handles, media images, or an execution loop. `vm/composition`
remains the sole creator of one `vm_session`, one `core_machine`, and the
session-owned VM-only devices.

The target division is:

```text
vm/profile/default_profile
  immutable PC/AT topology, ROM regions, port/IRQ/DMA routes,
  CMOS defaults, firmware service metadata, and profile firmware callbacks

vm/composition
  select descriptor -> create one session -> create real storage -> bind
  declared routes/providers -> freeze -> reset/run -> finalize

core/machine
  generic CPU, RAM, port bus, shared PIC/PIT/DMA/KBC/VADP, lifecycle, and
  provider contracts; no PC/AT address, IRQ, ROM, or BIOS policy
```

Profile declaration is not a machine constructor. Composition interprets it
against public core and VM machine APIs. A profile callback may provide a ROM,
POST, or interrupt-service override through a public firmware contract, but it
does not directly construct a session or call a VM sibling module.

## Current Split

The following behavior is currently real but dispersed:

| Concern | Current location | T208 destination |
| --- | --- | --- |
| Reset vector and ROM/BDA image bytes | `vm/profile/default_profile/firmware/default_profile.c` and `vm/composition/session/profile_firmware.c` | profile ROM and firmware declaration |
| Firmware service ordering and INT/POST wiring | `default_profile.c`, `profile_firmware.c` | profile firmware declaration |
| CMOS defaults and boot drive | `default_profile.c`, `vm/machine/cmos.*`, session device setup | profile CMOS declaration; session remains mutable CMOS owner |
| FDC, PIC, DMA, and port connection arguments | `vm/composition/session/machine_devices.c` | profile controller-route declaration interpreted by composition |
| FDD/HDD media storage and image selection | `vm_session` and `machine_devices.c` | remains composition/session policy, not profile data |
| qd* BIOS handlers | `vm/profile/default_profile/firmware/*` | retained as profile-specific temporary firmware providers |

## Descriptor Surface

T208 introduces one default-profile descriptor owned under
`src/vm/profile/default_profile/`. Its exact C spelling is decided in S2, but
its immutable fields are limited to these declarations:

- identity and compatibility revision;
- supported CPU/FPU profile limits and default memory topology;
- ROM regions, reset vector, and immutable ROM/BDA initialization plan;
- port ranges and device roles, without port-bus or device pointers;
- IRQ and DMA route records, without PIC/DMA pointers;
- CMOS byte defaults and boot-order policy, without mutable CMOS state;
- firmware POST/interrupt service metadata and profile firmware callbacks;
- VM-only controller topology such as FDC/FDD/HDD presence.

The descriptor must not contain host paths, media contents, host callbacks,
live pointers, copied CPU/RAM/device state, session identifiers, or mutable
configuration. External ROM support remains a later manifest/provider feature:
no Award, Phoenix, IBM, or other third-party ROM enters this descriptor.

## Composition Contract

Composition owns the only imperative sequence:

1. choose the immutable default PC/AT descriptor;
2. create `core_machine` and VM-only session storage once;
3. initialize generic core state through `core_machine` lifecycle only;
4. initialize VM-only CMOS/FDD/FDC/HDD/debug storage once;
5. apply the descriptor's mappings, routes, defaults, and firmware metadata;
6. bind profile firmware/provider callbacks, freeze configuration, then reset;
7. refresh and finalize in the existing owner-defined order.

`core_machine` continues to own generic shared-device lifecycle. The profile
may declare a route such as "FDC uses DMA channel 2 and IRQ 6", but composition
performs the binding against the actual session-owned FDC and core-owned DMA/
PIC objects. No descriptor application may duplicate storage, reset a device a
second time, or bypass the frozen configuration boundary.

## T208 Breakdown

### S1: Design And Inventory

Record the descriptor boundary, current dispersed values, prohibited state,
composition sequence, and regression contract. No runtime source movement.

### S2: Immutable Descriptor Introduction

Add the descriptor and direct unit/source-shape evidence. Preserve all current
values and startup order; composition may read the descriptor but no behavior
is intentionally changed. Stop for an additional machine/session, profile-to-
sibling dependency, or changed Console/debugger/boot behavior.

### S3: Route And Firmware Migration

Move default ROM mapping, CMOS defaults, POST/INT service metadata, and
FDC/PIC/DMA/port route constants from composition/firmware call sites into the
descriptor. Composition retains the imperative bindings. Add probes proving
the descriptor produces the existing reset vector, ports, IRQ/DMA routes, and
firmware service order.

### S4: Closure Audit

Remove superseded PC/AT constants from composition, reject them with a static
source gate, and retain current GCC smoke, FDD/HDD boot, DOS prompt, Console,
debugger, and two-session evidence. Produce the task-level T208 artifact only
if the runnable path changes during S2-S4.

## Admission Rules

T208 does not implement a new controller, external ROM loading, a new machine
model, CGA graphics, IDE, or 286/386 behavior. It creates the profile boundary
needed for those later device tasks to add a declared topology rather than
spreading new PC/AT constants across composition and firmware code.
