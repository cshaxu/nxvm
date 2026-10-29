# T580 S89 — PC110 guest clock-rate investigation

The S88 record documents a real released-product World Clock display advancing
two displayed seconds between two captures separated by about twenty host
seconds. That alone does not identify RTC, DOS timer delivery or CPU execution
as the cause.

An x64 task-private probe based on the existing real-TFT-firmware timer
diagnostic sampled the Core time observation and Lib monotonic milliseconds
after each million retired instructions. It ran the existing unpaced firmware
execution path for a bounded ten-second host interval and intentionally ended
before the timer diagnostic's own success checkpoint. The diagnostic exited
1 and is not a passing registered test. Samples were:

| Host elapsed | Core elapsed | Retired instructions |
| --- | --- | --- |
| 7,288 ms | 7,038,768 ticks | 1,003,520 |
| 12,474 ms | 13,654,795 ticks | 2,007,040 |

PC110's selected source axis is 33,000,000 ticks per guest second; the latter
sample is approximately 0.414 guest seconds in 12.474 host seconds. This
establishes slow throughput in this instrumented firmware diagnostic on this
host. It is not an actual Product runner measurement, a guest DOS/PIT or RTC
port measurement, or a claim that this ratio applies to World Clock. The
existing diagnostic and probe can affect throughput. Do not replace guest
time with host time on this evidence.

The private probe source, executable and captured log SHA-256 values were
`2540CA667E26C39E5B535FADCB4B3FEA459DFC66117B3B93D170AF9768B5E6BE`,
`F61B08B7A42561729EB2D292E87FD7257FA063C7795852E947C869E81A83DC51` and
`91A35749D7104D6EF719B15A72A4B18FA3E455C7FA77373DF80474F4ADADD6D2`.
No production code, user files, or shared component was changed.

The same private probe compiled as x86 produced 7,038,768 ticks at 5,955 ms
and 13,654,795 ticks at 12,589 ms, at the same respective retirement counts.
It too deliberately exited 1 before the timer checkpoint. Its executable and
log hashes are `ACAB299A81B4BFCC1122318BF43358D970B7E2BE70FCC92FCA5E042BCF2FAE54`
and `1E0EA43DF292D9A954416C115F2E55EB6CD744315BB21E9D4F5FE573DC35D20B`.
This supports the same bounded diagnostic observation on both widths; it does
not turn it into a product throughput or guest-clock root-cause proof.

## Real Machine runner, no UI

A second private probe constructed the real TFT firmware and PC110 Machine,
bound its normal driver to Emulator Machine, reset, read RTC seconds through
a paused Debug lease, resumed in the PC110 default Turbo mode for ten seconds,
paused and read the same RTC and Core tick observation. It used no UI, disks
or user configuration. Both runs exited zero and safely shut down:

| Host | Host interval | Core ticks | RTC second before/after |
| --- | --- | --- | --- |
| x64 | 10,041 ms | 18,356,726 | 00 / 00 |
| x86 | 10,035 ms | 11,846,736 | 00 / 00 |

These correspond to approximately 0.56 and 0.36 guest seconds on the nominal
33 MHz axis. This is direct evidence that the actual Machine runner does not
achieve real-time progress on this host during this early firmware workload,
and that the RTC reflects the slow guest time. It does not isolate the
performance hotspot or establish the exact cause of the later DOS World Clock
display. Reset and Debug sampling are outside the timed guest execution; the
small start/pause handshake overhead is included in the host interval.

Probe source SHA-256 is
`FAAC5C37F9CF9DD1DE8E25F1A1B952B4F8740943F08E794731359BC43DC1D3C7`.
x64 executable/log hashes are
`278C72D33A4236889C5E7A36362E491522F573A74EC1F843B5BE2535598F1FF8` /
`0B81764534070CAD6CBF02E52F4477E0B484CA65FE873078061F31433A33C105`;
x86 executable/log hashes are
`7876C2CB1E98D8FD319105B73169D5F2C611D2E37613E7FA602A199CFA6F5B7A` /
`704BA33CC43CBB8CD546099012A501F94EB2ED2B5F9ECA46DD6DD3F76C070263`.

## Firmware runner with isolated guest media

A separate x64 no-UI run used the released TFT firmware, a copied guest CMOS
and copied master/slave images in direct mode. It sampled at four paused
boundaries and resumed after each. At 30, 60, 90 and 120 host seconds it saw
respectively 45,703,586; 89,214,577; 133,373,522 and 174,000,710 Core ticks,
and RTC seconds 01, 02, 04 and 05. The 60- and 90-second CPU observations were
`F000:EA88` and `F000:EA86`, within the port-61 transition-polling delay
already described by [S40](t580-s40-generic-delay-acceleration-design.md).
No halted boundary was observed, so the planned single F12 input was not sent;
the 120-second run ended normally without establishing a DOS or World Clock
checkpoint. Periodic pause/debug sampling itself perturbs execution. This
supports slow real-firmware progress, not a measured DOS bottleneck or a
license to skip side-effecting port reads.

The private source, executable and log hash to
`0C43A23CB891A4560C7981763D57E90F46912E268600FA8765CF86F893A44319`,
`D64125D5FDD2077093AD918A8419161D2A917ACBF4DBBC1189693EA809EEABBD` and
`83C54D37189A46F7FBBA459D9A84E822E2F39A9249800C0F3F727ED472F6C033`.
Both copied disk hashes and the copied CMOS hash were unchanged after the run;
original user media and configuration were not opened by the probe.

## Reused late-guest execution trace

The existing S87 instrumented Product-path trace records host monotonic
milliseconds and retired instruction totals every ten seconds. Its associated
input log records completion of the eight calibration events at instruction
284,204,324. Between trace points 336,848,770 and 368,174,859, after that
calibration, the runner retired 31,326,089 instructions in 130,000 ms, about
240,970 instructions/second. An earlier interval from 2,228,224 to
22,685,971 retired 20,457,747 in 100,032 ms, about 204,512/second. The late
guest path therefore exhibits the same order of instruction throughput as the
early path; this evidence was already available and did not require another
long boot. It is an instrumented run, not a direct measurement of RTC, PIT,
World Clock, or the uninstrumented executable's precise throughput.

The reused runner log SHA-256 is
`C80ABEE113C6D5FD1FDE539804F948D42C894B3A61F8CCA2EB2160B34C80800D`,
matching the S87 record. The independent input log records all eight queue
operations with status zero. Neither result proves the cause of every guest
clock display or validates timing on other host machines.

## Bounded hotspot sample and rejected candidate

A short Windows thread sample of the task-owned x64 runner collected 74
instruction pointers in its executable and 158 outside it, including threads
waiting in system libraries. The largest individual in-image counts were
clock-domain conversion (7), KBC input computation (6), and memory route
resolution (4). The sample is too small and too distributed to identify a
dominant hotspot; it also perturbed the run. The sampler and its log hash to
`800280734D2FAE7B1B14546D28D0E8D9B515C470B4527F386D7154F28EDDD5E2` and
`14D15FA73097C3762E910E70FBF116E4CEA0959467F20B61BAAA0129F9175C73`.

A private archive copy tested a clock conversion candidate that skips an
unnecessary first division when an elapsed interval is below its denominator.
One sequential baseline/candidate run on the same runner measured 16,392,027
ticks in 10,038 ms versus 16,152,432 ticks in 10,041 ms. The candidate did
not improve this workload; the approximately 1.5% decrease is not a robust
benchmark conclusion from one pair. It is rejected without modifying Shared.
Baseline/candidate log hashes are
`F7CA525B94DDE2B58DF29588422A76B98DA639F7C7663BDFD9C163BFC2C59930` and
`8600E52599C1C2E3B4177C6BC974FA484CEFC4327C423DD738B7D7A2F604787F`.

A second sampler held handles only to the task's three probe threads, without
system tracing or UI. It collected 1,659 in-image instruction pointers; the
largest individual bucket was `kbc_keyboard_inputs` at 222, followed by
clock-domain conversion at 86. There were 3,577 out-of-image samples from
all three threads, including waiting system code. Sampling does not measure
complete call stacks or prove exclusive cost. Source and log hashes are
`C30D2807B07703658EC2A244ED82C9097CD90F879F308D314C8307DC6639AAF0` and
`53940A5FCC212E6F75D7ED14FDFAF866BE0A633CD091D07AAED1578F336B2481`.

An isolated KBC candidate skipped the repeat-deadline call when the existing
keyboard signal already reported zero repeat bytes. The source indicates this
is an idle-only equivalent condition; no production code was changed. One
baseline/candidate pair measured 16,815,963 ticks in 10,036 ms and 16,760,667
ticks in 10,038 ms, respectively. It did not demonstrate an improvement and is
rejected; one pair is insufficient to establish a precise percentage. Log
hashes are `B49FEEA2124A4B8D8B2DD1777E5E297D81EDF91A1AC1A23812C97B2977FD3775`
and `F92E971886D58D851A48BC68307324474240D52A484BE6C0888B32FFF5454EC7`.
Disassembly confirms the sampled `kbc_keyboard_inputs` symbol is its actual
small callback, not a broad mislabelled function range. The compiler already
combines the duplicated `keyboard_scanning` expression in `get_signals`; a
source-only removal of that duplication is not evidence of a runtime gain.
Even eliminating the entire 222/1,659 in-image sample bucket would not by
itself explain the order-of-magnitude gap to 33 MHz. This is an illustrative
sample-based bound, not a prediction of total speedup or a complete profile.

## Audit disposition and remaining receiver

S89 establishes that guest progress is substantially slower than the nominal
clock in the sampled early firmware and has comparable instruction throughput
in the retained later instrumented guest run. It does not establish a specific
low-risk production optimization or prove the exact later World Clock path.
Both tested local candidates are rejected; no code, public interface or
release artifact is changed by this audit.

The remaining T580 receiver is a representative late-guest Machine timing
and throughput investigation: correlate host time, guest Core ticks, RTC and
PIT/DOS time after the boot and input checkpoints, then identify a measured
hotspot and demonstrate any candidate against the same workload. Preserve
ordered device observations and all guest timing/interrupt semantics. A
boot-loop shortcut, a changed nominal PC110 clock, host-time substitution or
an architectural rewrite is not justified by the present results. The
released firmware/display, user input and user-media boundaries remain in
force. Full T580 qualification is not claimed.
