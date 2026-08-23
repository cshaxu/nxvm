# Td S128 Fourth-Pass Architecture And Coding Audit

Status: closed audit evidence. This record supplements the closed
[S125](td-s125-manual-architecture-coding-audit.md),
[S126](td-s126-second-pass-audit.md), and
[S127](td-s127-third-pass-audit.md) audits. The active contract is
[CURRENT.md](../../states/CURRENT.md); rules and design authorities remain
linked from [docs/README.md](../../README.md).

## Review Scope

This pass traces VM composition, profile, product, and public contracts through
their implementations and representative consumers. It records only manually
confirmed owner, state, interface, and source-design violations, plus bounded
non-findings. It does not yet make a whole-repository conformance or
runtime-correctness claim.

## Reviewed Surface So Far

| Area | Manual evidence | Disposition |
| --- | --- | --- |
| VM product console/catalog public contract | `src/vm/product/console.h`, `session_catalog.h`, `console.c`, `session_catalog.c`, `vm/main.c`, and product consumers | The public console context carries mutable parser/provider/session-manager/catalog state, while the public catalog exposes all mutable parsed record backing storage. Entry and tests instantiate the layouts directly. Confirmed finding A: VM product must expose lifecycle operations and copied observations, not owner state. |
| Model-40 construction ownership | `src/vm/composition/session/session.c`, `session.h`, Model-40 tests, and BYOB construction callers | `vm_session_create_model40_private()` is declared in the cross-module session header and only called from tests. It duplicates the large Model-40 Core configuration literal of the production BYOB constructor before both use the same lifecycle. Confirmed finding B: a test-only construction contract and duplicated configuration owner. |
| VM product/composition dependency direction | `src/vm/composition/session/provider.h`, `console_machine_adapter.c`, `vm/main.c`, and product callers | Composition includes the outer VM-product console provider contract and implements its adapter against the session manager. Product composition should assemble the two owners instead. Confirmed finding C: reverse product dependency and misplaced adapter authority. |
| VM lifecycle ownership | `composition/session/lifecycle.c`, `provider_lifecycle.c`, `session.c`, and representative create/reset/finalize callers | Start, reset, stop, resume, and finalization are centrally sequenced by the lifecycle owner; the separate provider lifecycle has a real VM-device/profile-firmware ordering responsibility. No additional duplicate lifecycle authority found in this reviewed surface. |
| Profile dependency direction | include sweep across `src/vm/profile`, composition, and product | Profile code does not include composition or product headers. Firmware context/BIOS concrete-state exposure remains a confirmed instance of the already transferred cross-module contract boundary rather than a second, distinct dependency cycle. |

## Confirmed Transfers

| ID | Finding | TODO transfer |
| --- | --- | --- |
| A | VM product console/catalog mutable-state escape | Cross-module VM contract boundary normalization (scope expanded). |
| B | Model-40 test constructor and duplicated configuration path | VM profile implementation-boundary repair (scope expanded). |
| C | VM product/composition reverse adapter dependency | Cross-module VM contract boundary normalization (scope expanded). |

## Review Limit

This pass disposes only the selected VM-product console/catalog, Model-40
test-construction, product/composition adapter, lifecycle, and profile-direction
surfaces. It has not disposed of all composition, profile, other product,
public contract, document, build-path, or runtime behavior and does not make a
whole-repository conformance or runtime-correctness claim.
