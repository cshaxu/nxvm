# T539 S9: XT PPI And Keyboard Extraction

Baseline 625ea7054. The [boundary review](../architecture/t539-s9-xt-boundary.md)
defines this automatic admission. Shared and NXVM changes have separate P
targets; no MyNES or sibling repository change belongs to this batch.

## Original Responsibilities And Cases

- Original `xt_keyboard.c/h` becomes one opaque `x86/devices/xtkeyboard`
  implementation. FIFO, serial frame, BAT and reset qualification have one
  owner; the old pair and its concrete PPI dependency are removed.
- Original PPI control/latches move to opaque `x86/devices/ppi8255`. The
  qualified Mode-0 model preserves original mode-set latch retention and
  inactive Mode-1/2 disposition, explicitly not full silicon qualification.
- NXVM's remaining `xt_ppi_keyboard.c/h` owns ports, DIP/parity/NMI, speaker,
  receiving latch and IRQ1. It samples copied PPI pins and connects the
  keyboard through line/release/byte-acceptance callbacks, not private state.
- Machine construction owns both lifetimes; reset clears state but preserves
  connections; destroy withdraws IRQ before destroying the receiver. Four
  range-selected durations are converted at composition with overflow-safe
  quotient/remainder ceiling arithmetic. Their L2 classification is unchanged.
- The original board smoke retains DIP selection, BSR, repeated PA reads,
  PB7 acknowledgement, IRQ1, parity/NMI, AT isolation, reset threshold and
  BAT ordering. FIFO-private assertions are replaced with all 16 delivered
  bytes, including the terminal FF overrun character.
- Standalone PPI coverage checks 128 control encodings across all three ports,
  all eight BSR bits, reset, invalid selectors and write-only control.
- Standalone keyboard coverage checks scan/BAT refusal versus reset/release,
  259+1 frame timing, inhibit, FIFO saturation/whole-sequence rejection,
  disabled timing and invalid construction. Tests use only public interfaces.
- Boot diagnostics retain actual I/O and use copied PPI pins and a deadline
  instead of private mode/BAT counters. No diagnostic-only API is introduced.

## Negative Controls And Similar-Issue Sweep

The following failures were reproduced before their repairs, not inferred
from passing pre-existing tests:

1. Refused serial completion could decrement zero remaining bits to 255.
   Completed data now waits without another bit deadline; release retries
   the same byte. Both scan and BAT, reset and release are covered.
2. Each of eight failed PPI route allocations rolled back but returned the
   cleared status. Saving the failure before rollback preserves NO_MEMORY;
   tests prove unrelated routes survive, new routes disappear and retry works.
   The source sweep found no identical remaining rollback/read-status form.
3. Clock hold stopped serial advance but left a non-progressing deadline.
   The query now suppresses that deadline until release, retaining the interval.
4. Restoring output direction without another PB write left a refused byte
   waiting. The adapter republishes existing line levels through its one
   observer when the empty receiver becomes available; scan/BAT tests cover it.
5. BAT finishing under clock/clear inhibition left no send deadline on release.
   Both negative cases now pass through the same serial-start function as
   scans. No extra queue, timer or polling fallback was added.

These are mechanism/ownership repairs, not new physical timing claims.
The range-selected timing and qualified PPI limitations remain explicit.

## Verification And Delivery

Final-source full units pass 347/347 per width; default external integration
passes 20/20 per width. Independent tools-off chip suites pass 18/18 per width,
including corpus, manifest and negative boundary checks. All six manifests,
the specialized static aggregate, documentation governance, local links and
whitespace checks pass. The migrated boot diagnostic compiles on both widths.
Intermediate passes before the last BAT repair are not used as final proof.

Remaining-profile boots each pass once: XT 21.54s/26.45s, AT 40.55s/50.55s,
Model 40 80.15s/96.10s (x64/x86). Both reusable build trees are restored to
default configuration. This is one successful matrix, not a repeated
reliability claim. Current alone owns acceptance status.

Shared P1 0f9c6b1a8 and NXVM P2 31e759965 are pushed to origin/master.
Coordinator-role actual-change review accepts the chip/adapter boundaries,
original-case mapping, refusal/rollback/line-order fixes, deleted old paths,
build and diagnostic migration, six manifests and eight artifact identities.
No item remains in the bounded S9 brief. Its packet is removed; T539 remains
open for the remaining finite inventory, with FDC next under automatic admission.

Production C/H totals +477/-371 (net +106); tests +312/-18 (net +294), including
new files. The added production responsibility is opaque lifetime/interfaces
and board attachment, not a second implementation or device framework.

## Artifact Identity

Eight optimized Release 0.5.0539 artifacts have expected PE 8664/014C machine
IDs and no compiler-debug sections. Runtime Debug remains available. Owner
INI contents, Lib/Common, MyNES and sibling repositories are unchanged.
The two new chip targets are not MyNES executable inputs.

| File | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 15F92EEDE27EF4E2F7B029637264A63509D851BD42293D2822B859BFEF1EF95D |
| nxvm_model40_0_5_0539_x86.exe | 37D639BD1CA9CF6A3EE35337F55E64D7FCD32482687728373429571B8CBA0CBC |
| nxvm_default_0_5_0539_x64.exe | B4DCE3C4D6047F8A22CBBE749064404AC31CBCC72ACD1CEE50F0638BDCADD5F8 |
| nxvm_default_0_5_0539_x86.exe | EBF5FA5FB59085D98724FA9C1464A698FEE572DDFC0CAD7603A956B562CBD88B |
| nxvm_xt_0_5_0539_x64.exe | 14F4C6F5442EB2A2A4FDD18555461EDE7AFCC775FC731EFF3A680F00015FC949 |
| nxvm_xt_0_5_0539_x86.exe | C6F9B82D1107A3FCAEA1BA609FA54CFADBD9359DE88447F7E3587C744CE443E1 |
| nxvm_at_0_5_0539_x64.exe | DFECCDDEF40CDC0389B591614F9F9C00B43DB499D8E10E41B96626B83040ED69 |
| nxvm_at_0_5_0539_x86.exe | 61221BB368836879E4F52E44F7E24092AC14399B41B61F8AD8742B8216196F64 |
