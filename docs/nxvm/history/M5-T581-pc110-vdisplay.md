# M5 T581 — MyPC110 display selection

Owner admitted the queued [proposal](../proposals/m5-pc110-display-variant-selection.md) on 2026-10-29. T580 S90 provides the accepted baseline. This task retains the proposal's two-artifact, stopped-only switching and complete failure/CMOS verification requirements.

## S1 delivery

Implemented one MyPC110 product, DSTN default and stopped-only
`vdisplay=dstn|tft`. Selection is applied by the existing worker before cold
reset; the Machine and attached media survive selection. Separate CMOS/RTC
images remain App-owned. Both firmware variants are embedded in each of the
two released MyPC110 EXEs; the obsolete TFT pair is removed.

All five PC products were rebuilt at 0.5.0581. Dual-width full unit suites
passed 712/712 each. Targeted product/lifecycle and real-firmware integrations
passed, and both architectures reached Personaware before and after selection
in the same Machine. [S1 evidence](../etc/evidence/t581-s1-vdisplay-design.md)
records scope, proof, hashes and limits; [artifact inventory](../etc/evidence/t581-s1-artifacts.json)
identifies all ten PC EXEs. No user INI/CMOS/media, MyNES/MyRaidenII or
Lib/Emulator code was changed. Native desktop/RDP acceptance remains with the
owner; S1 and T581 remain open pending acceptance.

Shared P1 `8031ee3e3` supplies the generic construction/reset seams and their
regressions (+153/-30 including presets and manifests). NXVM P2 delivers
the App, release artifacts and evidence as a separate target-scoped commit.

## Owner closure

On 2026-10-29 the owner accepted delivery `8031ee3e3` / `59ad6e78b` and
closed S1 and T581. The clean committed baseline retains all recorded test
results and limits; performance is the separately admitted next task, not
an unfulfilled display-selection criterion.
