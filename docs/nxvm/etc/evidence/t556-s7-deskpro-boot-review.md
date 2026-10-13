# T556 S7 DeskPro Boot Review

## Status And Scope

Work-in-progress evidence, not S/T acceptance. Shared six trees are unchanged.
Core owns memory mapping, FDC reset configuration and video state; DeskPro
owns its D4 selection and fixed board reset values. Firmware, media and INI
inputs have not been changed. No automatic key injection is introduced.

## External Cross-checks

Behavior reference only; no third-party source is copied or transliterated.

- PCjs revision `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`,
  `machines/pcx86/modules/v2/ram.js`, describes the D4 mapping register at
  80C00000h and FE0000h RAM alias into E0000h--FFFFFh.
- The same revision's `fdc.js` initializes the controller rate to 500 kbit/s.
  Its DeskPro notes describe deliberately probing absent sector 16 on 15-sector
  media. `chipset.js` performs device reads during DMA verify. NXVM already
  calls its device read provider for verify; that mechanism is not replaced.
- PCjs `chipset.js` initializes the 8042 output port with NO_RESET and A20_ON.
  DeskPro now selects the corresponding existing reset-output configuration;
  other board configurations retain their existing defaults.
- 86Box revision `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`,
  `src/video/vid_ega.c`, disables Compaq video mapping when Miscellaneous
  Output bit 1 is clear. This independently contradicts NXVM's old polarity.
- PCjs's published DeskPro EGA configuration uses IBM EGA and explicitly
  describes Compaq EGA support as not fully tested. It is not end-to-end proof
  of NXVM's CECG scanout implementation.
- PCjs also records a historical workaround that selected a 1.44 MiB drive
  to avoid a failed 1.2 MiB drive-type probe. Its own comment calls this
  papering over the missing DMA-verify error. NXVM does not adopt that
  configuration workaround or treat that reference machine as equivalent to
  the selected Model40/CECG/1.2 MiB configuration.
- VirtualBox documents a generic x86 virtual platform, rather than a DeskPro
  motherboard. Its successful DOS boot is not proof of Compaq D4 or CECG
  behavior; it is only a possible reference for generic device contracts.
  Reference: Oracle VirtualBox 7.0 User Guide, Configuring Virtual Machines,
  https://docs.oracle.com/en/virtualization/virtualbox/7.0/user/BasicConcepts.html.

## Observed Boot Proof

On 2026-10-13 the actual Model40 INI boot-matrix receiver passed once per host
width in the original persistent Ninja trees:

- x64: 145.58 seconds, `NXVM:INI-BOOT:NXVM.ini:installer-running`.
- x86: 189.44 seconds, the same marker.

The marker matches the captured guest text `Setup is determining your system
configuration`. It proves this checkpoint, not completed installation or manual
desktop input/rendering qualification. No F1 or other guest input was injected.

## Review Cleanup And Remaining Gates

The temporary DeskPro-specific diagnostics and private board-layout access
have been removed from the generic Core boot harness. DeskPro-specific
observations remain in its existing diagnostic tool. App FDC configuration no
longer includes the private Core FDC header. The text-page regression now also
checks an adjacent cell and the next row, not just a single character.

The historical T386 evidence retains its original record with an explicit
polarity correction instead of rewriting its old acceptance claim.

Both complete configured build graphs pass after cleanup. The first x64
546-unit run passed 542 and exposed four stale chip-test contracts:
`video_contract`, `compaq_cecg_contract`, `compaq_cecg_odd_even_page` and
`ega_planar`. Their setup still cleared RAM-enable for Compaq, or expected
control-register bit 2 to be forcibly cleared. These assertions/setups were
aligned with the repaired behavior; no production change was made in response.
All four then pass on x64. Temporary failure-line instrumentation was removed.
Core source/test manifests and the documentation governance gate pass.

The final configured 546-unit runs passed 545 on x86 and 544 on x64. The
remaining results were timeouts: `lib.test-types-boundary-selftest` on both
widths and `emulator.verifier-negative` on x64. An isolated rerun, without
changing test code or timeout budgets, passed all three: 12.64 seconds on
x86 and 14.22/30.74 seconds on x64. Thus each configured receiver has passed,
but neither complete parallel run is described as all-green. The additional
x86 neutral-memory-alias regression passed after rebuilding its target.
Both Core manifests were verified again after that test update. Shared six
trees still have no working-tree changes.

The x64 direct diagnostic was run with `--turbo
--no-retirement-observation`, without the optional F1 injection. It exited 1
without an installer marker: 35,492,996 instructions, final PC FC717h, text
`02048 KB OK`, D4 control FCh and FDC phase 0. This is not a passing boot
qualification and has not yet been reconciled with the successful normal
worker route. Retirement-based counters in this run are disabled, so their
zero values must not be interpreted as proof that those paths never execute.

Inspection identified two diagnostic qualification mismatches: its default
wall budget was still 90 seconds, below both measured normal-worker boots,
and its success cases omitted the normal receiver's `installer-running`
checkpoint. The diagnostic now uses the owner-approved 300-second budget,
recognizes that same literal captured text, and reports a wall timeout even
without detailed tracing. No guest execution or input policy changed.
The rebuilt x64 diagnostic then exited 0 with `installer-running`, captured
MS-DOS 5 Setup text, and 57,025,668 executed instructions. It used neither
F1 injection nor modified firmware/media. The rebuilt x86 diagnostic also
exited 0 at the same text checkpoint with 57,021,572 executed instructions.

## Actual-change Review And Similar-issue Sweep

The production delta is 137 added / 26 deleted lines (net +111); tests and
the existing App diagnostic are 211 added / 34 deleted (net +177), excluding
manifests. No extra renderer, worker, firmware patch, automatic input or
shared-component path was introduced. The new alias writability state is a
`lib_bool` in the existing memory mapping; the bounded setter only changes
an installed mapping. Board reset values belong to the DeskPro composition.

The sweep searched `compaq_cpu_video_memory_disabled`, `initial_ccr` and
`set_memory_alias_decode` through Core and the affected App/test trees.
The other CECG assignments are explicit configure/reset values, not another
register-bit polarity decoder. Ordinary FDC initialization remains 250 kbit/s;
only the selected DeskPro opts into 500 kbit/s. The XT positional initializer
adds explicit defaults with no behavioral change. Core and DeskPro tests
cover alias write protection/selection and FDC defaults. Logical and raster
text share the same CECG page-address helper; graphics is unchanged.

All eight PC artifacts were rebuilt and PE-width verified in the existing
Ninja/ccache trees. The final DeskPro hashes are:

- x64: `93F466881AC2B82C9651D41195A54FDDF6B6F43DB48AEF514738C11BB887B781`.
- x86: `DAC7BDDB814002B04D9645910A19F0F424E530E805517B0143BE9505E06DD0A8`.

MyNES source, media, configuration and release binaries are unchanged. Its
unit targets were rebuilt separately in the existing unified trees for the
repository regression: 45/45 passed on each width (22.31 seconds x64,
32.15 seconds x86); no MyNES release target was built.

Documentation governance and `git diff --check` pass. Implementation/artifact
P1 `307cf1bd6` is pushed to origin/master. Owner acceptance is pending.
Whole-task integration qualification belongs to S8. Manual desktop and
physical-hardware verification have not been performed or claimed.
