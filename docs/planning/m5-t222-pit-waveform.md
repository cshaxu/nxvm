# M5 T222: PIT Elapsed-Tick Waveform

## S1: Contract And Port-Probe Design

**Status:** complete.

## Scope

T222 replaces the present coarse counter with a deterministic, elapsed-tick
8253/8254-admitted waveform subset. It does not use host time, claim RTC
behavior, or introduce a VM-side device loop.

The core PIT solely owns counter, reload, phase, GATE, latch/read-back, and
OUT state. It receives only the T219 elapsed-tick delta. Channel 0 reports its
OUT *level* through the T216 PIC IRQ0 source boundary; it never mutates CPU
interrupt state. The PIC decides edge/level delivery.

## S1 Contract

Each channel has a port-visible raw count and an internal effective count.
Binary raw `0000h` loads 65536 ticks; packed-BCD raw `0000h` loads 10000
ticks. The raw count is refreshed from the effective count before a normal
read or count latch. A status latch remains first-read priority over a count
latch, and both preserve the existing 8254 read-back selection convention.

GATE is an explicit core PIT input. Modes 0, 2, 3, and 4 pause or restart as
specified below; modes 1 and 5 start only on a low-to-high GATE edge. A
control-word write establishes the documented initial OUT level but does not
manufacture an IRQ transition. Output providers are level callbacks and run
only when OUT changes.

| Mode | OUT and GATE contract |
| --- | --- |
| 0 interrupt on terminal count | programming/load drives OUT low; high GATE counts down; low GATE pauses; terminal count drives OUT high and stops. |
| 1 hardware retriggerable one-shot | OUT is high until a GATE rising edge; that edge drives OUT low and reloads/retriggers; terminal count drives OUT high. |
| 2 rate generator | low GATE forces OUT high and restarts the pending cycle; with GATE high, OUT is high except for one terminal low tick, then reloads high. |
| 3 square wave | low GATE forces OUT high and restarts the pending cycle; with GATE high, OUT alternates high/low with ceil(N/2) then floor(N/2) phases. |
| 4 software-triggered strobe | programming/load drives OUT high; high GATE counts down; terminal count drives one low tick, then high and stops. |
| 5 hardware-triggered strobe | OUT is high until a GATE rising edge starts/retriggers the count; terminal count drives one low tick, then high and stops. |

Modes 6 and 7 are aliases for modes 2 and 3. BCD arithmetic is decimal at the
same waveform boundaries as binary arithmetic. The task accepts no claim about
cycle-accurate 8254 internals beyond this stated port and waveform contract.
The degenerate mode-3 reload `N=1` has no low phase and remains high; this
prevents a fabricated unsigned underflow from becoming a waveform.

## S1 Probes And Gates

The focused probe will program ports `40h`--`43h`, use the public GATE input,
and assert these observable sequences:

- mode 0 pause/resume and terminal high transition;
- mode 1 rising-edge start and retrigger;
- mode 2 one-tick terminal low, reload, and channel-0 IRQ0 level transitions;
- mode 3 odd-count high/low phase lengths;
- modes 4 and 5 one-tick strobe behavior;
- binary zero reload, BCD zero reload, and status/count read-back ordering.

S2 may alter PIT only after this contract and probe exist. S3 additionally
requires PIT port vectors, DOS tick/time behavior, FDD/HDD boot, DOS prompt,
Console/debugger regression, and an update to T221's PIT relative-rate
evidence. T222 exits only when no host-timer shortcut or per-CPU-instruction
fake timer behavior remains.

## S2 Implementation

`t_pit` now keeps effective `reload`, `remaining`, and mode-3 `phase` values
beside the port-visible raw count. This represents binary 65536 and BCD 10000
reloads without `uint16_t` underflow. `core_machine_pit_set_gate()` is the
only GATE input; `core_machine_pit_get_output()` is observation only. The
output provider now receives an asserted/deasserted level, and the PIC adapter
maps that level exclusively to `core_machine_pic_irq_source_assert()` and
`_deassert()` for IRQ0.

The T219 scheduler continues to call `core_machine_pit_advance()` after each
completed instruction according to the frozen profile clock. PIT has no host
clock access and neither VM composition nor platform executes devices. A
focused waveform smoke covers all admitted modes, GATE/retrigger behavior,
zero reloads, BCD, and the mode-2 repeat rate; the retained read-back smoke
covers the port-latch surface.

## S3 Evidence

The focused `core-machine-pit-waveform-smoke` includes the mode-4 GATE pause,
mode-3 count-one, and real PIC IRQ0-source probes; the retained PIT read-back
smoke also passes. The official `current-gates-gcc` preset passes 57/57
executable CTest cases plus its static ownership, artifact, source-shape, and
retired-portal gates. Runtime coverage includes FDD/HDD boot, DOS prompt and
keyboard, `MEM`, video port, Console, and debugger regressions.

The relative-rate boundary is deterministic: mode 2 reload `N` repeats every
`N` elapsed ticks after its initial low pulse; mode 3 reload `N` repeats every
`N` elapsed ticks with ceil/floor high/low phases. This remains an elapsed-tick
model, not a host-time or cycle-accuracy claim.

Developer artifact: `build/output/nxvm_0_5_0222.exe`.

SHA-256: `74D06692330BFB97807DF8D8F915BAB69F403ABB17D3532C719DA3937CFC904E`.
