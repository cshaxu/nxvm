# T497 S5 IBM 5170 BYOB Media Replay

`M5:T497:S5:IBM5170-BYOB:OK`

## Admitted Media Results

| Profile, CPU, format, image | Result | Disposition |
| --- | --- | --- |
| Model 339, 80286, 360 KB, owner-supplied MS-DOS 5.0 boot disk | `installer-ready` | Accepted semantic terminal after the aperture repair. |
| Default-AT, 80286, 360 KB, same image | `installer-ready` | Control passes. |
| Default-AT, 80386, 360 KB, same image | `installer-ready` | Control passes. |
| Model 339, 80286, 1.2 MB, owner-supplied MS-DOS 6.22 boot disk | Repeated `66`-prefix #UD after successful FDC transfers | This asset requires a later CPU; it is not valid 80286 acceptance media. |
| Default-AT, 80386, 1.2 MB, same image | `dos-prompt` | Confirms the 1.2 MB FDD/FDC route and the asset's 80386 execution requirement. |

No external asset is tracked or named by a repository path in product code.

## First-Failure Repair

Before repair, Model 339 reached the legal instruction `26 A1 00 00` (`MOV
AX, ES:[0000]`) with `ES=B000h`. The 512-KiB CGA-only profile did not declare
the unpopulated monochrome aperture at `B0000h-B7FFFh`; the CPU physical read
therefore failed and surfaced as an internal CPU case error. This was neither
a 360-KB geometry nor an FDC/BIOS defect.

The immutable Model-339 descriptor now selects that aperture. The profile
materializes it through the existing Core absent-memory provider, which returns
`FFh` and discards writes. CGA `B8000h-BFFFFh` remains VADP-owned. This is the
sole memory mapping path; no CPU decoder, firmware handler, FDC state or image
heuristic changed.

## Focused Proof

- `vm-ibm-5170-model-339-composition-smoke` asserts the descriptor flag and
  provider query/read/write behavior at `B0000h` as well as the existing
  extended-memory absence.
- The Release BYOB replay reaches `installer-ready` for the 360-KB supported
  Model-339 medium.

The 1.2-MB drive remains a supported Model-339 drive geometry; this particular
DOS 6.22 image cannot prove 80286 usability. A compatible 1.2-MB 80286-era
boot image remains required for that separate semantic replay.
