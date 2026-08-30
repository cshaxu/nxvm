# T492 S3 8272A / IBM 5160 FDC And Media Current-Code Gap List 2

`M5:T492:S3:8272A-LIST-2:IMPLEMENTED-PENDING-GOVERNANCE`

## Coverage refinement

Visual review of NEC page 5-14 found the SRT formula omitted from accepted S2.
List 1 is therefore refined from 29 to 30 rows by T3; the source universe is
unchanged and T3 belongs to the existing `Specify` timing obligation.

| List-1 rows | Actual owner/path/test | Disposition |
| --- | --- | --- |
| F1--F4 | `src/core/machine/fdc.[ch]` owns the sole status/data grammar, phases and MSR; `core-machine-fdc-smoke` exercises command/result and NDMA handshakes. | Complete |
| F5 | Same owner implements Read Data/Deleted, Write Data/Deleted, Read ID, Specify, Seek, Recalibrate, Sense Interrupt, Sense Drive Status, Format and Scan. NEC calls opcode `02h` Read Diagnostic; the existing MFM `42h` `READ_TRACK` route is that same command and transfers index-to-EOT data fields. | Complete |
| F6 | Unknown `0x10` reaches the invalid-command result (`80h`); no 765B `Version` behavior is claimed by the selected XT route. | Complete explicit L0 |
| F7, F12--F14 | Core owns command/result arrays, per-drive seek state and logical-media status mapping; focused fixture proves logical no-data, write-protect, deleted-mark, scan, format and seek paths. Raw CRC, missing-address-mark and analog overrun are not represented by the logical-media provider. | Complete for logical media; raw-media conditions require an external L3 provider contract, not a guessed status |
| F8--F11, F15--F16 | Core FDC is the sole DMA request/PIC IRQ owner and exposes immutable DMA binding; source checks `verify-dma-fdc-boundary` and `verify-fdc-state-machine-boundary`, plus focused FDC smokes, pass. | Complete logical route |
| F17--F19 | `xt_5160_268.c` freezes `3F2h/3F4h/3F5h`, no selected `3F7h`, IRQ6 and DMA2; XT profile smoke proves the single route. | Complete |
| F20 | Core binding accepts four immutable media IDs; selected XT currently supplies one ID, leaving additional attached drives as a valid external frozen-profile input, not a second controller or media cache. | External-input L3 interface |
| F21--F22 | Core FDC plus immutable media registry owns MFM/logical geometry, sectors, CHRN, format and address-mark semantics. | Complete logical route |
| F23 | DOR remains the sole reset owner. Reset release queues the controller's four ordered Sense Interrupt reports and publishes IRQ after the configured 1.024-ms deadline; the NEC READY condition governs whether the interrupt is produced. Zero conversion is the explicit immediate L2 fallback. T503 S6 reconciles the earlier ready-sampling claim. | Complete controller route; selected reset-report cardinality is Other L3 |
| T1, T7--T8 | Physical electrical/mechanical and host-file behavior are outside the logical controller owner. | Retained L4/external boundary |
| T2--T3, T5--T6 | The sole FDC owner receives one copied `clock_ticks_per_second` ratio. It converts reset (1.024 ms), Specify SRT (1--16 ms per step) and the 500-kbit/s CCR=0 byte interval (16 us) into Core deadlines with an integer ceiling; zero retains the next-progression fallback. No controller, profile callback or host clock owns a second time axis. | L3-capable interface; a macro ratio is L2 until its selected board source qualifies it |
| T4 | HLT/HUT fields remain command state, but current logical-media FDC has no head-load/unload signal or media rotation state to which their manual intervals could attach. They are an explicit external L3 board/media-provider boundary, not an invented controller delay. | External-input L3 boundary |

## One coherent implementation batch

S4 consumed all and only the FDC/media-owner gaps: make reset completion
use a controller-owned pending report state; replace fixed timing literals with one immutable
FDC timing conversion for the represented reset/SRT/byte-rate transitions; and
record HLT/HUT against their actual absent board/media signal boundary rather
than inventing delay. Raw media status remains an external L3 provider
boundary, not a guessed result.
The FDC remains the sole command/phase/DRQ/IRQ owner; media remains the sole
byte owner; profile values are frozen at construction.

No profile-side FDC, second DMA/PIC route, media cache, raw physical emulator
or host-file policy is eligible for this batch.

## Verification

`verify-dma-fdc-boundary` and `verify-fdc-state-machine-boundary` pass. Focused
CTest passes `core-machine-fdc-smoke`, `core-machine-fdc-topology-port-smoke`,
`core-machine-fdc-media-change-port-smoke` and `vm-xt-5160-268-profile-smoke`
(4/4). The first two inject the frozen nonzero conversion and prove reset/SRT/
byte deadlines; the XT smoke proves the selected production profile copies its
4,772,727-Hz macro ratio without claiming physical board timing.
