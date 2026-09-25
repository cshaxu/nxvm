# M6 T43 S6 Receiving Artifact Completion

Coordinated by the MyNES T43 packet with owner-approved Shared/NXVM/MyNES
targets. This does not reopen closed NXVM T41 or allocate another numeric task.

## Need And Boundary

S5 changes only tests and needs no new executable. S4 `83022ea9f`, however,
changed production frame/console/UI inputs after the deployed NXVM 0535
baseline. Unit executables built against those inputs did not update the
EXCLUDE_FROM_ALL product targets. S6 explicitly builds all four receiving
profiles on both architectures. MyNES 0043 already includes S4 and its
incremental product build correctly reports no work. No timestamp-only rebuild
or new MyNES binary commit is needed.

Production source revision: `b34152e15` (unchanged throughout S6). Existing
version identities 0.5.0535 and 0.0.0043 remain; this is receiving artifact
refresh, not a new product implementation/version. No source, tests, public
ABI, INI content, protected payload or sibling repository changes.

## Build And Validation

For each x64/x86 tree `build/t41-s8-nxvm-<arch>`, configure the existing
Release/BYOB toolchain with `-DNXVM_PRODUCT_PROFILE=<profile>`, then run
`cmake --build <tree> --target vm-0-5-0535 --parallel 8` for XT, AT, Model40
and default. Restore the default selection before unit regression. CMake's
existing deployment owner validates PE architecture and copies each executable
to `assets/nxvm/<profile>/`; adjacent user INI contents remain unchanged.

All eight builds pass. PE machine IDs are 8664 (x64) and 014C (x86).
`objdump -h` finds no debug sections; each binary contains 0.5.0535 identity.
These are optimized developer artifacts, not new external-guest qualification.

Complete NXVM units pass 335/335 on x64 (23.69 seconds) and x86 (22.77 seconds),
using `cmake --build <tree> --target run-unit-tests --parallel 8`. Both product
documentation gates and whitespace checks pass. Shared/MyNES full-suite results
belong to the coordinating MyNES S6 acceptance record. MyNES dual product
targets pass unchanged with the hashes recorded by S5.

## SHA-256

Paths below are relative to `assets/nxvm/`; all filenames carry 0.5.0535.

| Profile / architecture | Bytes | SHA-256 |
| --- | --- | --- |
| compaq-deskpro-386-model-40-1200k / x64 | 1243409 | 4B4B845E385423AB9E2C94A2795A5AABF40533DD2FC0A52AF360F3CE4C5094DC |
| compaq-deskpro-386-model-40-1200k / x86 | 1412472 | E5C5D25E7753DA07864B9786875C5AB92ACB20823AFBE5E9BD9C2D21423EC377 |
| default-pc-at-80386-1440k-hdd / x64 | 1242897 | A586C4E3F3C1F40A67B64A1D0B9A33A03DADDB7A3E74A92A8E478515BF6201E6 |
| default-pc-at-80386-1440k-hdd / x86 | 1411448 | 8813D63AB8E135386E5D7B2CD6B385390821D8E4A75347394EC803DCC62A02F2 |
| ibm-5160-model-268-360k / x64 | 1242897 | F51BD628A9D446D579B6B2164CDE34103D69663221A3505F32C4F352B2A28ABD |
| ibm-5160-model-268-360k / x86 | 1411448 | 968F226715FDBDD50823642EE8C50A035BAD7856A12E4780A8BF48A121C89B6B |
| ibm-5170-model-339-1200k / x64 | 1243409 | 529BD9E5A7C6A9AB6F6BCCE768E2FF7BD1C6ADCB8E68913C6C0BE7D96BCEF4C1 |
| ibm-5170-model-339-1200k / x86 | 1411960 | E7FA0914F3A754B910CE51D6CFD138B6EA0A0A3FF4C8A14B145201F97D642716 |

## Prevention And Cleanup

Execution now requires Shared deliveries to account for all receiving products
and runnable profiles, with explicit executable-input impact, current product
target builds and per-product pushed artifact commits. A green unit suite alone
cannot prove deployed products are current. Test/document-only work and verified
unchanged outputs need no binary churn. No source/test lines added or removed.
Temporary S6 logs are removed after this evidence is finalized; existing build
caches remain for open T43 verification.
