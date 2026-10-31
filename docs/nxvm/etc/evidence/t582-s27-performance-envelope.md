# T582 S27 — performance envelope

Baseline: S26 closure `010a4b067`. This is a prioritization record, not proof that PC110 is playable or that two emulators perform identical work. Existing private media were not changed. Samples are wall time on this host; timings from different experiments are not additive.

## Controlled synthetic loop

A locally generated BIOS containing a NOP/JMP loop was run for 10,000,000 instructions, using task-owned scratch probes. The reference checkout was PC110-EMU `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`. No vendor firmware or disk was used. End PC was `000f0001`; NXVM Machine reported 40,000,014 guest ticks. One ordered sample per architecture:

| Build | NXVM CPU-only | NXVM Machine | PC110-EMU |
| --- | ---: | ---: | ---: |
| x64 | 4.039 s | 8.413 s | 0.860 s |
| x86 | 5.776 s | 12.242 s | 0.814 s |

The CPU-only fixture has different initialization and bus behavior; the reference has different timing and machine contracts. The same instruction stream and end PC do not establish equal machine state or equal work. The differences are a reason to investigate the execution chain, not a quantified whole-product speedup or an attribution by subtraction. Task scratch output SHA-256: x64 `b328ccee38c5b024c9f0749424492a7d5a5c5150a957e8a109951fc3f54856ca`, x86 `05940324258d2884b3e1472336d25df3fbc492e4a02ed4e5e93a81826b9095f3`.

## Existing product-path evidence

| Workload | Evidence and boundary |
| --- | --- |
| Fixed PC110 work | S26 candidate, private media, 10 million instructions, 79,964,294 ticks, end PC `0003dbc5`: x64 19.918/20.030 s; x86 24.014/23.941 s. Matching PC and ticks are not a complete-state comparison. [Evidence](t582-s26-rotate-constant.md). |
| Personaware Game | Current x64 release reached a stable sampled Game image 31 one-second samples after Enter, 255.0 s total; image SHA-256 `e5c1ab80c00f84c1b80c1069efa58fd01d761fde7ac07f7982f0b0b34ab0de99`. This does not establish interactive usability, RDP behavior or complete state equivalence. Earlier x86 Game measurements are not measurements of the current binary. [Evidence](t582-s26-rotate-constant.md). |
| World Clock | A sampled x64 run advanced 192,453,129 guest ticks in 60.668 host seconds, about 5.83 seconds at the configured 33 MHz. Sampling perturbed execution and the observation is not a desktop/RDP measurement. [Evidence](t582-s26-486-term-sampling.md). |
| Execution profile | Earlier sampling identified instruction execution and timing/observation as substantial costs; the reported nested percentages use different denominators and must not be added or treated as exact whole-program shares. [Evidence](t582-s20-execution-cost.md). |
| Game drawing | Sampling found ordinary execution dominates the measured drawing interval; SMM proportions differ by phase. Instruction byte samples are not retired-instruction counts. [Evidence](t582-s26-personaware-game.md). |

## Investigation order

1. Trace the actual instruction fetch, decode, execution, retirement and Machine advance chain, including repeated per-instruction work and component boundaries. Measure candidate costs on the same fixed workload before proposing changes.
2. Inspect timing/observation calculations for duplicate work while retaining the current guest clock and documented timing accuracy.
3. Examine scheduler deadlines and device advancement with real event boundaries; do not skip guest events or introduce another clock.
4. Reassess memory, video and other device costs against the actual workload after the above measurements. Preserve candidates that reduce production code or algorithmic complexity without a material regression, even if a speedup is too small to resolve.

Earlier LTO and other candidates, including build and binary-size tradeoffs, are recorded in the [investigation index](t582-s26-investigation-index.md); do not assume their measured results generalize or rerun them without a new question.

S28 must produce an end-to-end design tied to actual code and evidence. Current-binary x86 Game completion, real desktop/RDP behavior, comparable complete-state evidence, and final full-product receiving remain open. T582 remains open until the requested usability and receiving criteria are met.
