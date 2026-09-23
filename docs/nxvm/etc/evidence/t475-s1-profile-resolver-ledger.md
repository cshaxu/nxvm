# T475 S1 Profile Resolver Ledger

`M5:T475:S1:PROFILE-RESOLVER-LEDGER:OK`

`M5:T475:S1:PROFILE-RESOLVER-ROUTE-SWEEP:OK`

## Frozen Universe

This is a finite construction inventory, not a claim that the selected
machines have identical hardware or L3 completeness. It covers exactly the
three current VM selections: `default-pc-at`, `ibm-5170-model-339`, and
`compaq-deskpro-386-model-40`.

| Group | Current source owner | Resolver disposition |
| --- | --- | --- |
| Identity, one parent, compatibility revision and provenance | profile declaration | Immutable VM identity/provenance field; no Core visibility. |
| CPU, FPU, instruction, transaction, clock, guest-time and controller timing contracts | `pc_at_profile.c` descriptor/CPU-contract selection; `model40.c` Core configuration | Neutral copied Core-contract field; resolver validates availability but creates no timing rule. |
| RAM selection, absent-memory and D4 parity | AT descriptor; Model-40 private composition | Immutable topology field; profile connection owns selected value. |
| Reset vector, ROM map and firmware slot/policy | AT descriptor and profile firmware; Model-40 BYOB manifest/provider | Immutable VM firmware-policy field; BYOB paths and bytes remain session input, never profile data. |
| Port leaves, memory decode and display topology | AT descriptor; Model-40 private composition | Immutable typed route/window field; resolver detects overlaps before a Core plan is built. |
| PIC, DMA, PIT, RTC, KBC, NMI, speaker, IRQ/DRQ/HOLD/BUSRDY routes | AT descriptor and session plan assembly; Model-40 private composition | Neutral Core topology/route fields; Core remains their mutable owner. |
| FDC, HDC, drive/media IDs and selected protocol | `machine_devices.c`; Model-40 private composition | Immutable device declaration and VM policy; live media registry/bindings remain construction-local resources. |
| Firmware service list and CMOS defaults | AT descriptor and `profile_firmware.c` | Immutable VM-only policy; no resolver callback or assembled byte stream. |
| Session memory/CPU/FPU, boot and media choices | `vm_session_config` and factory parser | Immutable session request after profile selection; resolver accepts only profile-declared choices. |
| Display/input/debugger/presentation and platform transports | session composition | Dynamic host/product resources; excluded from profile data and bound only after resolution. |
| Core machine, plan, controller/device state, elapsed time and deadlines | Core plan/machine APIs | Excluded: Core owns all mutable state after copied-plan construction. |
| BIOS object, FDD/HDD objects, retained path buffers and Model-40 ROM byte buffers | `vm_session` | Excluded: session-owned mutable/lifetime data; no snapshot or pointer crosses into the resolver. |

The frozen groups cover every field in
`vm_profile_default_pc_at_descriptor`,
`vm_profile_default_pc_at_cpu_contract`, `vm_session_config`, the
Model-40 Core configuration, and both production Core-plan builders. A future
field must enter one group above before it can be resolved.

## Current Route Ledger

| Route | Current owner and path | S1 disposition |
| --- | --- | --- |
| `default-pc-at` | `vm_session_create` selects `vm_profile_default_pc_at_descriptor_get`, selects CPU contract, materializes `core_machine_config`, then `vm_session_storage_initialize` builds the plan. | Direct legacy route. The later `default-at` connection deletes it after parity proof; S2 must not migrate it. |
| IBM 5170 Model 339 | Same `vm_session_create`/AT descriptor path, selecting `vm_profile_ibm_5170_model_339_descriptor_get`. | Direct legacy route. The following IBM 5170-root task replaces it with the resolver consumer. |
| DeskPro Model-40 | `vm_session_create_model40_byob` sets Model-40 configuration, then `vm_session_model40_storage_initialize` builds its plan. | Direct legacy route. The following DeskPro task replaces it; it is not a resolver special case. |
| Core plan publication | Both builders call `core_machine_plan_create`, populate neutral topology/device bindings, then call `core_machine_create_from_plan`. | Retained Core publication mechanism. S2 constructs only a copied resolver result suitable for this route; it introduces no second Core creation API. |
| Profile option parsing | `vm_session_provider_parse_options` parses current CLI names and session choices. | Current parser remains unchanged. YAML/parser cutover is a later candidate; resolver accepts a typed request, not raw strings. |
| Tests that call `vm_session_storage_initialize` | Machine tests use the existing session-private fixture seam. | Test-only setup, not a production configuration path. S2 tests resolver behavior directly and must not make this seam public. |

The production sweep finds exactly two Core-plan assembly implementations:
`session.c` for the two AT descriptors and `model40_composition.c` for the
current DeskPro path. It finds exactly one Core plan publication operation in
each (`core_machine_create_from_plan`), and no profile-name branch in Core.

## S2 Boundary

S2 creates one VM-private resolver module with:

```text
immutable declaration + one optional parent typed patch + immutable session request
    -> validated copied vm_resolved_profile
    -> one copied neutral Core-plan input
```

It validates identity/parent topology, ownership conflicts, selected Core
contract availability, device enablement, port/memory windows, IRQ/DRQ routes,
firmware/media policy and post-freeze mutation attempts. It has no profile
name branch and no pointers to Core machines, devices, host resources,
firmware/media bytes or session objects. Bounded neutral fixtures prove the
resolver; they are not a fourth selectable machine.

No current production route is changed in S2. The three following profile
connection candidates each consume this API once and remove their respective
direct route. This preserves one eventual production construction path rather
than adding a resolver facade beside permanent legacy composition.

## S1 Review

The source review covered descriptor selection, CPU contract materialization,
AT and Model-40 plan construction, controller/device setup, session factory
parsing, and all production calls to Core plan create/configure/publish APIs.
No additional current production profile construction route was found. This
S changes documentation only; it adds no runtime interface, code path,
artifact, external source, or machine-local data.
