# T589 S7 OmniBook 300 hardware qualification

The cached x64 and x86 Release builds passed `build-unit-tests`,
`verify-current-specialized-gates` and the same 65 targeted unit
registrations. Each selection covers timing and transaction regressions,
the hardware chip contracts and the X86 Core and X86-NXVM manifests and
component checks. The broader S5 source baseline passed all 716 unit
registrations on each host width. S7 changes NXVM repository checks and
documentation only.

| Contract | Proof and receiver boundary |
| --- | --- |
| Am386SXLV | S2–S4 established the CPU profile, 24-bit physical address, word bus and separate management route. Core owns those bus signals. The HP board owns memory decode, clock and management-region backing. |
| PIC | The 8259 command and lifecycle tests exercise interrupt masking, cascade, priority, poll, end-of-interrupt and IRQ acknowledgment. The board must bind its selected lines. |
| PIT | The 8253 and readback tests, waveform tests and board IRQ0/divider cases cover counters, gates and clock conversion. Establish the OB300 timer input, channel consumers and refresh from board evidence. |
| DMA | The 8237 contract, board channel/binding tests and retained profile deadlines cover address, counts, terminal count and transaction ownership. Use board-owned channel connections and memory, and trace which devices consume them. |
| RTC | The MC146818 calendar and register tests and board CMOS authority tests cover flags, tick/IRQ and persistence boundaries. The board owns the HP CMOS defaults, address decode and reset/IRQ routing. |
| Display | [S5](t589-s5-ct65510-display.md) supplies the 65510 controller with one VRAM/latch owner. Board work must wire option ROM, aperture and compatible CRTC I/O, including index/data word lanes, panel state and guest timing. |
| Keyboard and pointing device | [S6](t589-s6-ob300-input.md) qualifies existing 8042/keyboard/AUX behavior. Board work must wire IRQ1/12, A20 and reset, and prove any HP-specific command through the original firmware. |

HP service publication 5965-0244, printed pp. 68–72, lists POST checks for
both 8259 controllers, both DMA controllers, 8253 channels, A20 and RTC.
The published diagnostics span multiple models. They establish concrete
compatibility targets but do not prove the exact OB300 pin connections.
Start the next task from the [SCAMP II reset and management leads](t589-s4-am386sxlv-management.md)
and the original firmware's first accesses. The FBh latch and the meaning
of its first write, ECh/EDh cold/SMM routing, card mapping and 60000h
management alias still require evidence. Use synthetic fixtures for board
signals, then prove original ROM boot in the separate machine task.

## Repository checks

- The opcode inventory now checks the primary dispatch's 256 distinct
  switch cases and the secondary dispatch's 89 modeled cases plus its
  undefined-opcode default. Previously both checks matched removed tables.
- The timing order check locates the selector independent of the old
  argument list. Its existing one-invocation and pre-interrupt assertions
  remain in force.
- The FPU static check now proves all eight ESC values enter the FPU
  dispatcher. The old negative check had no matching table left to inspect.
- The Jcc observation check now verifies the ordinary read-only path
  and the distinct SMM path, including bypassing transaction admission for
  observation and forwarding the read-only flag to the SMM provider.

Scratch fixtures confirmed rejection of a removed secondary case, an
extra timing selector at interrupt dispatch and a disconnected FPU ESC case.
These were local negative probes, separate from the two positive aggregates.

| Verification log | SHA-256 |
| --- | --- |
| x64 gate and hardware qualification | `519f6fe4df55648e2ac66075794e567aac9397cd147673d3e2f74aefa33b6b10` |
| x86 gate and hardware qualification | `1e4727120e3cfe4cfb9f09f348991c4a9c446908952f06ba0a4bbf2af1e3a025` |
