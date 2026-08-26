# T475 S3 Integration Closure Audit

`M5:T475:S3:INTEGRATION-CLOSURE:OK`

| Batch | Closure proof |
| --- | --- |
| S1 construction ledger | [S1](t475-s1-profile-resolver-ledger.md) freezes all three current profile routes and excludes every mutable Core/session/host resource from resolver data. |
| S2 resolver mechanism | [S2](t475-s2-profile-resolver.md) proves one typed-parent resolver, copied field provenance, copied Core-plan input, existing-plan handoff and the complete accepted/rejected fixture matrix. |
| One production path transition | The final source sweep finds only the two retained current plan builders: AT session composition and private Model-40 composition. Resolver calls occur only in its focused smoke until the explicit 5170, DeskPro and default-AT connection tasks consume it. No parallel migrated path exists. |

## Final Verification

- `cmake --build --preset current-gates-gcc` passes all 294 registered current
  targets, including the resolver smoke, updated exact T345 owner-test
  inventory, specialized gates and documentation governance.
- `cmake --preset mingw-gcc-x64-release` plus `cmake --build --preset
  current-gcc` produced stripped Release
  `build/output/nxvm_0_5_0475.exe`, banner `0.5.0475`, 1,198,682 bytes,
  SHA-256 `26E00D29D41EF4A69E234D31A6FD54D886D879B8DD2402F45C59FA2E55EE1281`.
- Direct launch reached the retained Console banner and profile prompt. The
  focused resolver smoke prints both S2 markers.
- Documentation governance, `git diff --check`, artifact identity and source
  sweeps pass. The resolver has no current production caller and the only
  `core_machine_plan_create` / `core_machine_create_from_plan` routes remain
  the two named legacy builders pending their individually approved migrations.

## Simplicity And Transfer

The code change is +289 production lines (`profile_resolver.c` and its one
cross-module value header), +152 focused-test lines and zero removed code;
documentation/build files are excluded from that count. The positive code is
one bounded owner mechanism, not a wrapper: it replaces no current production
route because S1 freezes those routes for later parity migrations. It has one
responsibility: resolve immutable VM values before Core construction, and one
focused test target. No state, scheduler, setter, callback, Core pointer,
machine-name-in-Core branch or second Core-plan API was added.

The remaining direct production routes transfer once, in queue order, to the
IBM 5170 root, DeskPro 386 child and default-AT child tasks. YAML remains a
later session-selection consumer. T475 has no remaining in-scope gap.
