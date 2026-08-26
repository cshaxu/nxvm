# T479 S6 IBM 5160-268/Xebec Fixed-Disk Current-Gap Ledger

`M5:T479:S6:IBM5160-GAPS:OK`

This consumes X1--X17 in the [source ledger](t479-s6-ibm5160-fixed-disk-source-ledger.md).

| Rows | Current owner/path | Gap and required S6 result | Disposition |
| --- | --- | --- | --- |
| X1--X4 | `core_machine_hdc` owns HDC state and PIC source; `core_machine_dma` has one typed device-provider binding.  HDC has no DMA binding. | Bind the Xebec personality to Core DMA channel 3 and PIC IRQ5 through the existing typed mechanisms.  DMA is the only data-transfer path; do not expose a second memory path. | Implement. |
| X2--X6 | `controller_interface.h` and `machine_board.c` assume every HDC has nine task-file ports; `hdc.c` transfers 16-bit task-file words. | Replace the public flat HDC configuration with a validated tagged immutable configuration: task-file fields for retained ATA/Compaq/WD1003, and four byte-stream ports plus DMA channel for Xebec.  Register exactly the selected grammar. | Implement; deletes universal task-file assumption. |
| X7, X11 | VM `hdd.[ch]` is the sole image/geometry/persistence owner; HDC owns a 512-byte sector buffer and CHS progression. | Reuse only those owners for Xebec.  Validate a selected printed geometry against inserted logical media; no MFM cache, second CHS, or new media object. | Implement. |
| X8--X10, X12 | HDC only has ATA-like status/error and command phases. | Add one Xebec request/response byte collector and finite command/response state under `core_machine_hdc`.  Read/write logical-sector commands, test-ready, recalibrate, request-sense and selected diagnostics require dedicated source-proven results.  All physical-only commands report a documented command failure, never ATA behavior. | Implement only source-proven logical subset; residuals explicit. |
| X13--X15 | Core timing can preserve a controller deadline but has no Xebec physical-drive signal or proven overall service-time model. | Retain selector values as state only if an admitted command needs them; publish no guessed rotating/seek/transfer deadline.  Keep physical status bits and media mechanics unsupported. | Manual L3 selector; L1 causal residual. |
| X16--X17 | No frozen 5160-268 profile/ROM receiver exists in `vm_session_profile_kind` or the profile resolver. | Do not invent a profile or bundle ROM.  Transfer the immutable selected-5160 construction and BYOB-ROM binding to the already ordered 5160 profile admission task; prove Core grammar in a focused Core smoke meanwhile. | Explicit transfer; no runnable profile claim in S6. |
| Retained routes | Model 339 uses IBM WD1003; Model 40 uses Compaq WD; default-at uses ATA PIO. | Preserve their task-file validators, profiles and regressions. | Required regression. |

## S6 Change Boundary

S6 may make the tagged Core HDC configuration, one Xebec-specific state branch,
typed DMA/PIC binding, port registration and focused tests agree with the
ledger.  It must not create a generic XT profile framework, controller ROM
loader, raw-MFM representation, a second HDC object, or an ATA compatibility
adapter.  The lack of a confirmed frozen 5160-268 receiver prevents a claim
that the executable can yet boot an XT disk; it does not prevent the bounded
Core controller contract and its focused proof.
