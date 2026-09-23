# Td S151 Four-Machine Fixed-Disk Receiver

`M5:Td:S151:FIXED-DISK-RECEIVER:OK`

## Reconciliation

| Selected machine | Existing personality and owner | Disposition |
| --- | --- | --- |
| `default-at` | Core ATA PIO personality; VM owns the logical image and persistence. | Retain as a focused regression input. |
| IBM 5170 Model 339 | Core IBM WD1003/ST-506 personality; Model 339 selects its Type-3 logical route. | Retain as a focused regression input. |
| Compaq DeskPro 386 Model 40 | Core Compaq/WD personality; the Model-40 profile retains its own immutable topology. | Retain as a focused regression input. |
| IBM PC/XT 5160-268 | T479 S6 establishes Xebec's byte-stream `320h`--`323h`, DMA3 and IRQ5 contract, but no XT profile exists. | The queued XT audit, functional closure, phase closure and final audit are the sole receiver sequence. |

The four affected proposals now partition the one missing route without changing
the existing HDC/media ownership: the profile audit freezes Xebec selection and
its immutable binding; functional closure owns the controller grammar and
logical-image consumer proof; the XT phase task owns availability/publication
and source-backed phases; the final XT audit proves the four-machine result.

No proposal authorizes an ATA compatibility shim, a second CHS/media cache,
raw-MFM representation, guessed physical deadline, generic XT profile, or a
firmware/media import. T479 remains correctly closed because its completed
scope established the HDC boundary and transferred the absent machine receiver.

`M5:Td:S151:GOVERNANCE:OK`
