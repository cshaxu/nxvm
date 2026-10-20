# T4 S33 — Black-Box Graphics Oracle Disposition

## Method

An official MAME 0.289 Windows x64 release was acquired outside this repository
as a transient black-box tool. The release package hash was:

```text
A1AA7912168C9D1B05E611906BC21B8B9BE3935822AEAD36D12A1DA363150B7D
```

The tool listed `raiden2` as `Raiden II (US, set 1)`. It was not used as a
source dependency, no source/table/handler was inspected or copied, and the
owner-supplied archive remained read-only.

## Result

`-verifyroms raiden2 -rompath O:\assets\raidenii` rejected the supplied archive
before any emulation. It reports two absent PAL images:

- `jj4b02__ami18cv8-15.u0342`, 341 bytes;
- `jj4b01__mmipal16l8bcn.u0341`, 279 bytes.

No guest execution, screenshot, trace, decoded graphics data, or derived
layout was retained. The temporary external emulator package was removed after
recording the release hash and neutral result.

## Disposition

The available archive cannot currently supply the proposed independent
black-box behavior oracle. That does not prove the existing Core model wrong;
it only means S33 cannot lawfully derive a graphics bitplane/decryption/layout
contract from this input. A later task needs either an owner-supplied complete,
lawfully usable reference set or independent primary board evidence. Until
then, no decoder, renderer, palette-layout claim or game-readiness claim is
admitted.
