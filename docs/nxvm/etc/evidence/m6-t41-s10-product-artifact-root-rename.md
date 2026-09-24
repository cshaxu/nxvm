# M6 T41 S10 Product Artifact-Root Rename

## Outcome

The versioned artifact roots are `assets/nxvm/` and `assets/mynes/`.  The
former `assets/binary-nxvm/` and `assets/binary-mynes/` directories are absent.
Their existing checked-in executables and adjacent INIs moved without a rename
of the executable, profile, or version contract.

## Included owner worktree items

- The edited MyNES INI and its `snapshot/` content move under `assets/mynes/`.
- The revised `.gitignore` moves its versioned-artifact exceptions to the new
  roots.
- The owner-deleted `.github/workflows/lib.yml` is included unchanged in
  intent.

## Guards

`cmake/nxvm/verify_product_artifact_roots.cmake` verifies both replacement
roots, rejects either retired root, and rejects retired path references in the
live code, tool, CMake, rule, and design authorities.  It deliberately does
not scan historical evidence or the active packet: those records describe the
former names to preserve the audit trail.
## Verification record

- `verify_product_artifact_roots.cmake`: passed; both replacement roots exist
  and both retired roots are absent.
- Documentation governance: passed for both NXVM and MyNES.
- MyNES: x64/x86 release product builds complete; the preserved dual-artifact
  files are present beneath `assets/mynes/`.  The existing full-suite logs end
  in passing results for each architecture.
- NXVM: x64 Release current target deployed and its current-artifact and
  product-artifact-root gates passed.  The existing four-profile x64/x86
  artifact pairs remain under `assets/nxvm/`.
- NXVM x86 fresh configure is not available in this shell: the preset's
  required `NXVM_I686_GCC` was unset, and the available GCC 16 replacement
  fails CMake's ABI-size probe before product configuration.  This delivery
  does not alter that toolchain contract or discard the checked-in x86 output.