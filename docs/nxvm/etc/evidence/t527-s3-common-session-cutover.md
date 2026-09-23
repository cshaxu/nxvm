# T527 S3 - Common Session Cutover

## Outcome

`common/session` is the one NXVM owner of the bounded copied fact FIFO, latest
frame slot, run identifier, lifecycle fact reduction and pressed-input hygiene.
The former `vm/session` owner and its presentation-plan ABI are deleted.

| Former owner | S3 owner | Disposition |
| --- | --- | --- |
| `vm/session/control` FIFO, latest display and run generation | `common/session` | Deleted and replaced. |
| `vm/session/session` lifecycle reducer and pressed state | `common/session` | Deleted and replaced. |
| machine construction/result binding | `vm/app` | One composition binding; it owns no FIFO, lifecycle, run id, retry or UI state. |
| YAML catalog | `vm/product/catalog` | Moved to its product policy owner. |
| Core-display to copied `ui_frame` conversion | `vm/app` via existing `vm/presentation/frame` | Retained once; no second presentation route. |
| product command grammar and NXVM window titles | `vm/product/console` | Retained product policy. |

## Deliberate sequencing

S3 does not export a CLI-provider callback because it would have no production
consumer until `common/debug` and the product CLI migrate in S5. The proposal
and active packet explicitly defer that live API, preventing an unused
framework surface. Machine-result producers presently receive the current run
identifier at the one `vm/app` ingress, as the prior control owner did; S4's
machine FIFO migration is the designated receiver for producer-origin run
identity.

## Verification

- `common-session-smoke`: lifecycle plan, target selection, source-retired key
  release, latest-frame delivery, stale run rejection and bounded FIFO overflow.
- Complete repository-only unit suite: 299 passed, 0 failed.
- `verify-dependency-dag`, `verify-product-session-manager`,
  `verify-session-readiness`, `verify-unit-separation` and
  `verify-t344-unit-registration`: passed.
- Documentation governance and `git diff --check`: passed.
