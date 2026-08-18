# T395 S1 Core Memory-Device Route Registry Audit

## Decision

Core retains sole ownership of memory-device route registration and its storage.
The previous twelve-entry inline array was an artificial composition limit. It is
replaced by private growable storage with an explicit 64-provider safety limit.
Capacity starts at 12 and doubles only while configuration is open; the final
growth is capped at 64. Allocation or limit failure occurs before a provider or
write observer is published.

## Reconciled Surface

| Surface | Disposition |
| --- | --- |
| `src/core/machine/memory.c` registration, lookup, freeze and finalization | Fixed: private pointer/capacity storage, bounded reserve before every append, cleanup on finalization. |
| `src/core/machine/memory.h` | Fixed: private Core state only; no VM/profile-facing registry or public ABI surface added. |
| `src/core/machine/memory_interface.c` | Existing overlay registration forwards to Core; no change required. |
| `src/core/machine/rom_mapping_interface.c` | Existing rollback removes registered providers through Core-owned private storage; pointer indexing remains valid and no ROM semantics changed. |
| `src/core/machine/vadp.c` | Existing ordinary and provider-plus-observer paths are both protected by pre-publication reserve; no change required. |
| `src/core/machine/machine.c` and test fixtures | Existing provider callers use the unchanged Core registration API; no change required. |
| Model-40 and BYOB composition smokes | Re-run; no profile decode, firmware, board-clock or physical-timing selection changed. |

## Retained Semantics And Regressions

- Route lookup remains registration ordered. A provider that declines with
  `TYPE_STATUS_UNSUPPORTED` falls through to the next registered provider or
  ordinary RAM; any other result remains terminal.
- An overlay retains its existing overlap admission behavior. It does not gain
  implicit priority over an earlier provider.
- Frozen registration keeps its existing `TYPE_STATUS_INVALID_ARGUMENT` result.
- A direct private-Core allocation hook fails the first growth attempt after
  twelve registrations and proves both count and capacity remain twelve; after
  removing the hook, the same registration grows and succeeds.
- The EGA transaction regression fills all 64 entries and confirms the
  provider-plus-observer registration returns `TYPE_STATUS_NO_MEMORY` with no
  VADP configuration, provider or observer publication. Its existing injected
  VADP allocation failure also remains rollback-clean.
- The same owned regression grows past 12 entries, verifies first-provider
  priority then decline fallback to an overlay, and verifies frozen rejection.

## Verification

- `ctest --test-dir build/mingw-gcc-x64 -R "core-machine-ega-registration-transaction-smoke|vm-model40-private-composition-s7-smoke|vm-model40-byob-s20-smoke|core-machine-checked-memory-smoke" --output-on-failure`
  passed: 4/4.
- The owned route test emitted `M5:T395:S1:ROUTE-REGISTRY-SCALABILITY:OK`.
- Full `current-gates-gcc` remains required before acceptance.

No asset, ROM, firmware, media, external source, board timing, physical clock,
or L3 claim is made by this S.