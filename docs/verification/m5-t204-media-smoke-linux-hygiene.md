# M5 T204: Media Smoke And Linux Adapter Hygiene

## Contract

The five FDD-only smoke targets and one FDD+HDD smoke target are defined once.
The generic registration skip list and argument-bearing registration both derive
from those declarations. Removed platform sleep facades had no callers; Linux
Console color mapping remains identical for its masked three-bit inputs.

## Evidence

- Current CTest registration remains 42 tests; its media subset is five
  FDD-only probes and `vm-product-session-smoke` with FDD and HDD.
- `current-gates-gcc`: 42/42 CTest smoke and 19/19 static/structure gates.
  Markers: `M5:T204:S1:CURRENT-MEDIA-SMOKE-CLASSIFICATION:OK` and
  `M5:T204:S1:LINUX-ADAPTER-HYGIENE:OK`.
- Linux native compilation is deferred to a POSIX host; source/CMake static
  parity passed on Windows.
- Artifact: `build/output/nxvm_0_5_0204.exe`; Console `EXIT` status 0;
  SHA-256 `3C476F0700C35A91D7A5E35B5ECB8006C5DF4730DC611BCE86CBD34488C1BBD5`.
