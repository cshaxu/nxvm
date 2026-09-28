# T539 S4: RTC Extraction

Baseline 803c9d019. The owner authorized automatic admission of subsequent S
batches on 2026-09-28. This batch consumes only the two RTC files in the
[finite ledger](t539-chip-migration-ledger.md); it does not qualify every
undocumented RTC behavior or close T539.

## Ownership And Preservation

The opaque Types-only `x86/devices/rtc146818` owns the sole calendar, phase,
register/RAM, IRQ level and SQW mechanism. NXVM owns index/NMI latches, clock
conversion and timing provenance, PIC routing, seed and checksum. The old RTC
source/header are removed, not wrapped. Board construction rolls back installed
ports if chip allocation fails; destruction releases IRQ before PIC teardown.

Thirteen retained function bodies match the baseline after normalizing only
the component prefix, Types max constant and same-width boolean return types:
encode/decode, hour encode/decode, divider predicate/frequency, periodic
frequency, second increment, alarm predicate/preview, UIP, phase conversion and
advance. Remaining changes are the opaque allocation/lifetime, direct register
boundary, callback instead of PIC mutation, bounds containment and distinct
no-event result for a stopped divider. Calendar/waveform formulas and selected
profile timing grades are not rewritten.

IRQ callbacks report level changes; C reads acknowledge flags and release the
line. Reset retains calendar, phase and RAM, and releases delivery. Tests use
architectural reads or the board-owned IRQ source, never private chip fields.
Failure diagnostics omit C rather than acknowledge an interrupt merely to print
state. No public test-only peek or state-dump interface was introduced.

## Malformed-Month Safety

The baseline guest register sequence programs month 00h and time 23:59:59,
then advances one second. Compiling the original RTC with GCC
`-fsanitize=bounds -fsanitize-undefined-trap-on-error` exits with Windows
illegal-instruction status -1073741795 at the bounds trap. The temporary
baseline probe used inert PIC sinks, not a replacement calendar, and is removed.

The one `rtc_days_in_month` owner now checks the decoded month before indexing.
For out-of-domain dates it keeps the programmed value and uses a bounded
31-day containment result. This is not a hardware-precision claim. The same
helper protects actual advancement and the copied alarm preview; all valid
month/leap computations remain unchanged. The permanent contract regression
exercises all 256 byte inputs in each of BCD and binary modes through both
paths, and the instrumented fixed executable exits zero.

## Verification

- Independent chip suites with tools disabled: x64 and x86 each 10/10,
  including six runtime cases and four manifest/corpus/negative checks.
- Final full NXVM units after diagnostic cleanup: x64 and x86 each 339/339,
  21.29 and 23.34 seconds respectively.
- Default-profile external integration: x64 and x86 each 20/20.
- Specialized static build completes all 93 steps; six manifests pass.
- Non-default INI boot matrices pass once each: XT x64/x86 15.53/18.90 seconds,
  AT 28.89/36.34 seconds, Model 40 57.74/65.07 seconds. Together with default
  integration, all eight profile/width combinations reach their existing
  DOS/installer checkpoint. There is no repeat-three-times requirement.

Three original pure calendar/phase/alarm cases move into test/x86. IRQ8/PIC,
port/NMI/seed, clock, reset and construction rollback cases remain NXVM-owned.
The source sweep covers RTC lifecycle names, selected-register access, private
calendar/register fields and build references throughout src/test/cmake.
The boundary gate rejects a second old RTC implementation and App/PIC policy
inside the shared chip; negative probes reject Common and private peer edges.
Lib/Common and MyNES have no diff; MyNES has no x86 link dependency and is not
a receiving product. Sibling repositories, external assets and INIs are untouched.

Counted source/test delta against 803c9d019: +913/-791, net +122 across 28 C/H
paths. Method: tracked C/H numstat plus the five new C/H files before staging;
delete/add counting preserves total line delta irrespective of rename detection.
The increase provides the opaque interface/lifetime and exhaustive safety test,
not a second calendar or device framework. Architecture/coding governance
kept signal wiring outside the chip and retained cohesive original handlers.

Commands used the existing dual-width NXVM trees for `run-unit-tests`,
`run-integration-tests` and `verify-current-specialized-gates`; standalone
`test/x86` configured with `X86_BUILD_TOOLS=OFF`. Six manifests use the
existing Common verifier with each explicit corpus root. The final documentation
gate and diff check pass. Temporary S4 probe/artifact/chip build trees are
removed after acceptance; the two NXVM incremental trees serve the next batch.

## Receiving Artifacts

Eight Release 0.5.0539 EXEs are deployed under `assets/nxvm/<profile>/`.
PE Machine is 8664/014C as named; all contain the expected version and have
no .debug/.zdebug/.stab sections. Runtime Debug is retained. The source mapping
is S4 Shared P1 plus NXVM P2; their commit identities accompany acceptance.

| File | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0539_x64.exe | 8531A893E01630FE3E98A109596355BDA85510F9B3BD81ED4B468519B0E0131B |
| nxvm_xt_0_5_0539_x86.exe | 04434DB901252A2D63D8E62BD161D20B5C3C1334A5E68BD28E3DC3243626FA7F |
| nxvm_at_0_5_0539_x64.exe | A7FB21ACBB77B696814DFD64F9C5DB5613AA619E542E2B34B27D6F461A63B78E |
| nxvm_at_0_5_0539_x86.exe | F2991744B71789CDE76E3EBCD40E5EC16AAC6FFFE0D7B76592BECE0C903A58BA |
| nxvm_model40_0_5_0539_x64.exe | 3B2F29B87B0777117080A187285F66476FA094A1E4B1A166D90A3F2426F336C8 |
| nxvm_model40_0_5_0539_x86.exe | C3169DBAA7AF8562BFFE4B879EF39B9D8CC9F620D27454F1C2B988153DFBFEB9 |
| nxvm_default_0_5_0539_x64.exe | 609A73F5D07DD08706229B2C0A7C2C7B0F7498C73DE96C8700119175A61F6A38 |
| nxvm_default_0_5_0539_x86.exe | 0B9DB45BFC2233135257FE64502CA105C3599504A9C65017A61C08E9EEA59B02 |

All four INI SHA-256 values are byte-identical to the table in
[S3 evidence](t539-s3-pit-extraction.md). No superseded artifact version or
unrelated product receiver was added.

## Actual-Change Acceptance

Shared implementation is 8a8435648; NXVM reconnection and the eight artifacts
are 06f99605d. Coordinator review inspected their actual changes, including
register ownership, C-read acknowledgement, callback lifetime and destruction,
port rollback, unchanged seed filtering/checksum, migrated test expectations
and the absence of an old RTC implementation. No new gap was found.
Required verification above passes. S4 is accepted; T539 remains open and the
remaining finite inventory is not implicitly accepted. Owner authorization
allows automatic admission of the next bounded chip batch.
