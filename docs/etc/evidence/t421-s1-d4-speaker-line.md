# T421 S1 DeskPro D4 Speaker-Line Evidence

`M5:T421:S1:D4-SPEAKER-LINE:OK`

## Authority and scope

The original Compaq DeskPro 386 Processor Description (D3PE), port `61h`
description and speaker section, defines bit 0 as the timer-2 gate and bit 1
as speaker data. With bit 0 clear, software directly controls the speaker by
toggling bit 1. With both bits set, timer 2 drives the tone. The documented
input is 1.193 MHz. This is original-source design evidence for the selected
Model-40 logical behavior. PCjs was read only as an independent cross-check;
no third-party code, firmware, media or ROM was imported.

The Core now owns a copied logical line with this exact rule:

`output = data_enabled && (!timer_gate || pit_channel_2_output)`

The rule deliberately represents only the digital line. It does not claim a
speaker cone waveform, amplitude, electrical level, calibrated frequency,
host audio device or host-time scheduling.

## One owner and profile selection

`core_machine` is the only mutable owner: it receives port-B writes, gates
shared PIT channel 2, samples the PIT callback, restores state on reset and
publishes `core_machine_speaker_observation` as a copied snapshot. VM/profile
composition adds no port provider or signal path. The DeskPro Model-40 already
selects D4 port B, so it executes the original Compaq behavior through that
owner. The IBM 5170 Model 339 already selects planar PC/AT port B and now uses
the same owner, not a parallel implementation.

Host audio, PPI breadth and physical acoustic behavior remain explicitly
transferred in the TODO ledger.

## Verification

Focused rebuilt tests passed:

- `current.core-machine-d4-platform-s4-smoke`: direct bit-1 control, timer-2
  gate/data output changes, and reset restoration.
- `current.vm-model40-private-composition-s7-smoke`: Model-40 D4 selection.
- `current.vm-ibm-5170-model-339-composition-smoke`: 5170 planar reuse.
- `current.core-machine-planar-parity-nmi-s3-smoke`: adjacent port-B/NMI path.

The rebuilt developer artifact is `vm-0-5-0419` at
`build/output/nxvm_0_5_0419.exe`, SHA-256
`859E1B93C6891E8EAAF0D98D4DBEF25F2383F911EC243390A50FB9A9CDBBA5BF`.

A serial current-gate run completed all 289 tests: 286 passed and three
existing DOS-smoke failures remain (`vm-dos-prompt-smoke`,
`vm-dos-keyboard-smoke`, `vm-dos-mem-fault-smoke`). They do not exercise
Model-40 or the new Core speaker observation, but they prevent a clean
whole-gate claim and remain unresolved outside this bounded receiver.
Documentation governance passes.