# T489 S2 IBM 5160 8237A Function And Timing List 1

`M5:T489:S2:8237A-LIST-1:ACCEPTANCE-CANDIDATE`

This is the complete selected unit: one Intel 8237A-5, its IBM XT page-latch
and refresh wiring, and the board-visible request route. It excludes AT
secondary DMA, individual FDC/Xebec service semantics and electrical waveform
simulation. Intel and IBM sources in the S1 ledger are normative; later
emulator comparison can corroborate, but cannot alter, these rows.

| ID | Source | Complete function, reset/cancellation and timing rule | Disposition |
| --- | --- | --- | --- |
| DMA-R1 | Intel 7, 10 | Each of four channels has base/current address and word count. Program access uses the first/last flip-flop; address moves by programmed direction, count decrements and terminal count is zero to FFFFh. Auto-init restores current values only at EOP. | Manual-L3 state/order; board transaction schedule L2. |
| DMA-R2 | Intel 7--10 | Command selects controller enable, priority, timing, transfer and memory-to-memory choices. Per-channel mode selects service mode, transfer type, direction and auto-init. Reset/master clear disables and clears documented control state. | Manual-L3. |
| DMA-R3 | Intel 1, 7--10 | Request/mask/status/temporary state supplies software request, DREQ masking, terminal-count observation and memory-to-memory byte holding. Status read clears terminal-count status; master clear resets request/status/temporary and masks all channels. | Manual-L3. |
| DMA-R4 | Intel 7--10 | Clear first/last, master clear, clear mask, single-mask and all-mask commands are distinct programming operations. | Manual-L3; host transaction elapsed placement L2. |
| DMA-F1 | Intel 1, 3, 5 | Unmasked DREQ or software request selects a channel; DACK acknowledges service. Fixed priority is channel 0 through 3, and rotating priority demotes last serviced channel. | Manual-L3 arbitration/order; external request cadence L2. |
| DMA-F2 | Intel 1, 3, 16 | A valid request asserts HRQ; accepted HLDA starts service. Normal service has SI, S0, S1--S4 and READY wait state SW; end releases HRQ and waits for HLDA low before new service. | Manual-L3 logical state sequence; Core placement L2. |
| DMA-F3 | Intel 5 | Single transfers one unit then releases; demand continues while DREQ is active; block continues until TC/EOP; cascade delegates one priority slot. TC/EOP and demand DREQ-low end service under the documented rules. | Manual-L3 mode semantics; CPU interleave duration L2. |
| DMA-F4 | Intel 1, 16 | Read moves memory to I/O, write moves I/O to memory, verify advances controller state without memory/I/O controls. Data is not staged through a DMA data owner. | Manual-L3 logical function; signal pulse widths L4. |
| DMA-F5 | Intel 3, 5, 7 | External EOP or terminal count ends a service; EOP is emitted on TC, updates request/mask as documented and may auto-initialize. | Manual-L3 lifecycle/order; pin waveform L4. |
| DMA-F6 | Intel 1, 5, 17 | Memory-to-memory uses channel 0 source, channel 1 destination and temporary register; channel-1 TC ends it. Figure 12 defines the two state groups. | Manual-L3 state sequence; physical cycle rate L2/L4. |
| DMA-F7 | Intel 7, 18 | Compressed timing removes S3 where system conditions permit; S1 remains when high address latch changes. The compressed sequence remains S2/S4; reset cancels active service. | Manual-L3 state selection; board clock conversion L2 and electrical limits L4. |
| DMA-T1 | IBM XT 1-7, 1-8 | One 8237A-5 and external DMA address/page latch are selected at `000h`--`00Fh` and `080h`--`083h`; there is no AT secondary controller. | Manual-L3 XT topology. |
| DMA-T2 | IBM XT 1-8, 1-14 | Expansion exposes DRQ1--3 and DACK0--3; DMA channel 1 is reserved for memory refresh, leaving the named non-refresh routes to their own device T. | Manual-L3 topology/causal routing; producer service cadence L2. |
| DMA-T3 | IBM XT 1-14 | The XT gives every DMA transfer five clocks, 1.05 microseconds per byte, when I/O ready is not deactivated. | Manual-L3 selected board formula; use requires the existing one Core time/transaction owner. |
| DMA-T4 | IBM XT 1-14 | Refresh cycles occur every 72 clocks, approximately 15 microseconds, require four clocks and consume channel 1 through the board timer/request route. | Manual-L3 selected board formula and route; timer implementation is PIT's unit. |
| DMA-T5 | Intel 13--18 | Data-sheet master/slave timing, 200 ns minimum cycle and signal setup/hold figures are electrical part constraints. | L4/out of scope; never convert directly to Core ticks without a selected compatible board contract. |

The frozen universe is `DMA-R1`--`DMA-R4`, `DMA-F1`--`DMA-F7` and
`DMA-T1`--`DMA-T5`: 16 rows. It covers every selected register/command,
request/grant and service mode, transfer/termination/reset behavior, XT
page/refresh route and timing category. S3 must map all sixteen rows to one
current owner path before any implementation decision.
