# T386 S28 CECG Odd/Even Page Receiver

## Scope and source boundary

The accepted S27 residual audit identified the selected Compaq Enhanced Color
Graphics `3C2h` bit 5 as the sole remaining shared functional receiver. The
primary guide labels that bit low page/high page in Odd/Even mode. Its Graphics
Controller Miscellaneous description says that, when selected, CPU address A0
is replaced by a higher CPU bit or this page-select bit. It also identifies the
Sequencer odd/even memory mode and the Graphics Controller mode as the related
CPU address/plane controls.

This receiver implements only that determined digital state path. It does not
model IBM EGA, VGA, Special Features, monitor selection or blanking, sync
signals, live-video status, firmware-programmed modes, electrical signalling,
raster duration, arbitration, waits, or L3 timing. The primary material was a
transient research input under the source policy; no manual content, vendor
firmware, media, local path or hash is retained here.

## Owner and mechanism

`core/machine/vadp.c` owns the reusable mutable page state and its consumers.
For the Compaq personality, with sequencer Odd/Even selected and Graphics
Controller Miscellaneous page substitution enabled, the CECG `3C2h` bit 5
selects one of the two 16-KiB page regions in each existing 64-KiB planar
backing store. The same owner applies the selected region to CPU planar reads,
CPU planar writes, and the copied display snapshot. A page change marks the
copy dirty. VADP reset restores the immutable CECG declaration; the Model 40
composition explicitly declares low page.

This is one shared VADP address/snapshot path, not a Model-40-specific memory
provider or a new test API. Generic EGA has no CECG `3C2h` route and therefore
cannot select this Compaq-only input.

## Focused proof

- `core-machine-compaq-cecg-s28-smoke` configures the real VADP memory provider,
  enables the source-defined path, writes distinct low/high-page values through
  the physical memory owner, verifies low/high reads and copied-frame pixels,
  verifies dirty publication, resets to the declared low page, and proves the
  generic personality has no `3C2h` write route.
- `vm-model40-cecg-s28-smoke` creates the fixed private Model-40 composition,
  uses its real executor memory and display-provider snapshot boundary, observes
  both selected pages, then resets and observes the declared low-page empty
  frame.
- The T345 owner-test inventory rises from 162 to 164 pure targets because the
  two focused owner tests are registered; the three mixed targets remain
  unchanged.

Focused command (GCC):

```text
cmake --preset mingw-gcc-x64
cmake --build --preset current-gcc --target core-machine-compaq-cecg-s28-smoke vm-model40-cecg-s28-smoke
build/mingw-gcc-x64/core-machine-compaq-cecg-s28-smoke.exe
build/mingw-gcc-x64/vm-model40-cecg-s28-smoke.exe
```

Observed markers:

```text
M5:T386:S28:CECG-ODD-EVEN-PAGE:OK
M5:T386:S28:MODEL40-CECG-ODD-EVEN:OK
```

The documentation governance and combined specialized/current-source gate passed after the full fixture and T345 inventory sweep: 280 current targets, including both S28 markers. Existing S9--S13 CECG controls remain in the current gate. The rebuilt local developer artifact is `vm-0-5-0389` / `build/output/nxvm_0_5_0389.exe`, SHA-256 `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`. Coordinator acceptance below records the independent actual-change review.

## Similar-issue sweep and transfer

The sweep covered CECG `3C2h` writer/reset declarations, planar memory
read/write offsets, copied snapshots, Model-40 composition, generic-personality
port isolation, all existing CECG tests, current-gate registrations and the
T345 strict-owner inventory. It found one owner path rather than a separate
profile workaround.

S28 consumes the sole shared functional CECG receiver. The next T386 step is
the proposal-required selected-device functional-closure audit. The retained
physical, firmware and board timing questions remain in `TODO.md` and transfer
to the DeskPro board/device timing candidate; this evidence makes no L3 claim.

## Coordinator acceptance

The coordinator reviewed P1 `7de64aca` directly: the configuration field is
carried into VADP reset state; the only page-address transform is shared by the
existing planar read/write provider and copied snapshot; the Model-40 declares
low page; and the tests exercise public memory/display routes without a
profile-local or test-only API. The fixture sweep explicitly repairs every
strict initializer, and both T345 inventory guards account for the two added
owner tests. The documented 280-target combined gate passed. S28 is accepted;
only the proposal-required selected-device functional-closure audit remains in
T386. No L3, physical, firmware or timing claim is accepted.
