# T497 S4 IBM 5170 Floppy Implementation

`M5:T497:S4:IBM5170-FLOPPY:OK`

## One Owner Path

The immutable Model-339 descriptor now declares the factory 96-TPI drive:
CMOS type `02h` (`0x20`) and no Drive-A field upgrade.  The session-local
selector maps an omitted or explicit `1200k` request to the existing 80x2x15
FDD geometry and an explicit `360k` request to the existing 40x2x9 geometry.
It rejects `720k` and `1440k`.  The physical-drive CMOS byte is not changed by
the compatible medium.

The existing flow remains the only production path:

`profile descriptor -> session floppy_kind -> FDD geometry -> Core FDC`.

No FDC, BIOS, Core timing, image-size inference, second media state, or
Default-AT policy is introduced.  The factory accepts the explicit format for
Model 339 only so it reaches the same selector; all other non-default profile
restrictions remain intact.

## List-2 Disposition

| Rows | Result |
| --- | --- |
| F1 | Descriptor and validator now use no field upgrade and CMOS `0x20`. |
| F2, F5 | Default and explicit `1200k` select 1.2 MB. |
| F3, F6 | Explicit `360k` selects existing compatible geometry without CMOS mutation. |
| F4 | Factory/direct requests share the same positive and negative selection policy. |
| F7 | Retained: no FDC/controller change. |
| F8 | Retained: no physical drive timing claim or receiver was added. |

## Focused Release Verification

After a Release rebuild, all focused executables returned zero:

| Command/result | Purpose |
| --- | --- |
| `vm-ibm-5170-model-339-composition-smoke` -> `M5:T497:S4:IBM5170-FLOPPY:OK` | native 1.2 MB, compatible 360 KB, rejected later formats, and unchanged physical CMOS type. |
| `vm-ibm-5170-model-339-firmware-fdc-topology-smoke` -> `M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK` | retained firmware/FDC topology. |
| `vm-default-pc-at-apply-smoke` -> `M5:T208:S3:DEFAULT-PC-AT-APPLY:OK` | Default-AT remains a distinct four-format policy. |

The implementation adds one selector and focused assertions; it removes the
false factory 1.44-MB/default-upgrade claim.  S5 owns actual BYOB boot replays.
