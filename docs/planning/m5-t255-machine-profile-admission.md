# M5 T255: Machine-Profile Admission Design

**Status:** S2 active.

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

## S2: Frozen VM Profile Contract

T255 defines a source-level architecture contract, not a versioned SDK, common
descriptor type, runtime selector, or loader implementation. A future VM
profile declaration is immutable and contains only these categories:

| Declaration category | Permitted content | Owner that consumes it |
| --- | --- | --- |
| Identity and capabilities | project-owned profile identity; CPU/FPU/RAM requirements; frozen clock ratios and admitted generic-core configuration | VM composition translates the generic values into `core_machine_config` and configuration-window bindings. |
| VM topology | VM device roster; port ranges; IRQ/DMA routes; controller-specific configuration; CMOS defaults | VM composition creates VM-only devices, then binds them through public core provider contracts. |
| Firmware | project-owned ROM bytes or an abstract ROM slot; mapping requirements; reset entry; firmware-hook metadata and immutable callback data | VM composition maps/binds it before freeze; profile firmware runs only through public machine contracts. |
| Media compatibility | abstract slots, supported geometry/type, boot ordering, and controller compatibility | Caller/product supplies a local path or creates media after composition has created the declared controller. |
| Pure mapping tables | host-key or host-pointer normalization tables that carry no host handle, queue, or session state | Composition selects and applies them at its input boundary. |

The declaration must not contain a live `core_machine`, VM controller, session,
thread, run handle, platform handle, debugger, mutable media object, local path,
CLI setting, Console/window policy, or product exit decision. A profile cannot
change topology, ROM, clock parameters, provider routes, or IRQ/DMA mapping
after construction begins.

### Construction, Freeze, And Teardown

1. VM composition selects one immutable profile and validates all declaration
   references before it creates a running session.
2. It creates one `core_machine` with the profile's generic capability values;
   during `INITIALIZED`, it translates allowed core configuration and creates
   VM-only devices and profile-firmware contexts.
3. It registers every provider, ROM mapping, port/IRQ/DMA binding, and firmware
   hook through the public core route, then freezes the machine exactly once.
4. It resets and runs the one frozen machine. Reconfiguration of RAM follows
   the existing explicit cold operation; profile selection and every other
   topology field require a new session.
5. On teardown, composition stops execution, finalizes profile/VM providers in
   reverse dependency order, destroys the core machine, then releases the
   profile declaration and its immutable assets.

No profile callback may reorder the above phases. No provider receives a global
profile or composition handle; it retains only the narrow immutable data and
public child references supplied at registration.

### Future External-ROM Manifest Boundary

An external ROM is not part of the profile declaration. A future profile may
declare a named abstract ROM slot with allowed mapping address, length, entry,
and read-only requirements. A separately approved VM-composition feature may
then accept a caller-owned manifest containing: profile identity and slot;
explicit local path; exact size; caller-provided SHA-256; requested mapping;
and a provenance statement that identifies the user's local source without
copying it into repository evidence.

Before core creation, that feature must open only the explicit local file,
verify size/digest/mapping against the profile slot, copy or retain immutable
bytes under the VM provider lifetime, and return a factual failure on absence,
read error, size/hash mismatch, illegal overlap, or invalid entry. It must not
download, search, enumerate, catalogue, persist, bundle, or make the ROM a
default. `core` receives only a generic immutable mapping provider and never a
vendor name, file path, manifest, or firmware-service meaning.

### Required Corpus For An Admitted Profile

Every profile must provide focused evidence for reset/boot, its declared
ports/IRQ/DMA routes, ROM mapping read/fetch/write protection, declared media
slot/controller behavior, and one guest-visible regression beyond POST. The
corpus must also retain the NXVM Console/debugger and relevant default-profile
media regressions. A successful POST alone is insufficient; unsupported ROM or
media must produce defined factual failure rather than a fallback profile.

## First Non-Default Profile Admission Template

| Required record | Admission requirement |
| --- | --- |
| Owner and source | Identify the `vm/profile/<name>` declaration, VM-only controller/provider code, composition binding site, and all profile-local assets. |
| Configuration source | State every frozen value and its authoritative source. Local media and BYOB ROM paths remain caller configuration, not source-controlled profile data. |
| Firmware legality/provenance | State whether firmware is project-owned or user supplied. For BYOB, record only manifest validation facts; never submit the ROM, a download URL, or a project hash catalogue. |
| Forbidden dependencies | Demonstrate no core model/vendor dependency, no profile-to-composition/platform/product include, no second machine/boot path, and no host shortcut into guest state. |
| Verification matrix | Include reset/boot, port/IRQ/DMA, ROM map, media, guest-visible fixture, retained Console/debugger, and current GCC/CTest gates. |
| Exit | Record declared/deferred behavior, artifact revision for behavior work, source/provenance evidence, and an owner-approved stop if a required capability crosses the contract. |

## S2 Evidence And Exit

S2 is complete when the architecture contracts, module layout, and source
policy state the same immutable declaration, composition-only construction,
and BYOB boundary as this record. It creates no runnable artifact.
