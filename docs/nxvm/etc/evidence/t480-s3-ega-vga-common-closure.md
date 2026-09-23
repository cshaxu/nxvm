# T480 S3 EGA/VGA-common VADP closure

## Result

S3 closes the selected source-backed EGA/VGA-common register surface in the
existing VADP owner.  It adds no video object, VM state, profile binding, or
parallel port path.

## Retained production path

`guest/firmware port or mapped-memory access -> VADP state -> copied display
snapshot -> VM presentation` remains the only route.  `t_vadp_data` retains
the CRTC, sequencer, graphics, attribute, latch, aperture and planar-memory
state; the VM receives only a copied snapshot.

## Reconciled behavior

| Surface | S3 result | Evidence basis |
| --- | --- | --- |
| Sequencer index 3 | Supported and masked to its six stored bits in the existing five-register sequencer bank. | IBM VGA primary ledger V2; it is common EGA/VGA character-map state. |
| Graphics data `3CF` | Reads the selected supported graphics register from that same VADP bank; unsupported selection reads zero. | IBM ledger V3 and the existing indexed-port grammar. |
| Attribute data `3C1` | Reads the selected supported attribute register from the same VADP bank; unsupported selection reads zero. | IBM ledger V5 and the existing attribute flip-flop grammar. |
| CRTC, latches and aperture | Retained; no duplicate mechanism was found in the full VADP/configuration/snapshot caller sweep. | Accepted S2 ledger; focused regressions. |

The three new read paths replace the former fabricated zero values.  This
removes a false second observation of VADP state rather than adding an
abstraction.  Production code changes are 16 additions and 5 replacements;
test changes replace stale zero-read expectations and add no new target.

## Verification

The following command passed after the final source and test changes:

```text
cmake --build build/mingw-gcc-x64 --target core-machine-ega-external-port-smoke core-machine-ega-sequencer-port-smoke core-machine-ega-registration-transaction-smoke core-machine-ega-controller-port-smoke core-machine-ega-crtc-boundary-port-smoke core-machine-ega-planar-port-smoke core-machine-ega-mode10-contract-smoke vm-ega-sequencer-system-smoke vm-ega-planar-system-smoke vm-ega-planar-dos-smoke vm-ega-mode10-boot-smoke vm-ega-controller-system-smoke vm-rom-ega-int10-system-smoke
ctest --test-dir build/mingw-gcc-x64 -R "(core-machine-ega|vm-ega|vm-rom-ega)" --output-on-failure
```

All 14 selected tests passed.  The port smokes emit
`M5:T480:S3:COMMON-OWNER:OK` and `M5:T480:S3:EGA-VGA-COMMON:OK`.

## Explicit transfer

S4 exclusively owns VGA-only DAC `3C6h`--`3C9h`, VGA aperture/chain-4,
256-colour frame construction and Mode 13h.  S5 exclusively owns a source-
backed unselected VGA card/profile binding.  Neither is implied by this common
register closure.
