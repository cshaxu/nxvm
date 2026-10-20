# M1 T4 S35 — External PAL BYOB Connection

## Owner-Approved Exception

The owner explicitly approved acquiring these two PAL images as external BYOB
inputs.  They are deliberately outside the repository, untracked, excluded
from manifests and not embedded in an executable.  This record contains only
names, locations, sizes, checksums and tool outcomes; it contains no payload.

## External Layout And Validation

The source split-set archive was obtained transiently from:

```text
https://mdk.cab/download/split/raiden2.zip
```

Only the two listed files were extracted.  The temporary archive was deleted
after verification.

| External location | Size | SHA-1 |
| --- | ---: | --- |
| `O:\repos.hobby\nxvm-assets\media-myraidenii\raiden2\roms\raiden2\jj4b02__ami18cv8-15.u0342` | 341 | `8b46f6673ddf11efbc3394ae423ec89d4a1283bf` |
| `O:\repos.hobby\nxvm-assets\media-myraidenii\raiden2\roms\raiden2\jj4b01__mmipal16l8bcn.u0341` | 279 | `95ce9cfbfb280dfc6a326e378684eff3c6f54701` |

The `roms\raiden2` set subdirectory is intentional: current MAME resolves
loose set members relative to each `-rompath` root as `<root>\raiden2\name`.
It allows the immutable owner archive to remain in its existing asset root.

## Black-Box Connection

MAME `0.289 (mame0289)` was run with:

```text
mame.exe -verifyroms raiden2 \
  -rompath "O:\assets\raidenii;O:\repos.hobby\nxvm-assets\media-myraidenii\raiden2\roms" \
  -noreadconfig -nowriteconfig
```

It returned:

```text
romset raiden2 is good
1 romsets found, 1 were OK.
```

One bounded headless launch using the same two-path `-rompath` and
`-video none -sound none -nothrottle -skip_gameinfo -nowriteconfig
-seconds_to_run 10` terminated normally, reporting `Average speed: 583.40%
(9 seconds)`.

## Repository Connection Limit

MyRaidenII presently has only Core and ROM-free/Core integration probes; it
does not yet have its Machine/Product runtime loader or an executable.  The
PALs therefore connect the current MAME black-box reference only.  They do
not yet enter a MyRaidenII configuration, Core input bundle or product EXE.
Adding that later requires a separate Machine/Product asset-loading contract;
this S neither invents one nor presents the MAME verifier result as graphics
decoder or playable-game qualification.
