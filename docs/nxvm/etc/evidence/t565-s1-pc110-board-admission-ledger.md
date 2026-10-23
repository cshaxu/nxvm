# T565 S1 — PC110 Board And Firmware Admission Ledger

## Scope and baseline

This is the T565 admission baseline, not a hardware implementation or boot
claim.  It rechecks the current repository at `330cdefb3`, the pinned PC110
research repositories and the owner-managed asset root before any PC110 source
tree or artifact exists.

| Input | Current identity | Disposition |
| --- | --- | --- |
| Open-Source-PC110 | `02ccf2e8e19b298a2cf04c5d430278eb087cc368`; one unrelated untracked `PCB/RAM-16MB.zip` remains untouched | Research/reference only; no source is imported. |
| PC110-EMU | `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`, clean | ROM role and functional-reference lead only. |
| PC110-QEMU | `bae390ebf21665ec97860d28e26ec46f1fe8f4e9`, clean | GPL reference only; no code or model is imported. |
| `nxvm-assets/profiles-nxvm` | Only existing XT/AT/DeskPro/default-PC and display profiles | No PC110 asset directory or approved build input exists. |

The external PC110-EMU checkout contains the previously catalogued candidate
bytes, but they remain outside the approved NXVM asset archive:

| Candidate | Bytes | SHA-256 | Current role |
| --- | ---: | --- | --- |
| `pc110_bios.bin` | 262144 | `232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312` | TFT-identity candidate only; not an admitted release master. |
| `MSM538032E@SOP44.BIN` | 1048576 | `9829fdb8281c12022dc3b77686044ed1a5213ab526ce4329f2841cd64171784c` | Font-ROM research candidate. |
| `M38223E4HP@QFP80.BIN` | 16254 | `96c6e37cfa52f30b303db70c2036cbf21e6e1bb638c5eb11343ab161db3c9cc0` | Power-MCU research candidate; address/vector contract unresolved. |
| `M38813E4HP@QFP64.bin` | 16255 | `b29761a2fd39abd9c9419ca73b03beb6c41bc52102e2a7429d1f023f82a2a2b8` | Keyboard-MCU research candidate; protocol/execution choice remains with input/power. |

This confirms, rather than changes, the existing T558 source-policy boundary:
raw third-party ROMs are not copied into this repository, and availability in a
reference checkout is neither distribution permission nor firmware-pairing
admission.

## Board and firmware batch

| Row | Evidence grade and owner | S1 disposition | Required next proof / receiver |
| --- | --- | --- | --- |
| BRD-1: population and construction root | Current source has no `app-mypc110`; T558 ownership audit assigns fixed board/firmware/panel bindings to that future App. Open-Source-PC110 identifies U61 VL82C420FC5, U21 Bowman and U35 Pluto, but mixes reconstructed and observed material. | One future `app-mypc110` composition is the sole root; no AT descriptor flag, second executor or generic device framework is admitted. | Board S2 must freeze the exact construction lifetime and supported population before adding source. |
| BRD-2: cold/warm reset, RAM and ROM/shadow map | VL82C420 material is a reconstructed/patent/reverse-engineering source; current Core has no PC110 chipset model. Post-POST dumps are not reset defaults. | Blocked from implementation as a reset-value table. | Board S2 must select a bounded contract with direct map/reset tests and label any reference model L2. |
| BRD-3: decode, A20 and standard IRQ/DMA/PIT/RTC routing | Current Core owns the neutral executor and existing standard controllers, but its AT preparation accepts only its fixed BIOS roles. | Reuse is possible only through one App construction route; no PC110 BIOS service, host interception or duplicate controller state is admitted. | Board S2 must prove each routing/decode attachment before using it; storage/input/display receivers own their endpoints. |
| SIO-1: configuration entry/exit | SMSC FDC37C665IR documentation identifies the `3F0h` unlock/index/data protocol; no PC110 implementation exists. | A future chip owner may model the configuration state once; App owns its PC110 decode. | Board S2 chooses the sole configuration/decode owner; no endpoint task duplicates it. |
| SIO-2: mode-dependent FDC/IDE routing | Component documentation and T558 checklist establish the potential conflict at `3F0h/3F1h` and `3F7h`; exact fitted route remains separate. | Not implemented and not allowed to become overlapping always-on handlers. | Storage receiver attaches actual FDC/ATA endpoints after Board establishes configuration ownership. |
| SIO-3: reset defaults/preservation | The documented SMSC default/bit-column conflict remains unresolved for the selected board revision. | Blocked: neither blanket zeroing nor blanket preserve-all is admissible. | Board S2 requires a bounded reset-reason contract or transfers the unresolved revision decision. |
| Firmware pairings | The identified BIOS candidate is TFT, while an unchanged stock DSTN master is not present in the owner asset archive. The documented offline reverse-panel route is only a preparation candidate. | Four executable identities and two compiled pairings remain design facts, not build-ready inputs. Runtime INI selection and cross-variant fallback remain forbidden. | A later board/firmware preparation S must receive an explicit asset/provenance admission, preserve master/derived hashes externally and prove the panel pairing before build embedding. |

## Similar-issue sweep

The four current App roots remain independent: each links inward to Core and
has its own fixed firmware construction; none imports a sibling App. The
existing AT preparation helper is not a PC110 construction contract, and the
current source has no `pc110` production/test root, target, artifact or
runtime model selection. The only identified documentation cleanup is the
already recorded My5170 historical `ibmpc` comment; it is unrelated to PC110
admission and is not modified here.

## S1 conclusion

The batch is finite and has one owner path, but no firmware payload is yet an
approved NXVM build input and several private chipset/reset facts remain
unresolved.  This does **not** block later source-backed board construction
research; it blocks firmware embedding, runnable EXEs and every boot claim.
The next T565 S must first choose a limited construction/decode batch from the
rows above, with the unresolved firmware/reset decisions kept explicit.
