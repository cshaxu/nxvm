# T404 S3: Fixed And Removable Media Lifecycle

`M5:T404:S3:MEDIA-LIFECYCLE:OK`

## Batch B media disposition

| Public path | Owner | Before | After |
| --- | --- | --- | --- |
| Generic fixed HDD from `vm_session_config` | VM session composition | Construction called the public insertion operation. | Private startup-only helper loads configured fixed media before publication. |
| Public `vm_session_insert_hdd` | VM session composition | A published default PC/AT session could replace fixed media. | Always unavailable after publication; Model 339 and Model 40 were already unavailable. |
| Public `vm_session_insert_fdd` | VM session composition | Console checked running state, but direct callers could bypass it. Failed insertion copied the prospective path first. | Owner rejects running sessions and stages the path locally until insertion succeeds. |
| Model-40 fixed HDD | Model-40 composition | Existing private startup-only geometry-checked path. | Retained unchanged. |
| Controller, display and input semantics | Core plus VM profiles | Existing focused proof only. | Retained for the remaining Batch B reconciliation; no register, IRQ/DRQ, physical timing or L3 claim. |

## Similar-issue sweep

The search covered every `vm_session_insert_fdd`, `vm_session_insert_hdd`,
Model-40 startup HDD and Console FDD command caller. The Console's precheck is
now defense in depth rather than the sole lifecycle rule. Direct machine-level
media helpers are internal/profile mechanics and are not public session
operations; their controller behavior remains Batch B scope.

## Verification

- `current.vm-session-media-lifecycle-s3-smoke`: pass,
  `M5:T404:S3:MEDIA-LIFECYCLE:OK`.
- Full current gate: 286/286 pass.
- Documentation governance: pass before acceptance.