# M5 T580 S1 — MyPC110 qualification ledger

## Frozen product universe

The product target remains four build-fixed artifacts, not runtime-selected
models:

| Member | Current input/status | Owner/disposition | Proof required |
| --- | --- | --- | --- |
| TFT x64/x86 | 256 KiB BIOS and 1 MiB font exist under external `ibm-pc110-tft`; hashes match the admitted manifest. | MyPC110 App S2 composition/build receiver. | Construct/destroy and product lifecycle first; firmware progress separately. |
| Original DSTN x64/x86 | No verified original BIOS/font input exists in the external asset root. PC110-EMU's only ROM hashes to the admitted TFT image. | External BYOB/source gate. | Lawful original input, manifest identity, then the same App product route. |
| Product entry/binding | Missing: `src/app-mypc110` contains only board/firmware diagnostics. | MyPC110 App S2. | One normal `x86-nxvm/product` binding and lifecycle test. |
| INI/runtime identity | Missing: no `mypc110.ini` template, selected profile or App artifact declaration exists. | MyPC110 App S2. | App-selected adjacent INI path, without runtime model/ROM choice. |
| Real firmware progress | T579 proves the bounded port-61/LPT/RTC cycle only. | Later finite device receivers. | Named firmware checkpoint, no synthetic BIOS path. |
| Display/panel | F65535 foundation and font route exist; full panel/graphics presentation is unqualified. | Later display receiver. | Source-backed mode/panel capture and actual output proof. |
| Storage/guest corpus | No admitted PC110 floppy/HDD/PCMCIA boot route or lawful guest corpus. | Qualification receiver after hardware source gate. | Explicit medium and finite guest checkpoint. |
| Input/power | Keyboard/TrackPoint substrate exists; power/APM remains deliberately unmodeled. | Existing App route plus later power receiver. | Lifecycle/input proof without host power fabrication. |

## Asset reconciliation

`O:/repos.hobby/nxvm-assets/profiles-nxvm/ibm-pc110-tft` contains the admitted
TFT system BIOS (`232101c8…5232312`) and Kanji font
(`9829fdb8…71784c`).  `PC110-EMU/Roms/pc110_bios.bin` has the same BIOS hash.
S13 also checked every full firmware candidate in PC110-EMU, PC110-QEMU and
Open-Source-PC110. The latter repository's
`Mods/LBA-BIOS-Patch/PC110_BIOS_lba.BIN` is distinct
(`6c859ad2…2bec1d`) but its own README identifies it as an LBA-modified image
which has never been run. It is not original-DSTN input. Its TFT material
documents a forward DSTN-to-TFT patch and explicitly states that a confirmed
unpatched DSTN video-BIOS dump is still required for the reverse patch.
Accordingly, no discovered image supplies the original member. These are
research/asset facts only; no external bytes or third-party source enter this
repository.

## Current code reconciliation

`mypc110-board` currently constructs a 486SX, Core/X86-NXVM board plan,
firmware map, F65535 route, font route, RTC, 8042 input substrate and the
bounded LPT model.  It is not connected to `x86-nxvm/product` or an App
executable.  The four existing PC Apps demonstrate the intended direction:
each App supplies a fixed product name, adjacent INI name, machine binding and
build-selected firmware; `x86-nxvm/product` owns the common parser/lifecycle.

## S4 C000 checkpoint disposition

The TFT BIOS's selected C&T video image is now an immutable instruction source
for the App-local C000 route while ordinary data reads remain the writable
shadow required by copied BIOS tables.  The real image crosses the former
`C000:6000` width-sensitive port boundary on x64 and x86.  F65535's finite
input-status reads and the two traced Core port-width defects are accepted
receivers with regressions.  This is a progress checkpoint only: it does not
qualify POST, panel output, Easy-Setup, storage, guest boot or the unavailable
DSTN member.  S5 owns classification of the next bounded post-C000 trace.

## S6 post-C000 port-61 disposition

The subsequent real-TFT sample observes alternating port-61 bit 4 values after
the C000 return on both host widths. The existing elapsed-tick source therefore
remains the owner and no new PIT, F65535 or Core receiver is justified. The
firmware is still in its own delay body; the exact sample and its limits are in
[the S6 evidence](t580-s6-post-c000-port61-sample.md). This does not advance
any display, panel, Setup, storage or guest qualification row.

## S7 post-C000 delay-return disposition

The same post-C000 real-TFT path enters `F000:EA86` with `ECX=040Bh`, executes
the delay routine's `F000:EA92` return and resumes at its caller's
`F000:EA84` epilogue on both widths. The port-61 polling body is therefore a
completed BIOS delay, not a missing hardware receiver. The exact bounded trace
is in [the S7 evidence](t580-s7-post-c000-delay-return.md). This opens no
display, panel, Setup, storage or guest qualification row.

## S8 display-receiver disposition

Research now identifies the first display receiver without treating a
host-rendered front panel as machine behavior. The F65535's standard VGA
mode-12 (`640x480x16`) copied scanout is absent from the current component,
while the existing X86-NXVM snapshot/frame path can already carry the required
indexed pixels. S9 owns that bounded L2 receiver; front LCD/power, physical
panel timing, original-DSTN input, storage and guest rows remain separate. The
source grades and boundaries are in [the S8 evidence](t580-s8-pc110-display-receiver.md).

## S2 boundary

S2 may add the missing **TFT-only** App entry, binding, profile declaration,
INI template and focused composition/lifecycle tests.  It must use the one
existing X86-NXVM Product path; it may not publish a PC110 EXE, claim real BIOS
boot, add a model selector or manufacture the missing original-DSTN asset.

## S44 receiver reconciliation

The earlier inventory above records its admission baseline. At S44 the same
frozen members have these dispositions; no product requirement is removed.

| Member | Current proof and remaining receiver |
| --- | --- |
| TFT x64/x86 | Fixed App, embedded inputs, composition units and previews exist. Both widths pass the unchanged BIOS timer calibration; later POST and release qualification remain open. |
| Original DSTN x64/x86 | A separately labelled, panel-table-derived preview pair exists. It is not a recovered factory dump or independent DSTN guest/panel qualification. |
| Product entry/binding | Both fixed variants use the normal X86-NXVM Product/Emulator route. Full live lifecycle/UX and release acceptance remain required. |
| INI/runtime identity | One adjacent App-selected mypc110.ini exists and is preserved; it does not select ROM or panel at runtime. |
| Real firmware progress | S43 locates the PIT/refresh calibration failure; S44 repairs its App clock relationship and proves delta 1668 on both widths, within the BIOS range. Remaining POST/Setup/boot milestones remain with T580. |
| Display/panel | F65535/font/graphics mechanisms exist. Actual panel output and independent TFT/DSTN display qualification remain open. |
| Storage/guest corpus | App ATA wiring exists; boot media, real guest progression and required storage/PCMCIA coverage remain open. |
| Input/power | Existing keyboard/pointer construction remains. Live input, Fn/inking and power/APM coverage retain their receiver/source prerequisites. |

See [the S44 clock proof](t580-s44-pit-refresh-clock.md). A timer calibration
pass advances only that firmware substage, not the final product predicate.

## S47 receiver reconciliation

The cold-initialization member is still open. The approved write-width
prerequisite and App configuration protocol now have actual CPU route proof;
both host widths retain timer calibration success. Complete CPU-only SMRAM
decode, Board-to-App SMIACT wiring, initializer predicates and trigger/acknowledge
remain one receiver batch, not accepted subcomponents of an assumed boot.
The [S47 record](t580-s47-cold-initialization-contract.md) identifies the missing
contracts, source grades, exact test results and receiving artifact hashes.
All other frozen qualification rows retain their S44 dispositions above.

The later S47 decoder receiver replaces prefilled C/F and the unproven F9
hard gate with register-qualified shadow access. Both widths now execute the
actual initializer, handler copy and return; the existing timer integration
requires those markers and passes. This disposes the previously skipped
initializer predicate as repaired, not the complete cold-memory/SMRAM member:
CPU-only isolation, Board SMIACT forwarding, trigger/acknowledge and RSM are
still open. The boot-only lower Flash bank view also remains unqualified.
No other frozen product row changes disposition and no PC110 EXE is published.

The final S47 active-view continuation implements the remaining cold-receiver
members: one CPU-only SMRAM overlay, Board-to-App SMIACT, guest preparation,
independent configuration locks, source-qualified ASMI, W0C acknowledgement,
rearm and real RSM. Both widths execute the registered vendor service and
return before passing timer calibration with delta 166C. Core/chip/App units
and all 712 repository unit cases per width pass; receiving artifacts rebuild.
Shared implementation is `794380f69`; the App, evidence and PC receiving
artifacts are delivered by NXVM `782f09ebb`, with MyRaidenII `5ed407b4b`.
Coordinator actual-change review accepts this cold-receiver batch. The boot-only lower Flash
view and all later POST/Setup/guest/panel/product members remain unqualified;
none is hidden by accepting this one cold-initialization receiver.
