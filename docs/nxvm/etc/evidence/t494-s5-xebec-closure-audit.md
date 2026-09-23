# T494 S5 IBM 5160 Xebec Closure Audit

`M5:T494:S5:XEBEC-CLOSURE:OK`

T494 closes the finite X1--X18 IBM 6139790 Xebec universe frozen in
[List 1](t494-s2-xebec-function-timing-list-1.md) and mapped in
[List 2](t494-s3-xebec-current-code-gap-list-2.md).

| Ledger batch | Final disposition |
| --- | --- |
| X1--X6 | One Core `320h`--`323h` port, DCB, command-phase, status and sense-byte owner retains the manual-defined logical protocol. |
| X7--X10 | IBM leaves reset/status and physical-media timing values outside this logical controller model. They remain explicit L1/non-estimated boundaries, not implementation gaps. |
| X11--X15 | Existing Core DMA3, CHS and media helpers remain the sole transfer route. S4 corrected Read Data to consume each positive DCB block through that same route; Write already had the matching progression. |
| X16--X18 | ATA isolation, controller-selection and immutable construction remain explicit: Xebec is neither an ATA alias nor a second media/cache owner. |

The S5 owner/tag/port/DMA/geometry sweep found no second Xebec controller,
port route, CHS state or media cache. The only open List-2 row, R1, was closed
by S4; no X1--X18 row remains unclassified or deferred.

## Gate corrections found during closure

- The `640x200` CGA smoke still expected a palette-selected red foreground.
  T493 had already established the correct binary black/white high-resolution
  rule. The test oracle now expects white; no VADP behavior changed.
- The T345 deferred-owner verifier carried stale expected counts (185/182)
  while its generated, exact inventory already contained 186/183 rows. The
  expected counts now match that unchanged inventory; no entry was removed.
- The public machine interface included private `pit.h` only to name the PIT
  personality enum. The enum now lives in the public controller configuration
  contract, deleting that private-header leak without changing state or ABI.
- The current-gate separation verifier now reads CMake's explicit target
  dependencies, including CMake 4.3's bracketed CTest grammar, and rejects an
  empty selection. It no longer relies on a Ninja-only graph query or lets a
  generator mismatch produce a false zero-test pass.

## Verification and artifact

- Focused Xebec two-sector Read, DMA terminal/error and ATA-isolation CTest:
  passed.
- Full serial current smoke gate: 300/300 passed.
- Current specialized gates passed, including the repaired T345 count check,
  public interface boundary and current-gate separation (`full=298`,
  `media=15`, `non-media=283`).
- Documentation governance and `git diff --check` passed.
- Release target `vm-0-5-0494` built through the configured optimized Release
  route. The copied stripped artifact is `build/output/nxvm_0_5_0494.exe`,
  1,238,877 bytes, SHA-256
  `C7BDFD1CE67917373DCD762E093D8A5FF86798DC4E59DEF162B5682EB6B0686A`.
  `objdump -h` found no `.debug` section. The product runtime debugger remains
  included; only compiler debug information is absent.
