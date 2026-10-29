# T580 S90 — Representative late-guest throughput

The retained S87 instrumented TFT Product run already contains both late
World Clock frames and runner instruction-count observations. It was not
necessary to repeat the lengthy boot for this comparison.

`frame-63.bmp` shows `00:00:55` and has modification time 00:52:10 UTC;
`frame-64.bmp` shows `00:00:56` at 00:52:30 UTC. Their respective SHA-256
values are `419337A2133EF9D04C2AA29DFD5DA9969F0603E8A3FE4246F2E9307FEBDA843E`
and `F0B8AF8DA92E9AA2C94C7543A92D96A5C1D1A45457B0E7C252BF70A46DFE1734`.
The observer's capture loop requests a frame every 20,000 ms.

The same run's runner trace ended at 00:52:27 UTC. Its final records at
monotonic 16002140 and 16022140 ms report 363,016,822 and 368,174,859
retirements, about 258,000 per second. These intervals are close in wall-clock
time to the World Clock frames; the file timestamps do not establish
millisecond-exact correspondence. The screenshots demonstrate one displayed
second per approximately twenty host seconds in this instrumented run. The
retirement counts do not measure Core ticks, RTC or PIT, so they do not prove
which guest clock World Clock reads or identify the performance hotspot.
The trace SHA-256 is
`C80ABEE113C6D5FD1FDE539804F948D42C894B3A61F8CCA2EB2160B34C80800D`.

This is evidence about the instrumented TFT Product path, not a qualification
of the uninstrumented build, the derived-DSTN variant, or other hosts. No
production code or user media was modified.

## Isolated compiler optimization comparison

The pre-existing `build/.ninja/gcc-x64-lto-perf` configuration uses GCC
Release with `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`, the repository ccache
launcher and `-flto=auto`; it was reconfigured in place and built the real
firmware POST integration target. A private copy of the existing S89 runner
probe object was linked against its LTO libraries, without changing source,
release executables or guest data. After the link completed, sequential
10-second runs measured ordinary build 18,209,485 guest ticks in 10,048 ms
and LTO 20,970,200 ticks in 10,030 ms: roughly 15.5% more ticks per host
second in this one early-firmware run. This is not a statistically established
speedup and is not a late World Clock measurement. At a nominal 33 MHz even
the LTO run advances only approximately 0.63 guest seconds per host 10 seconds.
Compiler optimization alone does not establish a solution. LTO has not been
adopted or qualified for released builds.

## Process-local call-stack sample

A second 10-second early-firmware run sampled the task-owned runner threads
using the native Windows stack walker; no system-wide tracing was enabled.
The tool held and suspended only those threads, so the run is perturbed and
these inclusive counts are not execution-time measurements. Of 1,088 stacks
containing the Machine runner, 673 contained
`x86_core_machine_publish_elapsed_ticks`, 371 contained CPU execution refresh,
and 235 contained board deadline observation. A stack can contain several
of these functions; counts must not be added or treated as exclusive percentages.
The separate leaf sample contained 1,042 in-image instruction pointers, with
no single dominant leaf (the largest was `kbc_keyboard_inputs`, 126). This
supports investigating the scheduler and CPU path together; it does not justify
skipping device observations or prove that any individual change fixes the
late guest. Probe output was 14,466,253 guest ticks in 10,039 ms, lower than
the uninstrumented run as expected from sampling. The sampling script and
log SHA-256 are `77AE4C36CD734A5550C483A3AC3CABBCCA47DBA25128711B5ECD247550011A6C`
and `18DEA050F63E4C9832C6D5EAD7F8CCE5C1233DC44C384185CF958B72132C681D`.

A separate paused snapshot after another 10-second early-firmware run
observed a valid next deadline one source tick away, pending refresh, no
pending DMA, and neither `l1_compatibility` nor `fast_advance_blocked`.
Both the board timing declarations and explicit DMA clock were present. This
one instant disproves neither transient L1 nor other earlier states; it does
show that simply blaming a permanently missing DMA clock is unsupported.
Probe source and log SHA-256 are
`125C298367439D274BC5524C19762D1C70C0530B31203B5DE1F64F917A878852` and
`00982E25B59BCB5A854497DA147FD2A8F8219185C76D1B58315275A13B3A25DF`.

## Identified orphaned refresh deadline

A private instrumented copy of the scheduler measured 2,771,277 advance
calls and 17,878,139 loop iterations for 17,878,139 guest ticks in 10,046 ms.
Every loop advanced exactly one tick; neither L1 fallback nor fast-advance
blocking was observed. The PC110 profile supplies a `refresh_request` callback
that returns false and a no-op `refresh_complete`. The shared Board routes
refresh request/completion to those callbacks, but its generic PIT output
sets `refresh_hold_pending` regardless. The generic pending flag is then
never cleared by the profile callback, yet board deadline observation uses
it to repeatedly impose a one-tick deadline. This is a concrete ownership
inconsistency, not evidence that the real PC110 has a one-tick refresh period.

Two isolated candidates tested the causal chain without changing production.
A deadline-only guard reduced loops to 4,286,544 for 30,541,080 ticks in
10,042 ms. A more direct latch-ownership candidate sets the generic pending
flag only when there is no profile refresh-request provider; it preserves the
existing CPU refresh pulse and reduced loops to 3,825,743 for 26,528,709 ticks
in 10,039 ms. Individual runs are not rigorous benchmarking, but both show
the orphaned flag accounts for the repeated one-tick loop. These early-firmware
runs do not validate late guest behavior, all profiles, interrupt correctness
or a production fix. Shared changes await owner review.

The baseline and latch-candidate logs hash to
`3ADE2ED59EE0786191C15CF5A7FB3662CB0542F0BC856A26462F99C6475868A2` and
`69044A6775265C15E5922CBBB4D4429FE08BBE28FD101011296D33460CA40EAA`;
the temporary scheduler and latch candidate source hashes are
`B1310523F3C9FD54184E50111DA76E052922B79E79D8B00FC6C5984C905A4737` and
`0B6B851A2EF760955B0F720B515ABB42F3710FF2A88B4B2098F757F1AEF2E4F5`.

The existing x64 `board_dma_rtc_authority_smoke` was privately relinked with
the latch candidate and passed, including its normal AT refresh/hold and DMA
checks. A separate temporary extension then installed a no-request profile
callback and drove the actual PIT channel-1 edge. The same probe failed on
the baseline (`PROFILE-REFRESH:FAIL pending=1`, exit 1) and passed with the
candidate (`PROFILE-REFRESH:OK`, exit 0). This is targeted x64 evidence, not
a replacement for the required dual-width and product regressions. Temporary
probe source SHA-256: `EF67ECC4EC23748A01B4C3FF2DDC54D1FA83B3A6367CBFD3D4DFB7733CB0E33A`.

The same private fixture and candidate were compiled and linked with the
existing 32-bit Release toolchain. The unmodified baseline again failed
`PROFILE-REFRESH` with `pending=1` and exit 1; the candidate passed
`PROFILE-REFRESH`, the existing AT refresh/DMA/RTC assertions, and board
controller-handle checks with exit 0. This isolates the same defect on both
architectures, but is not a production or whole-product qualification.

The source-wide profile-binding inventory found two production providers:
PC110, whose refresh callbacks currently report no request, and DeskPro 386,
which owns the PIT refresh output and provides its own request, completion
and deadline. A private observable Board archive containing the latch candidate
was linked with the existing DeskPro D4 refresh-hold fixture; both x64 and
x86 returned `D4-REFRESH-HOLD:OK`. Together with the generic AT fixture, this
checks the two existing ownership styles, but does not establish a full
DeskPro boot or cross-product qualification.

The pre-existing CPU refresh pulse invalidates external-cycle locality state.
A further private fixture initialized both locality-valid flags, drove an
actual PIT output edge under the PC110-like refresh callback, and verified
that both flags were invalidated while the generic pending flag stayed clear.
The candidate passed this pulse check and the existing AT assertions on both
x86 and x64. The test uses a synthetic profile callback and private state; it
does not replace real-firmware or product-level validation.

## Longer real-firmware candidate observation

A separate x64 candidate linked the isolated latch change to the existing
real TFT Machine runner and ran for 120 seconds without UI, using private
copies of the prior task media and CMOS. At the four 30-second paused
checkpoints it observed 90,737,308 / 169,551,991 / 327,517,883 / 919,084,497
Core ticks and RTC register seconds 02 / 05 / 09 / 27. The earlier S89
baseline on a similar path saw 45,703,586 / 89,214,577 / 133,373,522 /
174,000,710 ticks and 01 / 02 / 04 / 05. These are separate runs, not a
controlled paired benchmark; diverging firmware states prevent attributing
the entire difference solely to throughput. At 90 and 120 seconds the
candidate observed CS:IP 3800:1333, without establishing the program or
visible screen. No F12 was injected because no halted checkpoint was observed.
The process returned zero. This is not a DOS, World Clock, desktop or full
product test. Candidate log SHA-256 is
`9F862E5A5E2ACA1C18F1C046EAC111224E8A3E087989D3694E1316918F0D073A`.
The resulting private media hashes are
`F27518A03119C26D0CA2721CBC14935E764E2F7D7F2DE91A2EC91CA12E04793B` and
`5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`;
owner media was not opened.

The same instrumented Machine runner was independently compiled for x86 and
the baseline and latch candidate run sequentially. Baseline: 11,742,481
guest ticks / 10,062 ms and 11,742,481 scheduler iterations, all one-tick.
Candidate: 23,878,371 ticks / 10,047 ms and 3,521,364 iterations, of which
680,760 were one-tick. Both exited successfully; neither saw L1 fallback or
fast-advance blocking. This one early-firmware pair demonstrates the
pathological scheduler behavior and its removal on x86 as well, not late-guest
or production qualification. Log hashes: `CC0701AC76F4978640DA6AEC62A95FEF53FCD25DAE1E03567EA609969486FCC8` and `DCDB53A75E34843308F6FD1EE2F8F0CE66439EBCD9362819DA6CCCFBA493ECE8`.

## Binding-order review

The production plan applies the PC110 profile binding before DMA configuration
installs the generic PIT refresh callback. Board reset also clears the generic
pending latch before subsequent PIT reset/edge handling. Thus the tested PC110
construction and reset path does not require an extra bind-time cleanup to
prevent an earlier generic latch. The public configuration API also permits
other call orders, which were not dynamically exercised here; the candidate
needs an explicit late-binding review before asserting a general guarantee.
The existing CPU refresh pulse invalidates external-cycle locality, which is
why suppressing or detaching the PIT callback would not be an equivalent fix.

The late-binding question was subsequently reproduced via public Board
configuration calls: configure generic parity/DMA, drive a real PIT channel-1
edge while configuration is still open, then successfully bind a profile
whose refresh callback reports no request. With the initial latch-only
candidate the output was `PREBIND pending=1`, `POSTBIND pending=1 deadline=1`;
so that candidate does not cover every supported configuration order. A
second isolated candidate also clears the old generic pending flag on
successful profile binding; the same probe returned `POSTBIND pending=0
deadline=0`. This modifies neither production nor its public interface, and
is not yet dual-width or integration qualification. The owner review was
updated to include this additional necessary handoff.

The combined candidate was subsequently checked on both widths: the public
late-binding probe observed `pending=1` before binding and `pending=0` with
no one-tick deadline afterward on x64 and x86. Both widths also passed the
private profile/pulse extension, the existing generic AT DMA/RTC fixture and
the existing DeskPro D4 refresh-hold fixture. These are targeted candidate
checks; no production edit, full build, full unit suite or release is implied.

A follow-up private dual-width probe attempted to bind an invalid profile
while the generic refresh latch was pending. Both returned INVALID_ARGUMENT
and retained `pending=1`; a subsequent valid bind cleared the old latch and
removed the one-tick deadline. Thus the candidate transfers ownership only
after successful binding in these tested paths.

A scoped static sweep of `profile_binding` consumers in Board-Base found the
other scheduler-facing profile paths use the profile `cpu_hold_asserted` and
`next_deadline` callbacks directly. The orphaned generic refresh latch is the
only directly identified case where the generic pending state remains in the
Board deadline after the corresponding operation has been delegated. The
refresh-NMI, reset, and shutdown callbacks have different purposes and were
not equated with this defect. This is a source-level check of this binding
pattern, not a proof that all device timing or failure paths are correct.

## Audit: can PC110 use the generic refresh path?

The Board profile binder currently requires refresh-request and
refresh-complete callbacks even though other Board code supports their absence
and falls back to the generic latch. PC110 supplies a false-returning request
and no-op completion. DeskPro supplies real D4 callbacks and owns the PIT
refresh output. PC110 also uses an elapsed guest-time port-61 status model;
that status signal is distinct from a Core refresh bus transaction.

A private alternative allowed the two refresh callbacks to be omitted only
together and removed PC110's empty callbacks. It therefore used the ordinary
AT latch/request/completion path, rather than preserving PC110's current
no-request behavior. The early x64 runner observed 29,915,252 ticks /
10,047 ms, 4,266,541 scheduler loops, and 745,410 one-tick loops. A real
TFT BIOS timer checkpoint freshly compiled against current headers passed
with the same reported timer delta 1675 and 8,094,093 retirements as the
freshly compiled baseline. Both logs hash to
`1EB6D3B2338BB85A406F5728848DA813030CDE989B36588DE99BADCDDB446DD4`.
This one checkpoint does not establish equal memory-bus behavior, physical
PC110 refresh wiring, other guest programs, or the full product experience.

An earlier private link reused a stale test object and crashed; Ninja's dry
run independently showed that object needed rebuilding. The crash was not
reproduced after compiling the fixture with current headers and must not be
attributed to the alternative refresh model.

Making the callbacks optional and selecting the generic route is simpler at
the App boundary, but changes Core refresh transaction behavior and expands
the Shared binding contract. The existing sources establish the guest
port-61 timing relationship, not the physical PC110 memory-refresh request
path. Accordingly this alternative is not yet justified as behavior-preserving.
Preserving the existing no-request behavior with the two-location latch
ownership correction is the smaller established semantic change; further
source or explicit approval is needed before choosing the generic model.
No production or Shared file was modified in this comparison.

## Approved production correction and current verification

The owner approved preserving the current PC110 refresh behavior while fixing
latch ownership. The Board clears the old generic latch only after a successful
profile bind and sets that latch on PIT edges only when the generic owner is
active. The CPU refresh pulse remains in its existing path. The existing Board
DMA/RTC fixture now covers a pending generic request, rejected profile bind,
successful handoff, and a later PIT edge. The original AT refresh checks remain.
Both x64 and x86 affected unit selections passed 204/204, including 190
X86-NXVM cases, PC110 unit cases, and DeskPro D4 refresh-hold cases. The
real TFT BIOS `--timer` checkpoint passed on both widths with delta 1675 and
8,094,093 retirements. This is not a full desktop or World Clock qualification.

The stable release build graphs rebuilt the 12 affected PC and PC110 EXEs.
The following are deployed asset names, PE machine values and SHA-256:

```text
assets/nxvm/nxvm_0_5_0580_x64.exe 8664 231616B2EAE892799494F791B390E572D8A2989535AA5C87F5A097D4BE7C3792
assets/nxvm/nxvm_0_5_0580_x86.exe 014C F7C7D0A64E800D7476D80680E15FAF334D9C6FC44963752CA7EC91C6FB4236B7
assets/my5160/my5160_0_5_0580_x64.exe 8664 F09B69C7530E8C0862B7495D95A514120B4128B1CA613524B0222C63638BCBF5
assets/my5160/my5160_0_5_0580_x86.exe 014C E699FBE12A28758278E4D8FC4E5667A9880DE3B323ED43A7EE7084951842D03B
assets/my5170/my5170_0_5_0580_x64.exe 8664 11BD3A5B6E6E8CC7B7BA0B0CFE1B651F97D4F5CC641D9B9339F538DE52946657
assets/my5170/my5170_0_5_0580_x86.exe 014C 75806D81EFC509C6018370B375C725AC32EDCBA6515A6B3DC581BF814EA5B7EC
assets/mydeskpro386/mydeskpro386_0_5_0580_x64.exe 8664 468E2AE842289383E49D7259B9593B6CC7468D7A3871746D19158A8F7095A19E
assets/mydeskpro386/mydeskpro386_0_5_0580_x86.exe 014C D8C5AF5FE1B755888444D9DDFB7D050AB7ECE9A35ED9250268F2E3902BC86C93
assets/mypc110/mypc110_tft_x64.exe 8664 92F26C4FCA8C4DCE40FA5F942EF73F0D50D1E6E69B932E8BA99FCEAFC1F124D1
assets/mypc110/mypc110_tft_x86.exe 014C EF25DE623FEE3BA42B44FAF42BF5889A76C3E5A0548D8C4BE174F383952FE474
assets/mypc110/mypc110_x64.exe 8664 1D481880232C9D4BBE9A03C576B16C68749E8814F213C2F07AEF9753E15AAD32
assets/mypc110/mypc110_x86.exe 014C 83FDA66D542EE54A9465EC4D14EAF4428872454FD689CF34713E255B1D14639D
```

The complete registered PC110 integration selection passed 8/8 on each
architecture, including real-firmware boot handoff and FDC reset, product
CMOS, and positive/negative firmware identity checks. x64 took 116.34 seconds
and x86 156.40 seconds. Other PC profile regressions and final review remain
in progress; these checks do not establish the native desktop or RDP path.

The selected four PC App unit regression passed 39/39 on each architecture.
After restoring the default profile in each build tree, the default NXVM
product/INI integration test also passed on both widths. PE width and current
asset hashes above were rechecked after that final relink.

The Shared implementation and its manifests/tests are committed at
`c886b4818`. The production diff is +3/-1 lines; the existing test gains
83 lines and the two manifests each replace one hash. No Lib, Emulator or
public API is changed. Final actual-diff review retains the necessary Board
owner check and paired binding failure/success regression; no helper, state,
thread, or compatibility route was added to production. The four Profile
artifact pairs and both PC110 variants were rebuilt on this source.

The observed real Product World Clock latency and native desktop/RDP
verification were not rerun after this change. The x64/x86 registered product
integration and the real BIOS timer check do not substitute for that user
experience check. S90 remains open for owner validation and T580 remains
open; no full-repository unit or integration pass is claimed by this batch.
