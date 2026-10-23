# Source Layout And Coding

Apply shared [Coding Rules](../../rules/CODING.md). Product root spelling is
`myraidenii`; display/commit identity is `MyRaidenII`. Exact roots:

```text
src/app-myraidenii/{product,machine,core}/
test/app-myraidenii/{unit,setup,integration,diagnostic}/
assets/myraidenii/
docs/myraidenii/{design,states,proposals,history,etc}/
```

Create subdirectories only with real cohesive code, not empty future chips or
CMake stubs. The active source graph has one Board/Core component, one Machine
component and one Product component; their unit tests mirror those owners below
`unit/{core,machine,product}`. Core owns board, COP, native video and local
sound mechanisms; Machine owns image materialization and the Emulator driver;
Product owns configuration, identity, controls and the one composition route.
This records implementation ownership, not gameplay or release qualification.
Setup holds
only real shared integration/diagnostic harnesses; diagnostics are opt-in and
are never silently counted as integration. External game tests are integration,
not units. No tests under src and no fixture borrowing from another App.

New C uses Lib Types and C-runtime vocabulary, `lib_bool`/LIB_TRUE/LIB_FALSE,
LIB_NULL and existing lib_status errors. Deterministic Core has no platform,
filesystem, host clock or worker dependency. Public component headers end in
`_interface.h`; plain headers are private by default. Do not add `_internal`,
`_private` or compatibility aliases. Symbols identify their actual owner,
for example `myraidenii_machine_*` and `myraidenii_core_*`; game register
constants retain meaningful hardware names rather than task numbers.

`product/version_interface.h` follows MyNES's product-local pattern:
it owns `APP_PRODUCT_NAME`, `APP_PRODUCT_VERSION` and the two-line
`APP_PRODUCT_BANNER`. A numeric implementation T selects its four-digit
revision, not an S or compiler option. The banner is passed to Emulator
Product, which alone adds the shared Built on framing; the App does not format
that line itself.
Initial release format is `myraidenii_0_0_NNNN_{x64,x86}.exe`, alongside the
lowercase INI. M0 creates neither version header nor EXE. Only the latest
verified pair is retained/committed; no ROM bytes or generated embedded source.

M1 build registration uses a product option disabled for unrelated product-only
builds, one source list per owner and existing Ninja/ccache graphs. Libraries
are built once per architecture and reused by tests/App targets. Shared build
edits have Shared commits; product commits cannot mutate other products.
Tools are introduced under tools/myraidenii only when a real tool exists.
External research-tool configuration is not repository source: it belongs under
the owner-managed `nxvm-assets/media-myraidenii/` tree and commands must select
that location explicitly. No tool may create a repository-root `cfg/` (or
another product-unowned root directory).
