# T345 S3 Production Direct-Compilation Ownership

## Method

S3 consumes every non-owner-test row from the fixed 175-row T345 ledger,
using the target's complete direct C source set and the actual GCC warning
audit.  A linked library is not evidence for a target's direct compile
command.  The three owner-test rows remaining in mixed targets are retained
with their compiled production sources; S2 already proved their own direct
sources warning-clean.

## Strict Promotion

These targets each compile exactly one direct source, have a zero-warning
baseline, and have a contained target-local ownership boundary.  S3 applies
the four GCC options only to these direct targets.

| Target | Direct source | Ownership decision | Warning baseline | Actual coverage |
| --- | --- | --- | --- | --- |
| `type-facade` | `src/type.c` | Project type foundation; one-source target. | 0 | S3 target-local strict command. |
| `core-product-utils` | `src/core/product/utils.c` | One-source core-product facade; linked debug/xasm code remains independently deferred. | 0 | S3 target-local strict command for `utils.c` only. |
| `vm-platform-requests` | `src/vm/platform/request_bridge.c` | One-source request-bridge target; its dormant non-current smoke drift remains separately deferred. | 0 | S3 target-local strict command for `request_bridge.c` only. |

The CMake inventory rejects a change to any of these one-source target
surfaces.  The existing T344 actual-Ninja verifier proves the four options on
the direct commands; it does not claim strictness for dependencies.

## Retained Direct Rows

Every row below stays in the T345 generated ownership matrix with exactly one
future admission trigger.  Warning counts are from the fresh 175-row audit
after S2; zero does not override a mixed or inherited ownership boundary.

| Target and direct source set | Ownership/risk | Warnings | Single admission trigger |
| --- | --- | ---: | --- |
| `core-machine-executor`: `display.c`, `vadp.c`, `port.c`, `memory.c`, `cpu.c`, `fpu.c`, `cpu_instructions.c`, `pic.c`, `pit.c`, `dma.c`, `kbc.c`, `rtc.c` | Mixed core machine target, including inherited CPU executor construction; direct strictness would require all units and must not be inferred from linked tests. | 66 | A bounded CPU-executor or safely separated machine-target task with caller, fault, and behavior sweep. |
| `core-product-debug`: `debug_access.c`, `debug.c`, `xasm32/aasm32.c`, `xasm32/dasm32.c` | Debug target embeds retained xasm implementation; its warning and capacity/failure semantics are shared. | 43 | The existing xasm/debug capacity-and-failure admission modifies the shared implementation with all public callers. |
| `core-product-xasm-contract-smoke`: `utils.c`, `xasm32/aasm32.c`, `xasm32/dasm32.c` | Test target directly compiles the same retained xasm implementation under test-specific allocation seams. | 39 | The same xasm/debug admission separates or repairs the shared sources; do not strictify the test target alone. |
| `core-product-debug-input-failure-smoke`: `debug.c`, `debug_access.c` | Test-specific allocation seam directly compiles the debug owner. | 4 | The debug-input owner admission repairs/separates the production source with its failure contract. |
| `vm-product-console-input-failure-smoke`: `console.c` | Test-specific allocation seam directly compiles retained Console behavior. | 24 | A Console input/failure owner admission with runtime and interactive behavior sweep. |
| `vm-product`: `console.c` | Retained NXVM Console product source; its warning class is behavior-coupled. | 24 | The same Console owner admission, not a global warning cleanup. |
| `vm-platform`: `platform.c`, `host_surface.c`, `execution.c`, `input_flush.c`, `win32/win32.c`, `win32/w32cdisp.c`, `win32/w32adisp.c`, `win32/win32con.c`, `win32/win32app.c` | Mixed portable and inherited Win32 platform target; Windows host behavior and one direct warning remain coupled. | 1 | A Windows-platform ownership task or a safe target split with host behavior evidence. |
| `vm-machine`: `fdd.c`, `hdd.c`, `media_save.c`, `debug.c` | Mixed retained device implementation and newer media persistence; clean commands do not make the aggregate source-owned. | 0 | A device-target separation or substantive inherited-device change with media regression. |
| `vm-composition`: `session_factory.c`, `console_machine_adapter.c`, `debug_target.c`, `media.c`, `display.c`, `lifecycle.c`, `session.c`, `control.c`, `fault.c`, `execution.c`, `runner.c`, `provider_lifecycle.c`, `machine_devices.c`, `profile_firmware.c`, `machine_info.c` | M5 composition target; `session.c` has a dead helper and `profile_firmware.c` materializes five C string literals above the C11 guaranteed translation limit. Replacing those literals needs a bounded firmware/xasm materialization contract, not an accidental formatting workaround. | 6 | A firmware-materialization admission that defines chunking/capacity and preserves generated-ROM bytes before strictifying the whole target. |

The seven retained target groups account for all 51 non-promoted production or
embedded-production direct rows.  The three retained owner-test rows are the
test sources of the three embedded targets above, completing the original
57-row post-S2 deferred set.  The T345 verifier retains all 175 original keys:
121 owner-test, six embedded-production-test, one type-foundation, two safely
separable production, and 45 mixed/inherited production rows.

## No Implicit Runtime Repair

S3 makes no source or ABI repair.  In particular, the `profile_firmware.c`
overlength diagnostics reveal a string-materialization mechanism shared with
the constrained xasm boundary.  Splitting adjacent C literals would preserve
the same overlength translation unit string; adding an unbounded runtime
concatenation would violate the known capacity contract.  It is retained with
the exact future admission above rather than suppressed.
