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

## S2 boundary

S2 may add the missing **TFT-only** App entry, binding, profile declaration,
INI template and focused composition/lifecycle tests.  It must use the one
existing X86-NXVM Product path; it may not publish a PC110 EXE, claim real BIOS
boot, add a model selector or manufacture the missing original-DSTN asset.
