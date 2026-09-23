# T476 S4 IBM 5170 Root Closure Audit

`M5:T476:S4:IBM5170-ROOT-CLOSURE:OK`

## Proposal Exit Mapping

| Required outcome | Closure evidence |
| --- | --- |
| Freeze the selected root and its finite parity universe. | [S1 ledger](t476-s1-ibm5170-root-ledger.md) freezes configuration, provenance and parity. |
| Express the selected machine as immutable `pc-at-5170` root data. | [S2 resolver evidence](t476-s2-ibm5170-root-resolver.md) records copied root values, ports, routes, firmware services and Core materialization. |
| Use one root/session route without a second Core materialization. | [S3 cutover evidence](t476-s3-ibm5170-root-cutover.md) and the session sweep prove sessions own the copied root and no session calls the legacy descriptor getter. |
| Preserve selected behavior. | The frozen composition, clock, CGA, firmware/FDC and root-resolver smokes pass through the current-gate build. |
| Remove one source of duplicate construction/test complexity. | The obsolete `core-machine-exception-delivery-s2-smoke` aggregate and registration are deleted. Its four real owner smokes remain separately registered, compiled and tested. |

## Final Verification

`cmake --build --preset current-gates-gcc` passed with 294 registered current
targets. It includes documentation governance, mutable-state inventory, T345
ownership verification and the retained independent exception owner smokes.
The source sweep finds no session construction reference to
`vm_profile_ibm_5170_model_339_descriptor_get`; the retained static descriptor
is a root-declaration source only and is not a session or Core state owner.

The stripped Release artifact was rebuilt from source commit `b8a98502`:

| Field | Value |
| --- | --- |
| Target | `vm-0-5-0476` |
| Artifact | `build/output/nxvm_0_5_0476.exe` |
| Banner | `Neko's x86 Virtual Machine [0.5.0476]` |
| Size | 1,206,544 bytes |
| SHA-256 | `F3A5004E6C8932AAB2462B6000B2B8F0BA1F94ABE3CB30E2A3EE68A6487F052A` |
| Debug sections | None reported by `objdump -h` filter. |

No runtime Core, device, timing, YAML or profile-catalog behavior changed in
S4. The formatting-only static-function declaration adjustment keeps the
readiness scan from mistaking a function signature for mutable file-static
state.
