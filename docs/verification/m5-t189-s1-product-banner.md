# M5 T189 S1: Product Banner Verification

## Result

`vm/product/version.h` defines `PRODUCT_NAME`. The shared
`core/product/banner.h` defines the common version, copyright, build-time, and
fixed banner print macro. The runtime version source, formatter, and smoke
target are deleted.

## Verification

- MinGW-w64 GCC configure passed.
- `nxvm-0-5-0189` built successfully.
- `nxvm-product-console-lifecycle-gate` emitted
  `M5:T96:S1:CONSOLE-LIFECYCLE:OK`.
- Piping `exit` to the artifact emitted the required title, copyright,
  `Built on` line, and retained `Console>` prompt, then exited with status 0:
  `M5:T189:S1:BANNER:OK`.

## Artifact

- Path: `build/output/nxvm_0_5_0189.exe`
- SHA-256: `78C1B642F456D1599BA518308D486530597F67173AE9CE953AB7BF9AAAC22312`
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0189]`
- Classification: local developer artifact, not a release artifact.
