# T441 S1 VM Media Persistence Failure Hygiene

## Owner and reduced path

`src/vm/machine/media_save.c` is the sole VM persistence owner.  Its existing
`vm_machine_media_stage()` now serves both callers: the single-file HDD route
stages once and replaces once, while the FDD route stages both raw image and
sidecar before its existing paired backup/commit/rollback sequence.  This
removes the duplicated single-file staging implementation (43 lines removed
from that owner) without adding an interface, wrapper, filesystem layer, or
second save path.

The FDD caller owns its derived sidecar buffer.  It initializes that buffer to
null and releases it together with the sidecar name on every serialization or
paired-save failure.  A failed removal returns before disk state, generation,
image bytes, or address marks are changed.  HDD remains the only single-file
caller and returns its existing `vm_machine_media_save_atomically()` failure
without clearing resident media.

## Complete caller and failure sweep

The tracked production/test search was:

```text
rg -n "vm_machine_media_save_(atomically|pair_atomically)|vm_machine_fdd_sidecar_serialize" src tests --glob '!build/**'
```

| Hit | Disposition |
| --- | --- |
| `media_save.c` public save functions | Shared staging, temporary-file cleanup, replacement, and paired rollback owner. |
| `fdd.c` sidecar serializer and one paired caller | Corrected: derived allocation is released on every failed serialize/pair path before the unchanged resident FDD is returned. |
| `hdd.c` one single-file caller | Covered: it retains the established failure return before mutating connection state. |
| `vm_media_provider_smoke.c` direct single-file coverage | Covered: collision selection, invalid input, successful replacement and preserved target are retained. |

No other production caller, paired-save caller, or sidecar serializer exists.

The focused smoke exercises: invalid-path staging failure for FDD and HDD;
single-file concurrent temporary-name collision; FDD pair replacement failure
via an occupied sidecar backup name after both temporary files were staged;
resident FDD preservation; stale original raw image/sidecar preservation; and
absence of the exact generated temporary names.  It reports:

```text
M5:T441:S1:MEDIA-SAVE-FAILURE-HYGIENE:OK
```

The MinGW toolchain cannot link an AddressSanitizer build (`ld: cannot find
-lasan`), so sanitizer leak instrumentation is unavailable on this host.  The
observable allocation cleanup is therefore proven by the corrected owning
failure path and the real staged-pair collision cleanup assertion; no
test-only allocator or production hook was introduced.

## Verification and boundary

- `vm-media-provider-smoke` passes all retained media markers and the T441
  marker above.
- `current-fast-smokes-gcc` passes.
- The full `current-gates-gcc` reaches the independent existing T344 baseline:
  `expected 71 direct machine constructors, found 75`; T441 changes neither
  constructor source nor the historical-fixture gate.
- The developer artifact is `nxvm_0_5_0441.exe`, SHA-256
  `C06CF7C46553E53FE7592ABF9C7900110DB3F6A1FF4183C300F4EC8B4D40273F`.

The pair helper still does not claim portable cross-file atomic replacement:
host rename/remove failures are returned to the FDD caller, and no stronger
filesystem guarantee is inferred.
