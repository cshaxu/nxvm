# T499 S9: Model-40 D4 Current-Code List 2 And Owner Design

`M5:T499:S9:D4-LIST-2:OK`

## Current Owner Split

| Current surface | Present owner/path | Gap | S10 receiver/disposition |
| --- | --- | --- | --- |
| D4 compatibility backing, control, diagnostics, setup and parity mask | `vm_profile_model40_d4_memory` in `vm/profile/model40`. | One guest-visible controller state lives in a VM profile. | One private Core D4 state object. |
| D4 replacement/compatibility/control address dispatch | Four profile-owned memory-device callbacks are registered through generic Core dispatch. | Core routes physical access but the selected controller owns neither state nor decode. | Core D4 dispatch selected during frozen plan construction; delete profile callbacks. |
| Compatibility ROM initialization | The session copies the immutable firmware carrier into profile-private D4 backing before Core construction. | Profile retains the mutable D4 image and its lifetime. | Profile supplies immutable construction bytes once; Core copies and thereafter owns the D4 backing. |
| D4 parity/IOCHK | Profile records the parity mask and registers a profile write observer; a second plan path stores a pointer to that profile mask in Core. | Mutable profile pointer crosses the boundary and duplicate parity registration paths exist. | Core owns the mask, parity callback and ordinary-memory-write clear condition. Delete pointer/setter path and profile observer. |
| Model-40 reset | Provider lifecycle directly resets the profile D4 object. | Reset has a second controller owner outside Core. | Core reset resets D4 state; provider lifecycle loses the D4 reset call. |
| Static aliases/absent windows | Model-40 composition emits the selected reset aliases/absent windows for the D4 base/extension topology. | The prior `F40000h`--`F9FFFFh` aliases and `FA0000h` low-backing alias contradicted the selected External-L2 reset decode; setup-low `1` was a register-only no-op. | Composition supplies only the corrected frozen reset topology (`FA0000h`--`FDFFFFh` to `A0000h`--`DFFFFh`); Core D4 now owns the selected setup-low `1` extension-bank open-bus receiver. Other runtime selections remain unsupported rather than becoming profile callbacks. |
| Generic memory-device API | Core supports generic composition-owned overlays for unrelated mechanisms. | It is not itself a D4 duplicate. | Retain generic API; remove only D4's use of it. |
| Legacy direct D4 registration helpers | `vm_profile_model40_d4_memory_register()` and parity-enable helper have no production caller. | Dead parallel construction path. | Delete with profile D4 object. |

## Frozen Minimal Construction Contract

The selected Core plan gains one value-only D4 declaration containing only
source-classified construction facts: enabled flag, frozen diagnostic/setup
reset values and immutable even/odd compatibility-ROM lanes. Core owns the
fixed selected D4 decode and copies those lanes while constructing the machine;
no caller-owned pointer is retained after construction. The profile may select
this declaration once, but may not expose D4 state, memory callbacks, a reset
hook or a mutable configuration setter.

This is deliberately a selected component declaration, not a generic memory
controller framework. Ordinary Core RAM remains its existing single owner; the
separate D4 compatibility buffer is the one real D4 RAM image required by the
manual-defined compatibility/replacement relation, not a mirror of ordinary
RAM.

## Required S10 Deletions

S10 is complete only when the following production path no longer exists:

```text
session -> profile D4 object -> profile memory callbacks/observer -> Core dispatch
session/provider lifecycle -> profile D4 reset
Core plan -> pointer to profile D4 parity mask
```

It is replaced by:

```text
frozen profile plan value -> Core D4 construction -> Core D4 state/decode/reset
```

No profile-side forwarding wrapper, second image, direct CPU-memory mutation
or boot-specific route is admissible. The S8 ledger is the complete authority
for which control and setup writes may change state.
