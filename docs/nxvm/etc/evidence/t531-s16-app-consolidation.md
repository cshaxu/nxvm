# M5 T531 S16 App Consolidation And Shared Corpus Refresh

## Result

SoftPC commit `00d4461d0754ae2a7bd8684aa06df6541f83fef9` is the frozen
project-owned source baseline. The four shared trees are byte-identical to
that committed corpus:

- `src/lib`;
- `src/common`;
- `test/lib`; and
- `test/common`.

The former NXVM `src/vm/product` root is retired. `src/vm/app` is now the one
NXVM App root, with catalogue/configuration parsing, command policy, recorder,
version identity and composition. The one `vm-app` target replaces the former
two-target route. There is no compatibility target or retained old include
path.

Configuration resolves a copied `vm_session_request` to a
`vm_machine_config`. Composition then calls the existing public
`vm_machine_create` operation. Thus App owns product spelling and validation;
VM-machine remains the sole concrete CPU/FPU/device construction owner.
Command policy no longer includes the runtime lifecycle implementation header;
its retained VM-machine calls are public product operations.

## Actual-Diff Review And Sweep

The review covered all renamed App source/tests, their CMake ownership and
test registrations, direct dependency exceptions, static gates, integration
YAML support and the affected architecture/source-map documents. The tracked
code, test, build and gate surface is 449 added and 463 removed lines, net
minus 14, excluding documentation. The positive rename churn is mechanical;
the only behavioral structure change is separating App request resolution from
VM-machine creation. The retired product target, source root and forwarding
edge were deleted.

The live CMake, `cmake/`, `tools/`, `src/` and `test/` sweep has zero
`vm/product`, `vm-product`, `vm_product`, `request_factory` or former
`vm/app/app` occurrences. The one `nxvm_product_console_target` string is a
negative pattern in `VerifyProductSessionManager.ps1`, intentionally checking
that this retired vocabulary is absent from source.

## Verification

| Check | Result |
| --- | --- |
| SoftPC four-tree direct comparison | all four equal to commit `00d4461d0754ae2a7bd8684aa06df6541f83fef9` |
| Focused App command/catalogue/recorder build | passed in x64 Release build |
| Repository-only unit suite | 325/325 passed serially; 65.76 seconds |
| App ownership, dependency, direct-compilation and documentation gates | passed |
| x64 stripped Release 0531 | PE x64; SHA-256 `30171273680290D8AD1C866665811FC6BBB6C13EE96C060281113D418EECED83` in both required locations |
| x86 stripped Release 0531 | PE x86; SHA-256 `4CE4D1D08A641FFE3A6870F11341D43F0401428D98CDF1D932FBC17164B530EB` in both required locations |

## Disposition

This implementation satisfies the S16 boundary. T531 remains open for the
owner's later Common product-convergence review and whole-task acceptance.
