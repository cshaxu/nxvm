# T537 S3 independent-product matrix

Date: 2026-09-23.  Scope: post-consolidation references, product ownership,
and runnable x64/x86 verification.  No external firmware or media bytes were
imported.

## Repairs

- Live documentation links and shared checks now point only to the product
  documentation roots or `docs/rules/`; the owner-frozen root `README.md`
  remains byte-identical to `36925a4b`.
- The clean-build timing runner writes its generated decoder inventory beneath
  the build tree, not into retired documentation paths.
- The integration registration and CMOS smoke use the deployed
  `<profile>/NXVM.ini` identity, not retired flat `*.ini` names.
- Artifact deployment now copies INI text unchanged.  The removed rewrite had
  appended a `..` on each build, eventually changing a valid relative media
  path into `O:\nxvm-assets`.  Every retained NXVM INI resolves from its own
  directory to `O:\repos.hobby\nxvm-assets\media-nxvm`.

## Verification

- Both product documentation gates pass; the static App/Core/test/tool sweep
  reports `CROSS_PRODUCT_CODE_TEST_TOOL_REFS_OK`.
- MyNES x64 and x86 each build and run its complete 53-test selection.
- NXVM complete unit selection: 336/336 for default, XT, 5170 and Model 40,
  in both x64 and x86 configurations.
- External integration executes rather than skips: default 20/20, XT 1/1,
  5170 3/3 and Model 40 3/3, in both host architectures.  Model 40 reaches
  its installer-running terminal.
- PE inspection confirms all emitted x64 artifacts are `0x8664` and all x86
  artifacts are `0x014c` in their product-owned asset directories.

The Model 40 x86 build was recreated once after detecting that a temporary
configuration omitted `Release`; the final recorded matrix uses the i686
Release compiler configuration and passes its full suite.
