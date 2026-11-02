# T585 S18 — FM output and shared PCM timing investigation

The external Open-Source-PC110 ES488 research traces discrete YM3812 and ES488
outputs into a resistor-weighted analog mixer, with PCM nominally higher than
FM. This source is a behavioral reference, not a physical measurement in this
repository. Yamaha YM3812 catalog LSI-2138123 (1992-04, reference PDF SHA-256
`35c4a73c59a480419305644482bfe7edecedd016d9e92301c2c2e7fe97f4ae49`)
describes nine melodic voices or six voices plus rhythm, operator envelopes,
feedback, waveform selection, vibrato and tremolo. A bounded implementation
must label any approximated synthesis L1/L2; a nonzero sine wave alone is not
proof of YM3812 compatibility.

S16's existing host bridge emits ES488 byte samples as they arrive. Appending
FM frames to that stream would serialize two physically simultaneous sources;
using the latest FM registers when the worker drains a queue would also
retroactively change samples that precede a guest write. The mixer must observe
both sources on the same guest timeline, including register writes, DMA sample
updates, reset and pause. It must preserve accepted-prefix behavior, use one
Lib PCM stream, and not block or call host audio from the guest device callback.

The candidate path is one App-owned mixed-sample producer driven by the current
Board/Core time, with the ES488 DAC value held between DMA transfers and OPL
operators advanced against the same time. Existing audio staging and Lib queue
remain the only host submission path. A bounded producer must yield to the
existing Machine executor before its storage fills, including turbo and HLT;
otherwise it would lose sound or retroactively generate samples after register
changes. Its exact buffer and yield contract still requires validation against
Core's run budget and HLT advancement before implementation is safe. Do not
create a second wall clock, execution thread, or generic recovery framework.

The included Yamaha catalog establishes register purposes and voice layout but
not all numerical envelope/synthesis behavior. The authorized external emulator
can inform independently implemented L2 behavior under the repository source
policy; source code is not imported. No FM samples have yet been produced by
this project, and neither music compatibility nor physical audibility is
established by the S17 timer tests.

## Executor boundary checked

`x86_core_machine_run()` accepts a tick budget and can bound a run, whereas
the current X86-NXVM turbo runner passes zero ticks and uses an instruction
budget. HLT advances to a Core-selected next deadline through the existing
waiting path. Consequently a bounded mixer cannot assume that the current
4096-instruction turbo quantum always fits a fixed-size sample queue. A correct
implementation must bound both ordinary running and HLT against available
sample capacity while keeping the existing control rendezvous responsive.
Whether the current profile interface can express this without an additional
contract needs to be established by implementation and targeted tests; no
interface or queue has yet been added for this investigation.
