# T382 S8: Console Memory Round Trip

`M5:T382:S8:CONSOLE-MEMORY:OK`

Console no longer narrows `entry->memory_bytes >> 10` to `unsigned int` when
materializing `--memory-kib`. The checked catalog value is formatted as its
native `STD_SIZE_T` decimal representation.

The new current-gate smoke writes a valid YAML profile with
`memory_kib: 4294967296`, enters `SESSION OPEN`, and uses a normal manager
provider to assert that the provider receives exactly `--memory-kib
4294967296`. It deliberately records the transport boundary rather than trying
to allocate an implausibly large VM.

The S8 smoke and existing Console lifecycle smoke pass. The complete
single-worker current aggregate passes all 251 tests with zero failures.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57` |
