# M5 T255: Machine-Profile Admission Design

**Status:** S1 active.

## Original Request

Define how future VM profiles, including PC110, DeskPro, and a locally supplied
Award or Phoenix ROM, can be admitted without teaching `core` about a machine
model or firmware vendor, copying the default PC/AT assembly, or adding a
third-party asset to the repository. This is a design and governance task: it
does not add a profile, load an external ROM, or change `nxvm.exe`; it creates
no developer artifact.

## Rules And Stop Conditions

- `core` remains unaware of a machine identity, ROM vendor, media path, and
  BIOS-service meaning. It owns only declared generic mechanics and provider
  registration/freeze.
- A VM profile is an immutable declaration. Composition constructs all live
  objects, binds providers, freezes topology, and owns reset/run/teardown.
- Firmware remains VM-profile content or an override provider. It uses public
  contracts only and has no platform, product, composition, or host-resource
  shortcut.
- A third-party ROM may never be committed, bundled, downloaded, catalogued,
  or selected as a default dependency. A future BYOB manifest is a separately
  admitted VM-composition feature, not an external SDK.
- Stop and split if a proposed profile requires a core dependency on model or
  vendor details, runtime topology/ROM/IRQ switching, a frozen-provider-route
  bypass, committed third-party firmware, or an NXVM Console/debugger/boot UX
  change.

## S1 Audit: Current Reference Profile

| Concern | Current owner and evidence | S1 disposition |
| --- | --- | --- |
| Generic CPU/RAM/time capability | `core_machine_config` is populated from the default descriptor in [session.c](../../src/vm/composition/session/session.c); core exposes only generic CPU/FPU, RAM, provider, and timing contracts. | Remains core capability/configuration; profile supplies values, never a core model name. |
| PC/AT topology | [pc_at_profile.h](../../src/vm/profile/default_profile/pc_at_profile.h) declares device roles, port ranges, IRQ/DMA routes, HDC PIO shape, CMOS defaults, and timing defaults. | Reference declaration for a future VM profile contract. Device roles and controller policy stay VM-specific. |
| Core profile binding | [session.c](../../src/vm/composition/session/session.c) applies text/EGA configurations only while the core configuration window is open. | Valid composition-only translation of profile declaration to generic core configuration; no profile holds a live core object. |
| VM-only machines | [machine_devices.c](../../src/vm/composition/session/machine_devices.c) initializes CMOS, FDC/FDD, and HDC/HDD from the selected default descriptor. | Composition creates live VM-only objects; a descriptor must not become a constructor or session-state owner. |
| Firmware and ROM | [profile_firmware.c](../../src/vm/composition/session/profile_firmware.c) maps project-owned default ROM bytes, constructs default BIOS context, and applies declared hook identifiers. | Firmware bytes, callback context, and service meaning remain profile-owned implementation; composition controls binding order and lifetime. |
| Lifecycle order | [provider_lifecycle.c](../../src/vm/composition/session/provider_lifecycle.c) retains the concrete device/firmware initialization, reset, refresh, and finalization sequence. | Composition owns this ordered sequence. A profile can declare requirements but cannot reorder lifecycle by callback. |
| Media | [session.c](../../src/vm/composition/session/session.c) keeps image paths and inserts them into VM FDD/HDD objects after session creation. | Paths and create/insert choices are caller/product policy, not profile data. A profile may declare abstract media-slot compatibility only. |
| Host/product policy | input mapping is selected in [session.c](../../src/vm/composition/session/session.c); Console and platform policy are outside `vm/profile`. | Excluded from the profile contract, except a profile may own a pure mapping table consumed by composition. |

### S1 Finding

The default descriptor is already an immutable PC/AT reference declaration, but
its types and composition are deliberately specific to `default_profile`.
`vm_session` directly owns default BIOS/context storage and composition uses a
concrete hook switch. That is correct for the one retained profile but is not a
generic multi-profile construction API. T255 freezes the *required shape and
ownership*, not a premature common C ABI or runtime selector.

## S1 Evidence And Exit

S1 is complete when the table above remains consistent with the referenced
source, and the S2 contract distinguishes declaration, construction, firmware,
and caller policy. The S1 review records no core dependency on PC/AT, ROM
vendor, image path, or BIOS-service semantics.
