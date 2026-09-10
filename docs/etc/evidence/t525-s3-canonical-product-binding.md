# M5 T525 S3 Canonical Product Binding

## Retained Route

NXVM now has one product-owned coordinator over the byte-identical canonical
SoftPC library corpus:

- the product creates one logical monitor Console and the sole process Console
  broker;
- a Console session borrows that broker through the coordinator's bounded
  claim/release contract, while every Window remains session-local;
- Core frames are copied into the canonical public `ux_frame` value, and
  native input is converted at the coordinator before NXVM's existing guest
  ingress;
- a caller without a product Console binding is deliberately headless.  It is
  a valid composition/integration execution, not a failed Console startup;
- source retirement releases only keys owned by that retired NXVM source
  identity.  No host identity crosses into Core.

The retired unified presenter, `ux_run` route, old UX-only contracts and their
tests are deleted.  No local `src/lib` file changed in S3.

## Similar-Issue Sweep

The CMake source lists, all VM platform/presentation/input/mouse paths, public
VM headers and UX tests were searched for the retired unified APIs and direct
product-native operations.  Product code uses the canonical Console broker and
public Window/Console APIs only.  The remaining platform-specific mechanics
are inside the canonical library's platform-private implementations.

## Verification

- Canonical corpus: `cmake -DLIBRARY_ROOT:PATH=src/lib -P
  src/lib/verify_manifest.cmake` passed.
- Full repository-only unit: `ctest --test-dir build/t525-s3-x64 -L unit -j
  8 --output-on-failure` passed, **306/306**.
- Full external-asset integration: `ctest --test-dir build/t525-s3-x64 -L
  integration -j 4 --output-on-failure` passed, **44/44** in 151.17 seconds.
  This includes the multi-Window proof, Model 40, XT, IBM 5170 and the
  CPU/FDD YAML boot matrix.
- Static: Linux platform contract, Linux adapter hygiene, session boundary,
  platform lifecycle boundary and build ownership all passed; `git diff
  --check` passed.
- Release artifacts: the target rebuilt and architecture-checked both
  `nxvm_0_5_0524_x64.exe` and `nxvm_0_5_0524_x86.exe`.
  Their SHA-256 values are respectively
  `470E9C34939385C2FA04BFF6CD4A531BF5A15087476BC5C406CC47D5025B02FC` and
  `6F40C9C218180F938B31F90A9646E72B800CD04EF2D790D02EBE6A73D274312D`.

## Review Note

The first integration pass exposed two lifecycle defects in the cutover:
Window teardown cleared the display fact before its executor was joined, and
headless composition sessions were rejected for lacking a product Console
claim.  The coordinator now records the pre-stop presentation fact for the
single teardown decision and treats absent Console ownership as headless.  The
focused failures and the complete matrix both pass after that shared-owner
repair.
