# T475 S2 Profile Resolver

`M5:T475:S2:PROFILE-RESOLVER:OK`

`M5:T475:S2:PROFILE-RESOLVER-NEGATIVE:OK`

## Retained Mechanism

`profile_resolver_interface.h` and `profile_resolver.c` provide the one VM
resolver mechanism. An immutable declaration has one parent pointer, a typed
provided/owned field mask and copied values. Resolution recursively applies
the parent, applies the declared typed replacement, records the final owner of
each field, and returns a wholly copied `vm_resolved_profile`.

The result carries neutral Core-plan input (`core_machine_config`, controller
timing rules and a catalog-selected contract ID), enabled-device mask, port and
memory windows, IRQ/DRQ routes, firmware/media policy and session option
allowance. It contains no machine, plan, device, session, firmware/media-byte,
host-resource or callback pointer. Core still creates, copies and owns the
actual machine plan and all mutable guest state.

The catalog is a caller-supplied copied ID list. It confirms that a selected
neutral Core contract is available without exposing Core internals or creating
a profile-name branch in Core. The only exposed cross-module values are copied
public Core value types already used by the existing plan boundary.

## Validation Matrix

| Fixture | Result |
| --- | --- |
| Root plus one child patch | Accepts; child replaces only Core input, inherits all remaining groups, and records child/Core versus root/policy provenance. |
| Result after source declaration mutation | Result stays unchanged: identity, owners and values were copied. |
| Ownership mask without a matching typed replacement | Rejects; a child cannot claim an inherited field as a second owner. |
| Bound device absent from enabled-device mask | Rejects. |
| Overlapping port windows | Rejects before any Core plan is constructed. |
| Port window outside the 16-bit I/O domain | Rejects before any Core plan is constructed. |
| Duplicate IRQ line or duplicate DRQ line | Rejects before any Core plan is constructed. |
| Parent cycle/depth violation | Rejects; declarations form a bounded single-parent chain only. |
| Contract ID absent from the catalog | Rejects. |
| Invalid firmware/media policy or session request outside declared allowance | Rejects. |

`vm-profile-resolver-smoke` compiles under the VM-profile target's strict C
flags and prints both markers above. The exact owner-test inventory changes
from 177 to 178 pure targets because this is a new focused test executable;
the pre-existing mixed count remains three. The corresponding deferred-owner
verifier count changes from 180 to 181 source rows and its S2 target inventory
from 177 to 178; both values are derived from the same registered target.

The same smoke passes the resolved copied `core_machine_config` and copied
controller timing rules directly to the existing `core_machine_plan_create` /
`core_machine_plan_set_controller_timing_rules` APIs, then destroys the plan.
This proves the resolver output is the existing plan input, rather than a
parallel construction API or a mutable Core handle.

## Boundary And Transfer

S2 deliberately does not select or register a real machine. The current AT
and DeskPro direct construction paths remain the only current production
routes, exactly as frozen by S1. The later IBM 5170, DeskPro and default-AT
connection tasks each consume this one resolver and delete their respective
legacy direct path after parity proof. No resolver facade is retained beside a
migrated production route.
