# M2 T1 S3 Parity-First Headers Verification

## Evidence Reviewed

- The owner decided that M3 should not introduce a top-level `include/` tree
  before a stable SDK or packaging need exists.
- The current source tree already keeps C and H files together in the imported
  NXVM baseline.
- M3 T1 S1 needs a real linkable contract skeleton, not header-only paper
  interfaces.

## Result

The M2 architecture and M3 breakdown now require module-local contract headers
beside their C implementation, beginning with `src/core/*.h` and
`src/core/*.c`. Private headers use `_impl.h` and remain module-local. A future
packaging task may introduce top-level `include/` mirrors after the ABI is
stable.

This documentation-only subtask produced no runnable executable and therefore
no `build/output` artifact.
