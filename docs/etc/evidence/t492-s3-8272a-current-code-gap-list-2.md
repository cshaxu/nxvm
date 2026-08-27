# T492 S3 8272A / IBM 5160 FDC And Media Current-Code Gap List 2

`M5:T492:S3:8272A-LIST-2:IMPLEMENTED-PENDING-GOVERNANCE`

## Coverage refinement

Visual review of NEC page 5-14 found the SRT formula omitted from accepted S2.
List 1 is therefore refined from 29 to 30 rows by T3; the source universe is
unchanged and T3 belongs to the existing `Specify` timing obligation.

| List-1 rows | Actual owner/path/test | Disposition |
| --- | --- | --- |
| F1--F4 | `src/core/machine/fdc.[ch]` owns the sole status/data grammar, phases and MSR; `core-machine-fdc-smoke` exercises command/result and NDMA handshakes. | Complete |
| F5 | Same owner implements Read Data/Deleted, Write Data/Deleted, Read ID, Specify, Seek, Recalibrate, Sense Interrupt, Sense Drive Status, Format and Scan. `Read Diagnostic` has no opcode/execute route. | **Batch A gap** |
| F6 | Unknown `0x10` reaches the invalid-command result (`80h`); no 765B `Version` behavior is claimed by the selected XT route. | Complete explicit L0 |
| F7, F12--F14 | Core owns command/result arrays, per-drive seek state and logical-media status mapping; focused fixture proves logical no-data, write-protect, deleted-mark, scan, format and seek paths. Raw CRC, missing-address-mark and analog overrun are not represented by the logical-media provider. | Complete for logical media; raw-media conditions require an external L3 provider contract, not a guessed status |
| F8--F11, F15--F16 | Core FDC is the sole DMA request/PIC IRQ owner and exposes immutable DMA binding; source checks `verify-dma-fdc-boundary` and `verify-fdc-state-machine-boundary`, plus focused FDC smokes, pass. | Complete logical route |
| F17--F19 | `xt_5160_268.c` freezes `3F2h/3F4h/3F5h`, no selected `3F7h`, IRQ6 and DMA2; XT profile smoke proves the single route. | Complete |
| F20 | Core binding accepts four immutable media IDs; selected XT currently supplies one ID, leaving additional attached drives as a valid external frozen-profile input, not a second controller or media cache. | External-input L3 interface |
| F21--F22 | Core FDC plus immutable media registry owns MFM/logical geometry, sectors, CHRN, format and address-mark semantics. | Complete logical route |
| F23 | DOR owns reset; current release fabricates four ready-change Sense Interrupt results on every reset release instead of conditioning results on sampled ready inputs. | **Batch B gap** |
| T1, T7--T8 | Physical electrical/mechanical and host-file behavior are outside the logical controller owner. | Retained L4/external boundary |
| T2--T6 | `CORE_MACHINE_FDC_SEEK_TRACK_TICKS` and `CORE_MACHINE_FDC_500K_BYTE_TICKS` hard-code a 3-ms seek assumption and one byte gate; SRT is ignored, HLT/HUT are only stored, reset completion is immediate and no immutable source-qualified FDC timebase reaches Core. | **Batch C gap: manual timing facts currently fall back to L2** |

## One coherent implementation batch

S4 must consume all and only the FDC/media-owner gaps: add the omitted Read
Diagnostic command; make reset completion reflect sampled ready inputs; replace
the fixed timing literals with one immutable FDC timing input that carries
source-qualified SRT/HLT/HUT/reset/byte-rate values when a profile provides
them and explicitly retains L2 otherwise. It must also define a bounded typed
logical-media outcome for source-defined raw status only when an external media
provider can supply it. The FDC remains the sole command/phase/DRQ/IRQ owner;
media remains the sole byte owner; profile values are frozen at construction.

No profile-side FDC, second DMA/PIC route, media cache, raw physical emulator
or host-file policy is eligible for this batch.

## Verification

`verify-dma-fdc-boundary` and `verify-fdc-state-machine-boundary` pass. Focused
CTest passes `core-machine-fdc-smoke`, `core-machine-fdc-topology-port-smoke`,
`core-machine-fdc-media-change-port-smoke` and `vm-xt-5160-268-profile-smoke`
(4/4). These results establish present ownership only; they do not close
Batch A--C.
