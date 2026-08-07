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

| Concern | Current location | Intended owner |
| --- | --- | --- |
| Reset vector and ROM/BDA image bytes | `vm/composition/session/profile_firmware.c` and `vm/profile/default_profile/firmware/bios.c` | profile ROM and firmware declaration |
| Firmware service ordering and INT/POST wiring | `pc_at_profile.c`, `profile_firmware.c` | profile firmware declaration |
| CMOS defaults and boot drive | `pc_at_profile.c`, `core/machine/rtc.*`, session device setup | profile declares defaults; core owns the neutral mutable mechanism while VM owns PC/AT ports and NMI policy |
| FDC port and route arguments | `vm/composition/session/machine_devices.c` | profile controller-route declaration interpreted by composition; core retains PIC/DMA state and contracts |
| FDD/HDD media storage and image selection | `vm_session` and `machine_devices.c` | remains composition/session policy, not profile data; T270--T278 may move only neutral media/controller mechanisms to core |
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
4. initialize VM backing/debug storage and any not-yet-migrated VM controller
   storage once;
5. apply the descriptor's mappings, routes, defaults, and firmware metadata;
6. bind profile firmware/provider callbacks, freeze configuration, then reset;
7. refresh and finalize in the existing owner-defined order.

`core_machine` continues to own generic shared-device lifecycle. The profile
may declare a route such as "FDC uses DMA channel 2 and IRQ 6", but composition
performs the binding against the actual session-owned FDC and core-owned DMA/
PIC objects. T273/T276/T278 may instead bind a core-owned neutral controller
to descriptor-declared routes. No descriptor application may duplicate
storage, reset a device a second time, or bypass the frozen configuration
boundary.

## T208 Breakdown

### S1: Design And Inventory

Record the descriptor boundary, current dispersed values, prohibited state,
composition sequence, and regression contract. No runtime source movement.

### S2: Immutable Descriptor Introduction

Add the descriptor and direct unit/source-shape evidence. Preserve all current
values and startup order; composition may read the descriptor but no behavior
is intentionally changed. Stop for an additional machine/session, profile-to-
sibling dependency, or changed Console/debugger/boot behavior.

**Completed:** `pc_at_profile.*` owns the immutable default descriptor. Every
session selects it before storage creation; direct session initialization uses
the same selection at the storage boundary. Its smoke covers the identity, ROM
mapping, CMOS defaults, declared ports, and FDC route.

### S3: Route And Firmware Migration

Move default ROM mapping, CMOS defaults, POST/INT service metadata, and
FDC/PIC/DMA/port route constants from composition/firmware call sites into the
descriptor. Composition retains the imperative bindings. Add probes proving
the descriptor produces the existing reset vector, ports, IRQ/DMA routes, and
firmware service order.

**Completed:** live ROM mapping and firmware-vector ordering read from the
descriptor. The retained VM CMOS and FDC receive descriptor-derived
configuration, so they do not choose PC/AT ports, FDC IRQ, or DMA channel.
T273/T276 may move their neutral mechanisms into core without changing that
descriptor authority. CMOS defaults are exposed as an explicit device
initializer for a later behavior admission; this task preserves the legacy
reset contents. The application smoke verifies the live FDC configuration and
the declared CMOS initializer.

### S4: Closure Audit

Remove superseded PC/AT constants from composition, reject them with a static
source gate, and retain current GCC smoke, FDD/HDD boot, DOS prompt, Console,
debugger, and two-session evidence. Produce the task-level T208 artifact only
if the runnable path changes during S2-S4.

**Completed:** `verify-default-pc-at-profile-closure` rejects ROM/CMOS/FDC
route constants outside the descriptor and device configuration boundary. T208's
artifact is `nxvm_0_5_0208.exe`; current artifact identity is recorded by
`docs/planning/status.md`. The T208-focused profile/application
smokes, current DOS-prompt, keyboard, MEM, Console, debugger, platform, and
two-session smokes pass. The descriptor preserves the pre-T208 default CPU
selection (`80386`) and `FPU=none`; changing those compatibility defaults is a
separate CPU-profile admission, not profile cleanup.

## Admission Rules

T208 does not implement a new controller, external ROM loading, a new machine
model, CGA graphics, IDE, or 286/386 behavior. It creates the profile boundary
needed for those later device tasks to add a declared topology rather than
spreading new PC/AT constants across composition and firmware code.
