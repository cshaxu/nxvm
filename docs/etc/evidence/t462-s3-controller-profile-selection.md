# T462 S3: Controller Profile Selection

S3 completes the selected-profile leg of the one-way T462 construction contract:
`profile descriptor -> CPU contract -> VM session copy -> Core machine plan -> existing controller owner`.

The profile has no controller pointer, callback, live setter, route mutation or clock loop. The session copies the four compact rule IDs once while it materializes `core_machine_config`, then applies them to the opaque Core plan before Core construction. Reset remains Core-owned and therefore replays the same copied plan rather than consulting profile state.

## Selected Values And Provenance

| Profile/controller | Selected construction value | Plan result | Provenance/disposition |
| --- | --- | --- | --- |
| Default PC/AT PIC, DMA, PIT | All four rule IDs are `L2_FALLBACK`. | PIC, DMA and PIT remain declared L2. | No source qualification is invented. |
| IBM 5170 Model 339 PIT | Existing `596591 / 4000000`, reset phase zero, plus `SOURCE_RATIONAL_CLOCK`. | PIT declaration is L3-required. | T375 Model-339 source-backed nominal PIT conversion; no host-wall-clock claim. |
| IBM 5170 Model 339 DMA | Existing IBM AT `3 / 8` nominal clock ratio plus `SOURCE_RATIONAL_CLOCK` and `SOURCE_DMA_SERVICE_PHASES`. | DMA declaration is L3-required. | T450's IBM AT 3 MHz/five-clock source record; the generic five-clock conversion remains explicitly L2. |
| IBM 5170 Model 339 PIC | `L2_FALLBACK`. | PIC declaration remains L2. | T456/T462 S1 established no elapsed Core-latency source. |

The ratios feed the existing Core clock domains. DMA transaction/arbitration, PIT OUT/GATE/IRQ0 routing and PIC IRQ assertion/acknowledgement continue to be their current dynamic owner paths; S3 adds neither a route selector nor a second state copy.

## Focused Proof

`vm-model-339-clock-contract-smoke` creates Model 339, observes its copied Core clocks and rule IDs, confirms PIC L2 plus DMA/PIT L3-required declarations, exercises the existing clock domains, and verifies reset replays the values. It emits:

- `M5:T462:S3:CONTROLLER-PROFILE-SELECTION:OK`
- `M5:T462:S3:CONTROLLER-OWNER-CONSUMPTION:OK`

`vm-default-pc-at-profile-smoke` proves all default descriptor rule IDs remain L2 and that the selected CPU contract is a byte-for-byte copy. Both targets were configured, built and directly run successfully on 2026-08-24.

The tracked source/test change is +52/-2 lines across six source/test paths (`pc_at_profile.c`, `pc_at_profile_private.h`, `session.c`, `session_private.h`, and the two focused smokes). The positive change is the single copied selection path and its two fixture assertions; no replaced runtime timing path existed to delete.

S4 remains responsible for the PIC final disposition, all-ledger closure, current gate and Release artifact.
