# T461 S2: 8254 Chip Phase Contract

`pit.c` remains the sole owner of all three counter records.  A completed
count write now records one pending CR-to-CE transfer; it neither creates a
second timer nor lets a VM or scheduler mutate PIT state.  The mode-local
clock edge consumes that pending transfer at the manual-defined point.

| Manual rows | Sole-owner implementation and focused proof |
| --- | --- |
| P1--P4 | One counter record retains control, byte-write state, binary/BCD zero expansion and CR. `core-machine-pit-readback-smoke` proves null count before CE load; `core-machine-pit-waveform-smoke` proves binary and BCD zero durations. |
| P5--P7 | The existing count/status latches remain counter-local; status precedes count and first pending latch wins. `core-machine-pit-readback-smoke` probes pre-load null count, post-load status, interleaved read-back and repeated latches. |
| P8 | Mode 0 consumes a completed count on its next CLK; a two-byte rewrite stops and drives OUT low on the first byte. `core-machine-pit-waveform-smoke` probes both paths. |
| P9, P13 | Modes 1 and 5 retain one rising-GATE trigger bit. The next CLK reloads CE and starts the pulse; a later trigger reloads the same owner record. Waveform probes cover trigger and retrigger. |
| P10 | Mode 2 retains one-clock low/reload state. A rising GATE sets a single next-CLK restart, while a count rewrite waits for its documented period boundary. Waveform and IRQ0 probes cover the low/high sequence and GATE restart. |
| P11 | Mode 3 stores the actual CE representation: even counts, or count minus one for odd counts, decrement by two. Its phase field owns only output-half-cycle duration; it is not a second count. Waveform and read-back probes cover phase and CE progression. |
| P12 | Mode 4 loads only at its next CLK, pauses at low GATE and emits one low strobe. The waveform probe covers the pause and strobe. |
| P14 | Control-word writes reset the selected counter's local control/latch/phase state and establish initial OUT without inventing an interrupt edge. Read-back and waveform probes cover null-count and initial-level behavior. |

Focused verification on the S2 revision:

```text
ninja -C build/mingw-gcc-x64 core-machine-pit-readback-smoke \
  core-machine-pit-waveform-smoke core-machine-pit-divider-smoke \
  core-machine-pit-irq0-s2-smoke -j 1
ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R \
  "(core-machine-pit-readback-smoke|core-machine-pit-waveform-smoke|\
core-machine-pit-divider-smoke|core-machine-pit-irq0-s2-smoke)" -j 1
```

The four tests pass. P15 remains explicitly for S3: the chip owns OUT, while
the board's OUT-to-PIC route and scheduler visibility retain their separate
owner and project-L2 ordering boundary.

The tracked runnable change is `+95/-27` Core source lines and `+138/-25`
focused-test lines (`+181` net). The added state is exactly three counter-local
facts: pending CR-to-CE load, one-shot trigger and modes-2/3 restart. It
replaces the immediate-load path; no wrapper, parallel timer, alternate OUT
route or public API was added.

`M5:T461:S2:PIT-PHASE-CONTRACT:OK`
