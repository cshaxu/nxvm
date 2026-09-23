# T386 S6: Compaq Enhanced Color Graphics Personality

`M5:T386:S6:COMPAQ-EGA-PERSONALITY:OK`

## Scope And Provenance

This S implements only the selected Compaq Enhanced Color Graphics Board plus
Compaq Color Monitor digital personality around the existing shared VADP owner.
It derives hardware facts from the Compaq *Enhanced Color Graphics Board / Color
Monitor Technical Reference Guide* (December 1986), consulted as transient
research under the source policy. No guide text, ROM, firmware, guest media,
third-party source, local path, or binary is retained in Git.

The guide establishes an EGA-compatible board with 256 KiB video RAM, sixteen
simultaneous colours from a 64-colour digital palette, and the selected colour
monitor. It documents read status addresses `7C6h` (Environment), `BC6h`
(Display Type), and `FC6h` (Initial Mode). For the selected external Compaq
Color Monitor without an internal monitor, `BC6h` reads `30h`; selected EGA
initial mode reads `01h`; the EGA-supported Environment bit reads clear.
The six palette bits are named `r g b R G B`, bit 5 through bit 0.

## Owner Contract

`core/machine/vadp` remains the one mutable owner of EGA planar memory,
sequencer, graphics, attribute, CRTC, ports, reset and copied snapshot. The
new `core_machine_vadp_ega_personality` selector defaults to `GENERIC`; thus
existing generic/IBM EGA paths preserve both their ports and RGBI palette.
`COMPAQ_ENHANCED_COLOR` is an immutable core display declaration, not a VM
profile, catalog entry, firmware binding, or test-only route.

When selected, the same VADP registers the three source-backed read-only
identity/status leaves and maps all six palette bits into the existing digital
RGB snapshot range. Each primary/secondary component pair expands to 0, 55,
AA or FF. This is capture normalization, not a claim about analog monitor
signals or phosphor output. Reset preserves the declared personality and hence
the three selected identity reads. No second VADP, IBM-derived Compaq default,
or generic behavior mutation is introduced. VADP commits the Compaq personality only after the shared EGA sequencer/controller declarations succeed; invalid personality input leaves the adapter eligible for a later valid declaration.

## Focused Proof And Controls

`core-machine-compaq-ega-s6-smoke` proves:

- generic VADP does not own the Compaq status port, while the selected
  personality owns all three leaves and returns `00h`, `30h`, and `01h`;
- the shared mode-10 planar path produces palette index 15 and maps its
  Compaq `21h` palette entry to digital RGB `5500AAh`;
- personality status reads survive reset.

The following generic controls still pass unchanged:

- `core-machine-ega-controller-port-smoke`;
- `core-machine-ega-planar-port-smoke`;
- `core-machine-ega-mode10-contract-smoke`.

The new smoke is registered in the current gate. The audited strict owner-test
inventory therefore moves from 139 to 140 pure targets (three mixed targets
unchanged). The direct current-gate count is 256.

The specialized T344 fixture-shape gate exposed pre-existing inventory drift:
`HEAD` already had 70 `core_machine_create` test files, while its retained list
still asserted 67. The three unclassified, existing T386 owner smokes were the
second-PIT S3, D4 platform S4, and Compaq HDC machine S5 tests; S6 adds no
such constructor. This S reconciles only those exact existing entries, so the
mechanical inventory again reports 70 direct constructors, 22 shared tails and
48 retained shapes. No test setup or runtime behavior changes. The related T345
ownership verifier likewise still expected 138 owner tests and 136 S2 targets,
whereas its generated matrices contain 143 and 140. S6 contributes one to both
actual totals; the remaining historical drift is reconciled with the same
matrix-backed update. No ownership class, compile policy, or runtime behavior changes. The verifier success report now interpolates those validated counters
rather than retaining stale literal values.

M5:T386:S6:COMPAQ-EGA-SWEEP:OK

## Similar-Issue Sweep And Transfers

Searches across `src/core`, `src/vm`, tests, CMake, and T384/T386 material
found the shared VADP palette/port owner, the generic default profile/session
copy path, and the declared Model-40 display receivers. The shared owner is
repaired once here. `src/vm` remains intentionally unchanged: S7 alone may
compose an accepted Model-40 private carrier, external-ROM provider and selected
CECG contract; it may not expose a YAML/catalog profile early.

The CECG Control Mode `3C6h` dynamic configuration, light-pen latches/status
multiplexing, monitor/switch selection, option-ROM/BIOS mode programming,
physical monitor/cable signalling, raster/service duration, ISA availability
and board waits are not determined functional behavior in this S. They transfer
to the DeskPro board/firmware receiver recorded in `TODO.md`. IBM EGA retains
its separate current-product device-capability L3 receiver. This evidence makes
no runnable Model-40 or L3 claim.