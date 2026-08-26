# T484 S20 XT B6 Session Contract

`M5:T484:S20:XT-B6-SESSION-CONTRACT:OK`

## Qualified Boundary

The selected `ibm-5160-model-268` profile is already a complete immutable
Core plan: 256 KiB, 8088, one PIC, one DMA controller, XT PPI keyboard, CGA,
one 360 KiB FDC route, and the selected Xebec Type-2 route.  Its resolver
truthfully declares `BYOB` firmware, no dynamic profile override and no
selected session media policy.  Direct Core-plan tests prove that topology;
they do not make a product session runnable.

IBM *PC/XT Technical Reference* 6139821 (1986 edition) qualifies the 64 KiB
base-system-ROM window at `F0000h`--`FFFFFh`.  IBM *PC 20MB Fixed Disk Drive
Adapter* 6139790 (pp. 14--15) qualifies the optional Xebec ROM window at
`C8000h`--`C9FFFh`.  The latter is needed only when an owner supplies that
adapter ROM and requests the adapter-firmware boot path; it is not an excuse
to bundle, catalogue or discover vendor bytes.  The original-reference copy
used for the former is externally retained; its rendered system-memory map
was cross-read.  No ROM bytes, hash, local path or third-party source enter
this record.

The source policy is the legal and architectural authority for the manifest:
each selected slot names a user-local path, exact byte size, SHA-256,
read-only mapping and user provenance.  Paths and provenance are transient
composition inputs only.  The repository retains neither the input nor a
vendor/revision catalogue.  Core receives a generic immutable mapping
provider, never a file path or manifest.

## Frozen B6 Contract

| Item | Required construction fact | Owner and failure boundary | Current disposition |
| --- | --- | --- | --- |
| Profile | Only `ibm-5160-model-268`; all CPU, board and device choices remain its copied resolved plan. | Resolver validates immutable topology before session creation. Invalid/missing plan fails before Core construction. | Reusable. |
| System-ROM slot | One required `xt-base-system` immutable read-only `F0000h`--`FFFFFh` window, exactly 64 KiB after user-side assembly. Its manifest record carries path, `65536` size, SHA-256 and provenance. | VM composition opens and validates the external bytes before allocating/binding the Core machine. A hash, size or mapping failure leaves no runnable session. | Missing. |
| Xebec option slot | One optional `xt-xebec-option` immutable read-only `C8000h`--`C9FFFh` window, exactly 8 KiB if supplied. It uses the same manifest facts. | VM composition owns absence versus validation failure. Core only sees a present immutable mapping. No supplied option ROM means no claim of Xebec-ROM boot support. | Missing. |
| Session media | The profile fixes drive/controller geometry and routes; user media remains a session resource, not a profile mutation. A 360 KiB floppy follows the existing single FDC/media owner. A Type-2 HDD follows the existing Xebec/Core-media owner. | The sole session registry opens/owns images before run and reports insertion/geometry failures. No controller, CHS state or media cache is created in VM. | Blocked by an explicit XT rejection. |
| Firmware execution | The external mapping provider is the only XT firmware provider. Default-PC/AT synthesized POST, interrupt services and boot logic are not bound for XT. | Core owns immutable map installation and reset execution; VM composition owns validation/lifetime and maps provider failure to session creation failure. | Blocked by the non-Model-40 default-provider branch. |
| Product request | Existing typed `SESSION OPEN` request remains the only command authority. Its typed request gains the source-policy-required XT manifest fields; it does not add a command spelling, selector, discovery workflow or a second parser. | Product/session factory parses once, copies a frozen config, then hands it to the sole session creator. Wrong-profile fields are rejected there. | Missing XT fields and current provider explicitly rejects XT media/boot. |

`xt-base-system` is a logical profile slot, not a claim about a particular
physical chip revision.  Owner-selected revision/chip composition is made
into that exact immutable logical image outside the repository.  That keeps
the Core mapping contract independent of a vendor-ROM catalogue while
preserving the source-backed address/size constraint.

## One Future Data Flow

```text
typed SESSION OPEN request
        -> sole session factory validates one XT BYOB manifest
        -> session copies resolved XT plan + validated immutable ROM mappings
        -> Core installs plan, maps ROMs, and owns reset/device execution
        -> Core emits copied display/input-observable state to VM presentation
```

The media registry joins this flow only as a session-owned construction
resource.  It never becomes firmware state or a second HDC/FDC owner.  The
existing Model-40 interleaved two-chip loader is distinct: it has a different
source-qualified slot layout and may share only a genuinely generic validator
after a separate duplication review.  It is not a generic XT ROM mapper.

## Current-Code Gap

The resolved XT profile reaches `vm_session_create()`, but that function
returns `TYPE_STATUS_UNSUPPORTED`.  Even if that temporary stop is removed,
three non-Model-40 assumptions still prevent a correct route:

1. `vm_session_machine_storage_initialize()` and
   `vm_session_machine_devices_materialize_plan()` require a valid
   default-PC/AT descriptor instead of consuming the copied XT topology.
2. `vm_session_bind_execution_provider()` selects Model-40 firmware or the
   default synthesized-PC/AT provider.  Binding the latter to XT would invent
   an AT BIOS route and violate the selected BYOB policy.
3. `vm_session_provider_request_configure()` has no XT manifest fields and
   rejects XT floppy, HDD and every non-ROM boot selection.  This is a product
   request policy gap, not evidence that XT hardware lacks those routes.

The repair therefore replaces the false binary classification
`model40 ? model40 : default-at` with one finite profile-selected construction
choice.  It must delete the XT unsupported/rejection branches as their
selected replacements arrive; it must not retain an XT exception manager,
parallel session constructor, PC/AT alias, or a firmware-only side channel.

## Successive Acceptance

The immediate implementation receiver may do only the following as one
cohesive session-boundary change:

1. Add an XT-specific transient manifest shape and typed request fields;
   validate the required base-system slot and, if present, the Xebec option
   slot under the source policy.
2. Generalize existing session composition around the already copied profile
   plan so default-PC/AT, Model-40 and XT each select their one firmware and
   device-materialization route without a second session owner.
3. Permit only the profile-fixed 360 KiB FDD and Type-2 Xebec session media;
   prove rejected mutation/foreign-profile fields, mapping/hash failure,
   reset-vector mapping and both existing profile regressions.

An XT session with no user base-system ROM remains a clean creation failure.
An XT session with only the base-system ROM may run from ROM/floppy but must
not claim Xebec option-ROM boot capability.  Exact controller service timing,
physical drive behavior and XT board phase timing remain their existing later
receivers; this contract neither estimates them nor changes L-levels.

## Coordinator Review

The primary-reference mapping facts, existing source-policy terms and actual
session/profile source were reconciled here.  The contract has one immutable
input boundary, one session creator and one Core state owner.  It leaves no
asset import, local path, firmware revision, new product grammar or implicit
AT fallback for the next implementation S to decide.
