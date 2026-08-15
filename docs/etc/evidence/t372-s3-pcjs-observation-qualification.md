# T372 S3: PCjs Model-339 Observation Qualification

## Decision

No PCjs observation is admitted. Public PCjs documentation establishes a
candidate configuration vocabulary, not a compliant, revision-pinned
physical-timing experiment for the selected Model-339 field configuration.
The available runnable routes require third-party firmware and, for the
field-upgrade floppy path, DOS/guest media. The source policy prohibits
downloading, retaining, cataloguing, or making either a project research
asset. S3 therefore stops before launching a machine or collecting a trace.

## Locked-field qualification

| Selected field | PCjs public evidence | S3 result |
| --- | --- | --- |
| Model 339 / Type 3, 8 MHz, 512 KB, Rev.3, enhanced keyboard | PCjs's 5170 documentation describes Models 319/339 with these identity fields. | Documented candidate only; the page is not an immutable machine-run configuration. |
| CGA | PCjs exposes a `monitor="color"` chipset property. | Configurable candidate; no recorded instance ties it to every other selected field. |
| 1.44 MB field upgrade | PCjs exposes a `1440` floppy value and states that it requires Rev.3 plus DOS 3.20 or later. | Not observable in S3: the required DOS path is protected guest media and the upgrade remains a field configuration, not a factory Model-339 fact. |
| No fixed disk/ATA | PCjs documents PC AT hard-drive types and its command-line utility creates/configures hard-disk images for `ibm5170`. | No public immutable no-disk run record was found. S3 does not infer absence merely because a configuration option might exist. |
| Reference revision and trace | The public page identifies PCjs as a browser emulator and links its source, but provides no revision-pinned, whole-tuple run record. | Absent. No observation can be reproduced as required by S2. |
| Physical boundary | The page exposes configuration, not READY/wait, DMA handshake, controller clock, CGA retrace, or reset-settle measurement. | Absent. A browser-emulator event would in any case be corroboration, never an IBM hardware measurement. |

The consulted public materials are [PCjs's 5170 documentation](https://www.pcjs.org/machines/pcx86/ibm/5170/), its [machine utility documentation](https://www.pcjs.org/tools/pc/), and its [CPU-test landing page](https://www.pcjs.org/software/pcx86/test/cpu/). They establish the configuration and asset dependency above; S3 neither downloads nor runs them.

## Primary-gap applicability

The unresolved S1 groups are board conversion and physical-phase boundaries,
not an unallocated 80286 successful-retirement row. Consequently the owner
exception for secondary references does not itself authorize an emulator
instruction scalar. PCjs supplies no primary-manual range, board measurement,
or source-labelled observation for RAM/ROM/CGA/ISA waits, DMA/PIT/PIC
handshakes, FDC/RTC/KBC service, parity/CGA propagation, or reset settle time.

## Transfer

The exact receiver is the next admitted T372 physical-boundary evidence S:
it must seek a permitted primary-board/controller source or an owner-approved
research method that neither consumes nor retains third-party firmware or
guest media. Before any secondary observation, it must satisfy all four S2
admission conditions, including an immutable whole-tuple configuration and a
public stimulus. No residual is closed and **5170 Model-L3 remains open**.

## Similar-issue sweep

S3 checked every locked field, the documented PCjs display/floppy/disk
controls, the public CPU-test/tool routes, every S1 residual group, and the
already rejected 86Box/MAME alternatives. None supplies both a compliant
whole-tuple configuration and a primary-source-open physical measurement.
No source, firmware, media, binary, raw trace, runtime behavior, ABI, or
timing value changed.
