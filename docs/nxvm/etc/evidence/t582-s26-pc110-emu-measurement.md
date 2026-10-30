# T582 S26 — PC110-EMU same-host exploratory measurement

An independently written task-only harness compiled the unmodified local
PC110-EMU Core at commit `81235b5` with the same host's x64 GCC `-O3`. It
loaded the same owner-supplied DSTN BIOS image used by MyPC110 and attached a
private copy of the same master image. One call requested 10,000,000 loop
steps. The process returned zero in 3.389 host seconds, including creation and
loading; the reference reported 9,029,199 instructions, PC `0003C2B8`, and a
running CPU. It reported 14,396 port-61 reads, 7,198 read-based toggles,
18 BIOS idle-HLT resumes, two synthetic iterations of its F000:4139 loop,
and 471,169 INT 16 calls.

This is not a matched-instruction or end-to-end speed comparison. The reference
uses high-level service hooks and compatibility shortcuts, counts some work
differently, reported a 4,014,080-byte attached image after its own media
interpretation, did not load the external font, and was not supplied the NXVM
CMOS or second disk. Although the input BIOS image is the same, NXVM's existing
PC110 firmware construction applies its L1 POST-suspend compatibility patch to
this image; the reference loads the unmodified input. It reached a different
guest state. Its 3.389 seconds
therefore cannot be divided into NXVM's fixed-work result to claim an
equivalent speedup, nor does it establish that its user interface or clock is
correct. The result nevertheless demonstrates that the local reference can
run this ROM/media combination and that it advances the observed bootstrap
through materially different paths. Its per-read port-61 toggle, host-derived
RTC, INT services, guest-specific repairs and instruction-loop handling are
real source differences; their individual effects have not been isolated.

No external source was imported, no owner media or external repository was
changed, and the temporary executable is not a release artifact. The probe
does not justify weakening the existing time model. Log SHA-256:
`2F49986761A087D765E07E87CC22D2AD9A0D06A5FFE30C8FF9FC6BEF8587F39E`;
reference Core source SHA-256:
`D484454BEB37A9D3EEF5CF716FCEF80E8131772C82532DF3A411704C57BF898A`.

## Isolating the port-61 shortcut

A task-only x64 build changed just the existing port-61 refresh-status read
from guest-time toggling to a per-read toggle. It intentionally changes
device timing and is not a production candidate. With private disk and CMOS
copies and the Release trace setting, the original model reached the fixed
10,000,000-instruction checkpoint (79,964,294 ticks, PC `0003DBC5`) in
21.49 seconds. The per-read version did not reach that checkpoint before
the bounded 150-second run was terminated and reaped.

A second 45-second observation of that candidate logged 7,596,998 executed
instructions and linear PC `000F699A`; for the remaining logged interval the
instruction count and PC remained unchanged while guest ticks advanced.
The selected BIOS input at the preceding addresses contains `CLI; HLT; RET`;
PC110-EMU separately has a specific compatibility path that resumes the HLT
at `F000:6999`. The experiment supports an interaction between the shortcut
and that firmware wait, but did not capture the live CPU halted flag or prove
the complete causal chain. It does show that importing only the per-read
status behavior does not reproduce a successful reference boot in this setup.
No timing policy, ROM, public code or released binary was changed. The
45-second observer was intentionally terminated and reaped after collecting
the bounded data, so it is not a completed boot. Log SHA-256:
`BFDFC5AC4DEF1A63796A09171E0046380FF95104D554617529D422A254699F7C`.

## Combining the refresh and HLT shortcuts

A further task-only candidate combined the per-read refresh toggle with a
signature-checked temporary firmware copy that replaces the `F000:6999`
HLT with a no-op. Both deliberately change guest behavior and are not
production candidates. With private media and CMOS, it reached 10,000,000
instructions in 18.984 seconds at 73,857,919 guest ticks and PC `000F6943`;
the ordinary model's checkpoint is a different PC and tick count, so those
durations do not compare equivalent work.

In a separate product-path run, the candidate did not reach the Personaware
menu. At the final sampled quantum it had executed 121,090,236 instructions
and remained in the BIOS's port-61 bit-5 polling region (`F000:693D` through
`F000:6943`). The task-owned process was deliberately stopped after 186.66
host seconds; this is a failed/incomplete observation, not a completed boot.
The local PC110-EMU reference also contains an address-specific escape for
this very loop that forces the ready condition after a fixed number of visits.
The current experiment does not prove a defect in NXVM's PIT model: this path
was entered after two artificial changes, and no PIT state was captured.
These results show that carrying over the first two shortcuts alone does not
deliver a working accelerated boot. No production source, public interface,
owner media or released executable changed. Observed log SHA-256:
`0476DE80F6ED48E5273AB5FD4200CA9175FACBBCC6E6326683322A384035BE61`.

### PIT state in the shortcut-only path

A separate task-only 55-second run sampled the existing PIT state from the
single guest worker, retaining both artificial shortcuts. At one, two, three
and four million port-61 reads, PIT channel 2 had control word `B0`, gate 0,
output 0, no ready/active/load-pending count, and zero remaining count and
phase. In that observed state, the existing output has no scheduled transition;
the BIOS bit-5 wait cannot become ready merely by advancing guest time. Earlier
samples show channel 2 had produced output while configured and gated, so this
is not evidence that its output is permanently disconnected.

The run was intentionally terminated and reaped at its 55-second bound. This
diagnoses the failure of the artificial shortcut combination only; it does not
show that normal firmware operation configures the channel incorrectly, prove
a PIT defect, or justify forcing bit 5. No production source or owner media
changed. Log SHA-256:
`0C5289E60D02666FD935CBF17A2D9BF72DCBC5C583E361DE9434A86C1944CBB7`.

## Three-shortcut upper-bound experiment

A further task-only variant added a signature-checked BIOS branch override at
`F000:6943` to the previous per-read refresh and HLT experiments. The branch
override is not identical to PC110-EMU's delayed, flag-adjusting escape; all
three shortcuts change guest behavior and this is not an eligible production
design. At 10,000,000 instructions it reported 77,461,118 guest ticks and PC
`000C4274` after 21.331 seconds. This is a different guest state from the
ordinary model and cannot serve as an equivalent speed comparison.

In a separate private-media product-path run it did not reach the menu feature
before its task-owned process was deliberately stopped and reaped after
337.5 host seconds. Its last sampled quantum reported 157,266,108 instructions,
1,012,853,806 guest ticks and PC `000C4274`; the recent samples alternated
within firmware/video ROM code. This does not prove the guest can never boot,
but it provides no evidence of the desired startup or gameplay benefit. It
does not justify adding a sequence of firmware-specific shortcuts. The input
ROM, owner media, production code and published EXEs were not modified.
Log SHA-256:
`8F309EE9773C282B2C3E3E78DC1A35F8BA30CEB507231C22F13156F3FEB6EE61`.

## Same synthetic instruction loop

A separate task-owned x64 harness generated its own 256 KiB synthetic BIOS:
the reset vector jumps to an unchanging `NOP; JMP` loop. It used no vendor ROM,
disk, PC110 firmware, BIOS service, or input. Both NXVM's neutral 486SX Machine
and the unmodified PC110-EMU Core ran 10,000,000 instructions in the same
process. NXVM reported 8,399 ms and PC `000F0001`; PC110-EMU reported 727 ms,
the same PC and 10,000,000 instructions. The reference's BIOS service and
identified loop-escape counters remained zero. An earlier independent run
reported 8,248/1,017 ms with the same endpoints.

The same harness also ran 10,000,000 refresh calls on the existing isolated
NXVM 486 CPU test fixture with the same loop, yielding 4,176 ms, EIP 1 and no
reported stop or fault. Unlike the neutral Machine, this fixture uses a simple
memory bus, starts directly at the loop and does not publish Machine time. It
is useful for separating areas to investigate, but subtracting the times is
not a precise scheduler cost. CPU reset controls, cache and memory backing,
and timing/observation contracts are not identical between the three cases;
matching PC and instruction count is not full-state equivalence. The reference
does not implement the same 486 source-timing and device scheduling contract.

This demonstrates a large per-instruction overhead difference on this narrow
loop even without the real firmware shortcuts; it does not attribute the whole
difference to one function or demonstrate a PC110 gameplay speedup. No source,
interface, owner media or published artifact changed. Log SHA-256:
`E257E16191CAD859077BE8B8621D49CA62F77CB6CF30E9BA812B3330B95C12B4`.

### Formula input prefix scan experiment

This repeats the already documented formula-input scan candidate in
[S11](t582-s11-performance-observation.md); it was inadvertently run again
before checking that record. The repeated measurement does not establish a
new candidate or justify additional repetition.

The existing formula-input collector scans the same prefix-byte set twice:
once to collect lock/segment flags and once to locate the opcode. A scratch
candidate combines these scans while retaining the original flag rules. It
adds no state or timing shortcut. Sequential baseline/candidate/candidate/
baseline synthetic-loop runs gave Machine times of 9,388/8,907/9,027/9,617 ms
on x64 and 14,204/14,616/14,606/12,650 ms on x86. Each reported ten million
instructions, PC `000f0001` and 40,000,014 guest ticks. The x64 samples are
encouraging but x86 is mixed, the workload is narrow and host variability is
visible. These results do not establish equal full state or a real PC110
speedup, and do not justify a production change. The scratch source and
probe live under the task-owned build directory; no source, interface, owner
media or published binary changed. Further work must prioritize a repeatable
gain in the actual guest while preserving the complete timing observation.

The same candidate was then tested on the existing private-media PC110 startup
fixed-work probe, in baseline/candidate/candidate/baseline order. At ten
million instructions, each run reported 79,964,294 guest ticks and PC
`0003dbc5`. The x64 times were 20,108/18,459/18,464/18,359 ms; x86 times were
23,748/27,091/28,711/26,939 ms. Changes in the second baseline show meaningful
host variability; the data do not prove a stable improvement and the x86
samples do not support adoption. Equal checkpoints do not prove equal complete
state. After these measurements the owner approved selecting a net code
reduction even without a large measured gain, provided other properties do not
clearly regress. The production collector therefore now uses one traversal
for the same prefix classification and flag collection. Its asymptotic time
remains O(n); it removes one duplicate scan and 17 net source lines, with no
new interface or state. The earlier timings are not a demonstrated speedup.

A model-level comparison covered all byte sequences of length zero, one and
two and 100,000 seeded longer prefix sequences (165,793 total). This compares
the traversal rules, not actual CPU execution. The actual dual-width cached
unit suites passed 712/712 each, including the CPU timing and prefix tests.
Default integration passed 48/48 on each width (x64 was split into 45 plus
three PC110 firmware cases). Selected My5160, My5170 and DeskPro integration
passed 3/3, 5/5 and 6/6 respectively on each width. The first x86 My5160
attempt failed two cases because its test executables had not been relinked
for the selected profile; relinking and rerunning passed 3/3. The x64 test
linker also transiently could not open several output executables; the same
build tree completed on explicit lower-concurrency retries. These failures
are recorded, not counted as passing attempts.

The external selected-set MyRaidenII startup and deployed Console probes
passed on both widths. The cached seven-product builds produced or reused 14
release executables; their PE widths and hashes are recorded in
[the artifact inventory](t582-s26-prefix-artifacts.json). MyNES does not
link this X86 Core change and its binaries were unchanged. Desktop-labelled
integration tests passed in this environment, but no interactive PC110 RDP
gameplay or real-time usability was established. T582 remains open.

### Instruction-boundary copy attribution

A task-only x64 link wrapper counted calls to the actual linked `memcpy`
symbol during the private-media startup fixed-work probe. It recorded
10,981,129 calls at return offset `3b13b`, totalling 4,172,829,020 bytes
(380 bytes each); disassembly maps this call to `ExecBoundary`. Its existing
source assigns the complete CPU into `instruction_state.data.oldcpu`.
The checkpoint remained 10,000,000 instructions, 79,964,294 guest ticks and
PC `0003dbc5`; the instrumented run took 22,451 ms. The wrapper adds atomic
counting overhead, sees only linked memcpy calls and is not an exclusive time
measurement. Preview copies occur separately; they cannot be added to the
boundary count as independent retired instructions.

The snapshot is used for more than timing: existing undefined-opcode and
WAIT restart paths restore the full CPU, and Debug can request its entry
state. Dropping or reducing the copy would change those contracts. A future
candidate must preserve the complete snapshot and measure both widths and
actual guest throughput. No production code, interface or artifact changed.

### Synthetic-loop timing and publication ablation

On the same x64 synthetic loop, a scratch link wrapper replaced the timing
selector with a constant one-tick result; another separately suppressed
Machine time publication. Both intentionally change timing and are not
production candidates. At 10,000,000 calls/instructions and the same reported
loop PC, the baseline CPU-only/Machine times were 4,304/8,603 ms. Omitting
the selector measured 2,020/4,993 ms; omitting publication measured
4,594/7,863 ms. Suppressed publication reported zero Machine ticks, and the
constant selector changed the tick total from 40,000,014 to 10,000,000.
These are single ordered measurements of a narrow workload, not independently
isolated costs, full-state equivalence or legal acceleration. They suggest
that repeated timing work deserves priority, but do not authorize a constant
timing model or reduced device accuracy.

The prior S5 scratch implementation, which shares one prefix count between
four 486 timing helpers but removes the standalone helper entry points, was
also linked to this synthetic loop. In two sequential x64 orderings its
Machine baseline/candidate timings were 10,338/8,191 ms and 10,336/8,375 ms;
CPU-only times were 3,845/3,806 ms and 3,829/3,935 ms. On x86 the respective
Machine times were 14,412/13,740 ms and 14,501/14,710 ms; CPU-only times were
6,133/7,049 ms and 7,646/7,332 ms. Endpoints and guest ticks matched, not the
complete CPU state. These measurements do not establish a reproducible benefit
across widths or a real PC110 improvement; the prior x86 Game observation is
also unsupportive. They do not justify adopting the test-incompatible scratch
variant. No production code, public interface or binary changed.
