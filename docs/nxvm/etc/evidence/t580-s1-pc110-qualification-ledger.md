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
Open-Source-PC110's TFT material documents a forward DSTN-to-TFT patch but
states that a confirmed unpatched DSTN video-BIOS dump is still required for
the reverse patch.  These are research/asset facts only; no external bytes or
third-party source enter this repository.

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
