# T582 S26 — next-instruction preview bound and 486 observation

Taken control transfers can require a next-instruction lexical preview.
The 80286/80386 timing evaluator uses the decoded length/components in its
source timing; independently, the CPU's completed instruction observation
promises to preserve the next-instruction component count across asynchronous
entry. Machine retirement observers and physical qualification consume that
fact. The 486 nominal timing itself does not require the 286/386 next-term
formula, but suppressing its observation preview changes the CPU observation
contract. A missing subscriber in a product configuration does not change the
CPU API's completed-observation promise.

A task-only ablation suppressed the timing collector's observation preview;
it deliberately changed that contract and was never a production candidate.
For a synthetic `CMP`/taken-`JE` loop, baseline/ablation/ablation/baseline
times in milliseconds were CPU 4403/2888/2955/4324 and neutral Machine
10272/6983/7071/11089 on x64; CPU 6998/5500/5253/7782 and Machine
14665/11132/11144/14059 on x86. All Machine runs reported 10,000,000
instructions, 40,000,014 ticks and PC `000f0002`. This branch-dense loop
establishes a possible upper bound under an intentionally altered observation
contract; it is not a production speedup, complete-state equivalence or a
representative Game measurement.

On a separate x64 PC110 private-media product-path run, the same ablation
reached a visually verified complete Personaware game scoreboard 24 one-second
samples after Enter, with total run duration 240.37 seconds. The prior normal
run took 25 samples after Enter and 265.48 seconds total; the runs are ordered,
use asynchronously sampled frames and do not establish equal complete guest or
host states. They cannot establish a one-second gain or a precise startup
speedup. The complete ablation frame SHA-256 is
`978BBC114A41A95F65D422F76CDAE98E678467EE9BBBA48F985BA4874D75D3AE`.
Machine-injected input is not native Window/RDP gameplay validation.

An initial ablation run used an incomplete two-pixel frame marker and stopped
at a partially drawn scoreboard; its 25-sample result is not a valid complete
frame comparison. The subsequent run above used the existing additional
score-label pixel and the saved frame was visually inspected. The prior normal
run's saved 25-sample frame had already been visually verified complete.

The existing asynchronous retirement-observation case previously covered
80386 only. It now also exercises 80486SX, reusing the same taken-branch and
non-branch cases with a pending NMI. The normal test passed on both widths.
Linking the ablation into that same test failed on both widths with the 486
branch's `next=255`, demonstrating the added case catches the contract loss.
Only the existing Shared test and its manifest changed; production behavior,
and public APIs did not. Cached dual-width unit suites each passed 712/712;
default integration passed 48/48 per width, and 5160, 5170 and DeskPro profile
integration passed 3/3, 5/5 and 6/6 per width. The MyRaidenII external startup
and product-console probes passed on both widths, without proving gameplay.
All 14 product EXE architectures were checked. MyPC110 and MyNES binaries
were byte-identical to the preceding release; the other ten rebuilt executables
differed only in four PE timestamp/checksum bytes each. Both build trees were
restored to the default PC profile. Manifest, documentation and artifact-root
checks passed. These results do not establish a safe lazy preview design or
playable PC110 speed. T582 S26 remains active.
