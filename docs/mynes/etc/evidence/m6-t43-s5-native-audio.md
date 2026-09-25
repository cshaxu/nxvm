# M6 T43 S5 Native Audio Diagnosis

Baseline: `f67eaefed`. The owner admits repair of S4's shared native-audio
failure as the next S; the investigation covers both receiving products.

## Observed Boundary

- Reproduced `library.audio_native` on x64 before any change: failure at the
  non-silent loopback assertion, about 0.72 seconds.
- Temporary capture diagnostics observed 42 packets, 39 marked silent; no
  nonzero sample was found. Mix format is extensible, stereo, 48000 Hz, 32 bits.
  These diagnostics were removed; the native test is replaced below.
- A read-only standalone endpoint probe returned `S_OK`, `mute=1`, volume
  `0.200000` for the default render/eConsole endpoint. It did not change volume
  or mute. Both production and capture select that default role.
- A muted host endpoint is a concrete environmental prerequisite to examine,
  not yet proof that every production audio path is correct. A controlled
  unmuted comparison was not performed; no production defect is claimed.

Microsoft's [GetMute contract](https://learn.microsoft.com/en-us/windows/win32/api/endpointvolume/nf-endpointvolume-iaudioendpointvolume-getmute)
defines true as the endpoint stream being muted. Its
[endpoint-volume interface guidance](https://learn.microsoft.com/en-us/windows/win32/api/endpointvolume/nn-endpointvolume-iaudioendpointvolume)
warns that changing master settings affects other applications. Consequently
the agent requested explicit permission to unmute temporarily and restore it;
no host-setting change is authorized or performed yet.

## Owner Decision And Correction

The owner rejected physical-signal dependence and an optional loopback mode.
Removed the physical loopback test, including successful early returns when no
endpoint exists. No opt-in switch remains. The replacement includes the real
Win32 adapter and substitutes only OS calls in its test translation unit.
Existing stream-worker tests remain unchanged. Production audio, public APIs,
Types declarations and App implementations are unchanged.

Coverage: mono/stereo at 22050/44100/48000 Hz, PCM format, exact sample bytes and
ordering across 4+4+1-frame batches, repeat/clear, render/cancel wait ordering,
cancel/failure outcomes, three submission failures and retry, all 13 initialization
OS-call failures, COM/interface/event cleanup. A temporary mutation replacing
PCM copy with zero-fill failed the exact-byte assertion as expected. This proves
corruption detection, not physical audibility or native-driver conformance.

Architecture/coding review kept the test at its native adapter boundary: no
production workaround, parallel audio implementation or public test API. The
similar-issue sweep includes existing worker queue tests, adapter reuse,
wait/cancel, submission and cleanup. Capture-only machinery is retired.

## Verification And Artifacts

- Complete Release builds of `build/mynes-gcc-x64-release` and
  `build/mynes-gcc-x86-release`, `cmake --build <tree> --parallel 8`: pass.
- Complete configured CTest suites, `ctest --test-dir <tree> --output-on-failure
  -j 1`: x64 130/130 (200.82 s), x86 130/130 (206.78 s). Each includes
  77 Shared and 53 MyNES cases; audio registration is replaced one-for-one.
- `cmake --build build/t41-s8-nxvm-x64 --target run-unit-tests --parallel 8`
  and corresponding x86 tree: 335/335 each (21.43 s / 23.02 s test time).
- Six manifests, Shared test Types boundary and whitespace checks pass.
- Documentation governance is checked before delivery and closure.

These are repository-only/regression results, not external-media T acceptance.
Production roots retain S4 imported bytes; `test/lib` intentionally differs
from SoftPC by this approved correction. No sibling repository was changed.

Both incremental Release builds evaluated the 0043 developer targets. Unchanged
production inputs retain the existing stripped MyNES 0.0.0043 pair from S4:

- x64, PE8664, 244750 bytes, SHA-256
  `F38E62A6D1F725003DD9460E4AA42879BE53EE42A794B39C19B5FBA5349F9BE7`.
- x86, PE014C, 237070 bytes, SHA-256
  `0A8B7B60E8E9231002440EFAA507D42272D60E4A69A025201BE9EB8819F4C744`.

No artificial binary delta is needed for this test-only correction. Temporary
endpoint probe, mutation executable and import scratch are removed after results
are recorded. Reusable build trees are retained for open T43 consumer regressions.

## Actual-Change Review

Review covers deleted loopback, complete replacement test, registration,
manifest and documents. Source/test count uses Git numstat, excluding docs and
manifest: 217 added, 209 removed, net +8 across three paths including CMake.
The small increase buys deterministic failure and cleanup coverage. One production adapter
remains; there is no optional physical-test path. Shared and MyNES deliveries
use separate target commits, followed by coordinator governance acceptance.

Shared implementation: `b7cbb30a9`, pushed to origin/master. The documentation
delivery records the complete verification above before coordinator acceptance.
