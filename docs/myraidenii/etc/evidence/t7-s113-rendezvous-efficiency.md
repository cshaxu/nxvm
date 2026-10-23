# M1 T7 S113 — MyRaidenII Machine rendezvous efficiency

## Baseline

The x64 Release selected-set startup probe completed 2,000,000 instructions
and 33,287,472 guest ticks in 4,969 ms.  The Board's declared guest axis is
16,000,000 ticks per second, so this bounded Core-only route ran at about
0.42 times nominal guest time.  It uses a 4,096-instruction Core budget.

The production Machine used a distinct 256-instruction host-control bound and
called the existing Emulator executor callback before and after every bound.
The callback owns input/lifecycle rendezvous and asks the Machine for a frame;
the Machine already declines frame composition within an unchanged VBlank.
The 256 bound therefore created sixteen times more callback/frame-poll turns
than the tested direct route without adding guest-time precision.

## Change and retained boundaries

The one App-local constant is now 4,096 instructions.  Both callback positions
remain: the first still consumes control/input before Core runs; the second
still publishes the completed safe point.  The Core run remains the sole guest
time owner and retains its zero tick budget.  Audio draining, host-stream
backpressure, VBlank publication, failure propagation and all public interfaces
are unchanged.

At the measured pre-change throughput, a 4,096-instruction turn is about
10.2 ms.  That is below the Board's nominal 18.0 ms VBlank interval, while
removing the fixed sixteen-fold control polling.  This is a host scheduling
bound, not an emulated timing change.

## Excluded candidates

- Full-frame conversion already has one successful publication per VBlank.
- The L1 YM/OKI synthesizer is computationally heavy, but changing its sample
  rate, waveform or model would change audio behavior.  A prior bounded pitch
  micro-optimization did not establish a benefit.
- No Shared callback contract, thread, queue, renderer or second timing source
  is needed for this local scheduling correction.

## Verification boundary

The existing Machine Driver unit now rejects any budget other than 4,096 while
retaining its two-callback/lifecycle/audio checks.  The selected-set startup
probe remains a Core-direct diagnostic; it verifies the same external ROM route
on both widths but cannot measure the Product driver's callback reduction.
Native Window/RDP interaction and audible output require owner desktop review.

## Completed verification and release

The focused Machine Driver unit passed on x64 and x86.  The complete registered
MyRaidenII unit selection also passed on both widths (13/13: x64 in 6.60 s;
x86 in 7.45 s).  The direct selected-set owner-ROM probe completed on both
widths after the change: x64 completed 2,000,000 instructions in 5,087.1 ms;
x86 completed the same bounded route in 5,882.5 ms.  This confirms the
selected-set route remains valid, but remains deliberately separate from a
desktop gameplay measurement of the Product driver.

Release artifacts were rebuilt and deployed without changing any Shared
component: `myraidenii_0_0_0007_x64.exe` is PE `0x8664`, SHA-256
`7ca14e828966d72da13a7e20555830ec14ad1dfa4634aa766a7a8726bca1c4ca`;
`myraidenii_0_0_0007_x86.exe` is PE `0x014c`, SHA-256
`f0b9bce6e9aa3f64b44619c7fe17dd4ceeae55eb6e5903dab505815be41b564f`.

The native Window/RDP and audible-output qualifications remain owner desktop
validation, not a consequence of the automated evidence above.

## Follow-up steady-state isolation

After owner feedback that the delivered quantum reduction was still not usable,
the bounded x64 selected-set route was rerun through its existing external
staging wrapper at 8,000,000 instructions with the established VBlank-relative
Coin/Start sequence.  It completed in 23,650.1 ms for 133,667,500 guest ticks
(about 0.35 of the declared 16 MHz guest axis), while producing 100,444 PCM
frames, 4,948 YM register writes and four active OKI voices.  There was no
Board or COP provider failure.

To separate the Sound contribution without retaining a second production path,
the existing diagnostic probe was temporarily built with its already documented
optional Sound input omitted, run once, then restored and rebuilt from the
tracked source.  That same bounded route completed in 20,438.7 ms.  Sound
therefore accounts for roughly 13.5 percent of this measured boot/attract
phase, not the missing factor required for real-time guest execution.

The remaining roughly 20 seconds are V30 execution and its required Board
advance (video/COP) path.  Window composition was sampled only four times in
this direct diagnostic, so it cannot explain the Core-only deficit; actual
Window work can only add cost.  A separate global-LTO diagnostic reduced the
earlier 2,000,000-instruction route by about 9 percent, also insufficient and
not a portable release remedy.

Consequently no further App-local quality-reducing change is justified:
lowering PCM rate, frame skipping, lying about the guest clock or weakening
video/COP advancement would change the emulator rather than remove duplicate
work.  The remaining performance owner is the shared X86 Core execution path.
S113 records this transfer boundary; it does not alter Shared source.
