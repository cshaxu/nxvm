# T433: Core L3 Master Ledger And Conformance Report

`M5:T433:S5:MASTER-LEDGER:OK`

## Master ledger

| ID | Owner / evidence / regression | Disposition |
| --- | --- | --- |
| CPU-EXEC | Core CPU; [S2](t433-s2-core-cpu-capability-reconciliation.md) | Conformant selected forms; unallocated forms transfer. |
| CPU-EXCEPT | Core CPU; S2 exception smoke | Accepted L2 timing fallback. |
| CPU-PREFETCH | Core CPU; S2 prefetch smoke | Accepted L2 physical-producer fallback. |
| CPU-RETIRE | Core machine; S2 retirement smoke | Conformant deterministic observation; physical qualification transfers. |
| CPU-FPU | Core FPU; S2 FPU smokes | Accepted L2/x87 TODO fallback. |
| TIME-CLOCK | Core clock/timeline; [S3](t433-s3-core-transaction-time-reconciliation.md) | Conformant deterministic contract; physical oscillator transfers. |
| TIME-LIFECYCLE | Core machine reset; S3 lifecycle smoke | Conformant. |
| TXN-MEMORY | Core transaction/memory; S3 smokes | Conformant logical route; DRAM/READY transfers. |
| TXN-PORT | Core transaction/port; S3 smokes | Conformant logical route; electrical timing transfers. |
| TXN-ARBITRATION | Core transaction/DMA; S3 smokes | Conformant logical order; HOLD/DACK transfers. |
| MEM-RAM-A20-PARITY | Core memory/machine; S3 smokes | Conformant logical route; physical controller timing transfers. |
| MEM-ROM-FIRMWARE | Core firmware/ROM; firmware capability smoke | Conformant immutable/copied lifecycle; board timing transfers. |
| MACHINE-CONFIG | Core machine config; configuration/port-assembly smokes | Conformant copied validated topology; later machine-plan candidate receives declarative timing. |
| CTRL-PIC | Core PIC; [S4](t433-s4-core-controller-device-reconciliation.md) | Conformant logical lifecycle; INTA transfers. |
| CTRL-DMA | Core DMA; S4 DMA smoke | Conformant logical lifecycle; DREQ/DACK transfers. |
| CTRL-PIT | Core PIT; S4 PIT smoke | Conformant deterministic route; physical phase transfers. |
| CTRL-RTC-CMOS | Core RTC; S4 RTC smoke | Conformant logical route; physical clock transfers. |
| CTRL-KBC-NMI | Core KBC/machine; S4 KBC smokes | Conformant selected route; physical delay transfers. |
| CTRL-FDC | Core FDC; S4 FDC smoke | Conformant logical service; flux/phase transfers. |
| CTRL-HDC | Core HDC; S4 HDC smoke | Conformant current ATA route; physical disk/MFM transfers. |
| MEDIA-BACKING | Core media; S4 provider smoke | Conformant logical provider; physical media transfers. |
| DISPLAY-VADP | Core VADP; S4 display smoke | Conformant digital route; monitor/contention transfers. |
| DISPLAY-PRESENT | Core presentation; presentation/mailbox smokes | Conformant copied snapshot boundary. |
| INPUT-HOST | Core platform input/KBC; input source smoke | Conformant copied ingress; host cadence transfers. |
| TRACE-DEBUG | Core trace/debug; trace/debug smokes | Conformant copied observation boundary. |
| PLATFORM-MAILBOX | Core platform mailbox; mailbox smoke | Conformant copied publication boundary. |
| PLATFORM-RESOURCE | Core backing resource; resource smoke | Conformant provider lifecycle. |
| PLATFORM-WAIT | Core platform wait; wait smoke | Not applicable to guest-time/L3; host adapter only. |
| SESSION-COMMAND | Core product session; session-manager smoke | Conformant product utility boundary; not machine timing. |
| PRODUCT-DEBUG | Core product debug; debug-target smoke | Conformant product tooling boundary; not machine timing. |

## Conformance conclusion

All 30 frozen IDs have exactly one owner/evidence/disposition. The audit finds
no implementation discrepancy requiring repair. Corrective S6 supplements
this summary with the required actual L3 fact gap, Core-feasibility decision,
plan/program declaration, evidence admission and focused proof for every row.
Corrective S7 then distinguishes retained source sufficiency from an absent
Core rule, a VM-profile fact, or a host/product boundary:
[T433 S7 source-sufficiency ledger](t433-s7-core-source-sufficiency-ledger.md).
No boot/default timing result or generic feasibility result is used as proof.
This remains a factual Core migration baseline, not an L4 or universal-device
claim.

`M5:T433:S5:CONFORMANCE:OK`
