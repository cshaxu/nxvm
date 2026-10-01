# T539 S75: Opaque CPU lifetime

## Result

`core_machine` now owns exactly one opaque CPU-execution pointer. The CPU
implementation allocates and frees the enclosing execution, CPU and decoder
layout; the board no longer embeds or mirrors any of those fields.

- Creation supplies the existing CPU-bus provider and board context.
- Existing profile, FPU, diagnostic, prepared-entry, reset and run paths use
  that one context.
- Creation failure releases the FPU and board allocation; destruction releases
  the CPU owner once before remaining board teardown.
- `machine.h` imports only `cpu_interface.h`. The private CPU layout appears
  only in the test fixture needed to prepare legacy instruction inputs.

## Verification

- Complete repository-only units: x64 **426/426**, x86 **426/426**.
- x64/x86: `verify-core-cpu-pic-authority`,
  `verify-t332-cpu-fixture-lifecycle`,
  `verify-t344-historical-fixture-shapes`, and
  `core-machine-lifecycle-ownership-closure` passed.
- NXVM documentation governance passed.
- `git diff --check` passed.

## Rebuilt 0539 Release artifacts

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| Default PC/AT | `29c7609fe290719ac5d77135408e619c099cddcee772ccb2723e8d0f039fff2d` | `dd100b047d812e4d8b407e6e9fe3fe82044267b7a12d97e045dd2e9394436a13` |
| IBM 5160 XT | `18d49bf18f553447b3c1c54791b3299c0a08134b7ce80748a07730e32fa0e27a` | `b7a87df70e8131aa6f2c89c618ce197178f4b8831a222d6c2bb0de5a6ccc60d2` |
| IBM 5170 AT | `bb7356647ab77da53bfd5fb9d1327c0e9bd0eccb28ef7273c2901437713c138d` | `6a7dc20a7378dc44bde8e56d7fe0bce46ae2790ee320b39c8c2f8d9187d38f62` |
| Compaq Model 40 | `3c05bab1013204598ea8eb279034245caac6d2ad0cc2c33349ad68ac7d2b97fa` | `1d571bac2044e170791e763db47661a697e822dbf6b04b81bed284dacf729126` |

Each product target reported an optimized Release artifact with the expected
PE architecture. No MyNES source or artifact is part of this S; the one
accidentally rebuilt MyNES executable was restored to its committed bytes.
