# T580 S49 — Integrated FDC reset and IRQ receiver

## Mechanism and source

Baseline is `6c474e957`. S48's unchanged-BIOS observations identify the
missing integrated FDC: the guest resets 3F2 and waits for its IRQ-completion
flag, while the App declares no controller. This receiver is independent of
the retained lower-Flash/Setup gap.

Open-Source-PC110's pinned `Discovery/Pluto/readme.md` identifies U22 as
SMC FDC37C665IR, its standard FDC register view and the external dock drive;
its hash is recorded in S48. A controller exists without that removable drive.
The original SMC FDC37C665IR/FDC37C666IR datasheet is now archived externally
under `manuals-nxvm/fdc/smsc-fdc37c665ir-datasheet.pdf`, SHA-256
`6BADCD602795E02168F99EA15F84B9231DB740747BEE906A57E2D99CC5EA9606`.
Acquired on 2026-10-25 from the [Bitsavers scan mirror](https://ftpmirror.your.org/pub/misc/bitsavers/pdf/standardMicrosystems/_dataSheets/SMC_37C655.pdf);
the server basename is misleading, but the scan's cover identifies the 665IR/
666IR parts. Printed pages 43 and 72 were rendered and visually checked.

Table 19 on page 43 fixes ST3 bits 5 and 3 high; these are unused on SMC,
not mechanical-drive READY/two-sided observations. Page 72 describes the
default reset polling interrupt. Therefore the existing 765-compatible
model receives fixed asserted legacy inputs for all four unit selections,
while installed-drive mask and all media IDs remain zero. This does not
invent an external drive, READY signal from a nonexistent medium, or an
inserted disk. The exact status constants have direct manual evidence; the
overall standard-compatible SMC receiver is explicitly L2, not a full 82077/
SMC Super-I/O implementation or physical timing qualification.

## Retained implementation and boundaries

One App-owned immutable topology declares standard 3F2/3F4/3F5/3F7 ports,
IRQ6, DMA2 and the existing 33 MHz conversion axis. The same existing Board
plan publishes the FDC through its sole controller, PIC, DMA and deadline
owners. No configuration helper, controller, queue, callback, clock, public
API, compatibility fork or Shared change is added. The ordinary source and
unit-test vocabularies remain Lib Types.

The minimal historical `mypc110_base_composition_create` deliberately has no
media registry/controllers and is not the Product/BIOS receiver. It remains
bounded rather than acquiring a second media owner only for a new test.
Both fixed Product variants use the changed actual machine-construction
topology. HDC and owner INI/media behavior remain unchanged. Extended SMC
3F0/3F1 configuration, UART/parallel functions and dock hardware are not
claimed as implemented by these standard endpoints.

## Tests and rejected intermediate attempts

The existing production-construction unit asserts the copied FDC/DMA topology,
fixed compatible status inputs, zero installed drives and zero floppy slots.
It uses the actual described driver's existing Debug port operation to read
MSR=80 and release DOR reset without an inserted disk. Existing Board FDC
units retain detailed reset/SENSE and media-change ownership; App tests do
not duplicate those implementations.

The same existing real-BIOS executable extends its former timer receiver:
after all initialized-copy/ASMI/RSM/timer predicates, it observes guest DOR=08,
then 0C, the real IRQ-wait return at FC971 and the FBD07 branch reached only
after the BIOS has matched all four reset SENSE results C0 through C3.
No host writes the completion flag. Its CTest registration replaces the earlier
timer-only entry; there is no additional executable, framework or duplicate
integration run. The older timer CLI remains a manual earlier-prefix view.

The first wiring draft left legacy READY inputs zero and timed out on both
widths. This was not accepted. Source review then found the documented SMC
fixed status semantics above; the draft is replaced, not worked around by
host flag writes or increasing the runtime budget. A unit-only attempt to
issue SENSE commands with no guest-time advancement was also rejected; it
does not replace the actual guest IRQ/SENSE proof or justify another clock.

## Verification boundary

Use the final dual-width results recorded below, not the rejected timeout or
the intermediate constructor-only pass. PC110 previews remain prepublication;
this receiver alone does not qualify full POST, Setup, a boot sector, guest,
live panel/input/lifecycle, factory DSTN inputs or product release. Published
PC/MyNES/MyRaidenII artifacts and owner INI/ROM/font/media remain unchanged.

### Final results and actual change review

Both final host-width BIOS integrations report the same actual predicates:

```text
timer passed=1 failed=0 delta=166C executed=11612848
copied=1 initialized=1 asmi=1/1 rsm=1 resets=3
FDC-RESET asserted=1 released=1 irq-return=1 senses-complete=1
status=OK reason=BUDGET pc=000F7490 terminal-fault=0
```

The normal quantum ends after the semantic marker, not at the 32M containment.
Four protected-memory GP probe deliveries remain visible; they are not terminal
Core faults and are not suppressed. The final x64 integration passes in 68.74
seconds, x86 in 87.48 seconds. Final App units pass 11/11 per width; the three
existing FDC Board regressions also pass per width. No new test target is added.
Both fixed preview variants build on both widths, and a subsequent unchanged
build reports `ninja: no work to do` for each original graph. App dependency/
test-support boundary, documentation governance and whitespace gates pass.

Actual tracked source/test counts use `git diff --numstat` against `6c474e957`,
excluding documentation, generated files and artifacts:

| Path | Added | Removed | Net |
| --- | ---: | ---: | ---: |
| src/app-mypc110/pc110_plan.c | 17 | 0 | 17 |
| test/app-mypc110/CMakeLists.txt | 6 | 6 | 0 |
| test/app-mypc110/integration/real_firmware_post_integration.c | 31 | 3 | 28 |
| test/app-mypc110/unit/machine_construction_smoke.c | 19 | 0 | 19 |
| Total | 73 | 9 | 64 |

The positive delta adds the absent immutable hardware wiring and strengthens
the existing actual-consumer regression. Production adds no function, API,
mutable state, state machine or alternative execution route. The five new
booleans are monotonic test checkpoint facts only. The retained production
path is App topology -> existing Board FDC/PIC/DMA -> existing controller.
No changed Shared tree needs a manifest rewrite because no Shared file changes.

Preview identities (not qualified published products):

| Width/variant | PE | SHA-256 |
| --- | --- | --- |
| x64/TFT | 8664 | `2460B2E4C4034A7508609379FEABB63D6A42D5F416F0825FD22F0843E0233644` |
| x64/derived DSTN | 8664 | `80AFCE6B2506015BE17B14D11FF5D6FA7C4608A94443F9C8CC05AC74C941E9F0` |
| x86/TFT | 014C | `8905B60A75D2C1718EA85E2CE7DA8E9F410EB33E90BC46C61D10C965B4316EA1` |
| x86/derived DSTN | 014C | `415DC37F3B0C9C15B99C0EB45169DB40DDE8A8CD9F7782A63A83DA78181551E8` |

The files remain in the original graph's `src/app-mypc110` output directory;
no unqualified PC110 EXE enters assets. The unmodified previously published
Apps do not require binary refresh for this App-local source change.

### Task cleanup

All owned native observations have exited. Required results and the following
hashes are retained before the real cleanup tool previews/removes the task
directory, including the two source-page renderings (546,963 bytes total).
The manual original stays in its external archive; valid caches stay intact.

| Temporary file | Bytes | SHA-256 |
| --- | ---: | --- |
| smc-configure.png | 264468 | `654319D47686E69D3DEE68B20D87AFD71ABC0C7718AFB58E03B6881325C3B837` |
| smc-st3.png | 277657 | `DDA6E4D661F0760A9C3D93407045487C1777864FF143D8E34080DA753B6E128D` |
| x64-fdc.log (rejected) | 543 | `1132000D65A717C62B61653541951F55954E0C3F2B841F62C0518816F2DAC26F` |
| x86-fdc.log (rejected) | 543 | `B62C9934CC2D9DB464F7CB5C39AA3BEC1FBBBA5E6FB1B62D4B225CF1F149EA85` |
| x64-final.log | 1876 | `4AAD58275C664BDFD7B68DF09C0CFECD6C6476E1608BA315C5E80E9C5F9406A4` |
| x86-final.log | 1876 | `39D626CB882F98916C8CD0979AE04B6D7D75713251856D0EEC84458B0EE3939D` |
