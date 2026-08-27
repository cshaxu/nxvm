# T497 S1 IBM 5170 Floppy Source Ledger

`M5:T497:S1:IBM5170-FLOPPY-SOURCE:OK`

## Source Hierarchy

| Source | Inspection | Conclusion | Authority |
| --- | --- | --- | --- |
| IBM *PC/AT Technical Reference*, 1502243, March 1984, System Board p. 1-50 and diskette compatibility text | The printed CMOS drive-type table was visually checked; OCR was used only to locate it. | CMOS type `01h` is double-sided 48-TPI; `02h` is high-capacity 96-TPI. A high-capacity drive reads/writes 160/180 KB, 320/360 KB and 1.2 MB media. | Manual L3. |
| 86Box `src/floppy/fdd.c` and `fdd_td0.c` | Read-only source inspection. | Its 5.25-inch 1.2 MB drive model has a double-step capability and explicitly distinguishes 360 KB media in a 1.2 MB drive. | Other L3 corroboration; no code or timing is imported. |
| PCjs `machines/pcx86/progressive/pro386/ega/2048kb/machine.xml` | Read-only source inspection. | Its IBM-5170 chipset configuration selects `[1200,1200]` floppies. | Other L3 corroboration of the selected 1.2 MB configuration. |
| Bochs 2.6 local source | Its generic floppy device is available, but no selected 1984 IBM 5170 machine declaration was found in this bounded inspection. | No Model-339 fact is derived from it. | Not evidence for this row. |
| MAME and QEMU | No local checkout was available. | No claim is made from their absence. | Unavailable. |

## Frozen Facts

| ID | Required fact | Source conclusion | Level | Next receiver |
| --- | --- | --- | --- | --- |
| F1 | Original Model 339 Drive A is a 96-TPI high-capacity drive. | IBM CMOS type `02h`. | Manual L3 | List 1. |
| F2 | 1.2 MB is native selected media. | IBM high-capacity compatibility statement; PCjs corroborates. | Manual L3 | List 1. |
| F3 | 360 KB is compatible media, not a replacement physical-drive declaration. | IBM high-capacity compatibility statement; 86Box corroborates double-step distinction. | Manual L3 | List 1. |
| F4 | 720 KB and 1.44 MB are not original 1984 Model 339 drive defaults. | IBM table enumerates only 48-TPI and 96-TPI 5.25-inch types for this system; no primary row selects later 3.5-inch hardware. | Manual L3 negative configuration boundary. | List 1. |
| F5 | Exact mechanical double-step, rotation and ready timing are not claimed by the current geometry-only FDD path. | The selected IBM facts establish drive/media compatibility, not a complete physical service-time model. | Explicit non-claim. | List 1. |

## Current Owner Gap

`src/vm/profile/default_profile/pc_at_profile.c` declares Model 339 with
`diskette_drive_a_field_upgrade == true` and CMOS floppy type `0x40`, the
current project's 1.44 MB selection. `src/vm/composition/session/session.c`
does not initialize Model 339 `floppy_kind`; the zero enum value is also
1.44 MB. `session_factory.c` rejects every explicit Model-339 floppy request.
The result is a false physical-drive declaration plus a hidden default, while
the FDD itself already has distinct 1.2 MB and 360 KB geometry values.

List 1 must define the immutable drive versus mounted-media distinction; List
2 must bind the correction to the existing descriptor, session request and
FDD constructor only. Default-AT's explicit four-format route, XT's 360 KB
route and Model-40's 1.2 MB route are distinct contracts and must remain
unchanged.
