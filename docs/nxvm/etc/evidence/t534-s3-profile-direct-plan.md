# T534 S3 Direct Profile Plan

The former resolver stored a recursive parent graph, per-field owner strings
and a second effective-value copy.  Retained fixed boards now construct one
direct `vm_profile_contract_values` value, validate it against the board's
contract catalog, and copy it into the frozen plan snapshot.  Runtime
provenance remains in the Profile evidence and source comments, not guest
construction state.

The former PC/AT FDC/HDC materializer was a callback from Machine into
Machine-owned device code.  It now lives beside the selected Profile plan and
uses only frozen Profile descriptor data plus neutral Core-plan operations.
Machine retains media object lifetime, registry binding, reset and finalization;
it no longer decides board controller wiring.

Similar-issue sweep:

- `rg` over `src/core`, `test/core` and `CMakeLists.txt` finds no resolver,
  parent/field-owner provenance or generic Machine materializer production
  symbol.
- XT, default PC/AT, IBM 5170 and Model 40 tests now inspect direct effective
  values/topology rather than a test-only inheritance mirror.

Verification: focused direct-plan tests and the complete repository-only unit
suite pass, 333/333 in 26.99 seconds (`ctest --label-regex ^unit$ --parallel 4`).
The retained external integration suite also passes, 20/20 in 23.73 seconds.
`git diff --check` passes.
