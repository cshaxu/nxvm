# T482 S4 Closure Audit

`M5:T482:S4:PROFILE-RECONCILIATION:OK`

`M5:T482:S4:CLOSURE-AUDIT:OK`

## One Route And Ownership

`session_catalog.c` is the sole YAML grammar owner. It accepts only root
`schema: nxvm-session`, `profile`, optional declared choices, `display`,
`boot`, `media`, and the constrained BYOB resource form. It rejects legacy
v1/nested identity, unknown fields, duplicate fields/blocks, incomplete media,
and malformed firmware before an entry is listed.

Console copies one catalog request and passes it unchanged to the Core product
`SESSION OPEN` command authority. The command authority performs open/select
and messages; it does not rebuild CLI strings. The manager treats the request
as opaque and the VM provider consumes it synchronously, copying retained
dynamic inputs into the new session. The resolved Core plan remains the one
machine-configuration authority.

## Profile Reconciliation

| Profile | Permitted YAML session choices | Rejected as machine authoring |
| --- | --- | --- |
| `default-pc-at` | declared CPU/FPU, memory, display, boot and media | unknown CPU/FPU/profile and any Model-40 firmware fields |
| `ibm-5170-model-339` | display, boot and floppy media | CPU/FPU/memory overrides and fixed-disk media |
| `compaq-deskpro-386-model-40` | constrained two-chip BYOB manifest, display and ROM boot | CPU/FPU/memory overrides and non-ROM boot |

The production/test sweep for `--profile`, `--cpu`, `--fpu`, `--memory-kib`,
and Model-40 CLI fields finds no production VM bridge. The sole `--profile`
hit is the retained negative test proving legacy arguments are rejected.

## Verification And Artifact

- Focused parser, provider, typed Console route, Model-339 Console and Model-40
  Console regressions pass.
- The full registered `current-gate` CTest selection passes **296/296** with
  four-way parallel execution; the specialized gate build passes, including
  T447 product-session authority.
- Documentation governance and `git diff --check` pass.
- The current optimized stripped Release artifact is
  `build/output/nxvm_0_5_0480.exe`, SHA-256
  `30C75B4463B084A0EEF2CCC33B8FF35B0AD5230EE95F8E5B314EABF1ADF7F779`.

No T482 residual transfers: the next queued work begins the independent IBM
5160 capability audit.
