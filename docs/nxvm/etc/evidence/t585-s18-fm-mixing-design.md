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
changes. This candidate required an explicit buffer and yield contract against Core's
run budget and HLT advancement before implementation. Do not
create a second wall clock, execution thread, or generic recovery framework.

The included Yamaha catalog establishes register purposes and voice layout but
not all numerical envelope/synthesis behavior. The authorized external emulator
can inform independently implemented L2 behavior under the repository source
policy; source code is not imported. At the S17 baseline no FM samples had
been produced, and its timer tests established neither music compatibility nor
physical audibility.

## Executor boundary checked

`x86_core_machine_run()` accepts a tick budget and can bound a run, whereas
the current X86-NXVM turbo runner passes zero ticks and uses an instruction
budget. HLT advances to a Core-selected next deadline through the existing
waiting path. Consequently a bounded mixer cannot assume that the current
4096-instruction turbo quantum always fits a fixed-size sample queue. A correct
implementation must bound both ordinary running and HLT against available
sample capacity while keeping the existing control rendezvous responsive.
The implemented profile contract now expresses the bounded quantum; a real
runner regression checks both bounded and unbounded turbo paths.

## Implementation under verification

A single App-owned, 48 kHz timeline now combines the latest timestamped ES488
DMA sample with the OPL signal into the existing Lib PCM stream. Guest register
writes first render the elapsed interval using the previous register state;
there is no host call inside the port callback. A 20 ms profile-owned quantum
limit and an audio deadline during HLT bound production before the existing
executor drains the finite queue. The limit is installed only when a host PCM
stream exists; endpoint-free PC110 and other profiles keep their existing
budgets. Partial Lib submissions retain the unaccepted suffix.

An actual guest-path regression exposed that `get_elapsed_ticks()` rejects
RUNNING. Consequently the old ES488 DMA callback could not obtain a timestamp;
its earlier Debug-port setup did not prove real guest-issued DSP commands. Core
now has an explicitly owner-callback-only read of its existing clock, while the
external paused snapshot retains its original restriction. Tests execute real
DSP and OPL OUT instructions rather than relying solely on Debug writes.

The synthesizer is an independently authored **L1 approximation** of the
Yamaha register layout and nine two-operator melodic channels: frequency,
multipliers, key-on/off, a continuous envelope, attenuation, feedback, additive
or FM routing and four estimated waveforms. It does not yet implement rhythm
percussion, vibrato, tremolo, key scaling or the quantized Yamaha tables; no
claim is made that it reproduces all YM3812 music or real hardware sound.

Dual-width tests exercise the real guest DMA and OPL writes through
Machine into Lib PCM, simultaneous-source mixing in the existing audio unit
fixture, partial acceptance, full-buffer recovery and resumed playback. These
checks establish the exercised paths only; full qualification is recorded below. Tests initially revealed an immediate-breakpoint
observation before the final DAC interval and a test-only notification-sink
restore omission; the breakpoint now permits elapsed guest time and the sink is
restored. Neither is counted as a successful earlier run.

## Verification and actual-change review

The x64 cached unit run passed 713/713. Subsequently extended PC110 and
Shared fixtures were rebuilt and passed separately. The x86 cached unit
run passed 711/713; `lib.types-layout-selftest` and `emulator.verifier-negative`
timed out at their existing budgets while an unrelated repository was building
concurrently. Each was rerun individually and passed in 78.44 s and 22.88 s,
respectively. The initial aggregate was not a clean pass and no timeout was
expanded. The x64 and x86 full registered integration suites each passed
48/48, including desktop and native MIDI; after the final PC110-only changes,
the PC110 suite passed 20/20 per width and affected audio, composition and
firmware-artifact checks were rerun. No human or RDP audibility test is claimed.

Four fixed PC profiles were rebuilt from the same cached trees and exercised
on both widths: My5160 3/3, My5170 5/5, DeskPro 386 6/6 and default NXVM 3/3.
Both MyRaidenII selected-ROM probes completed 2,000,000 instructions with the
same `db1f73fa40511ae0` frame digest; both deployed product Console tests passed.
This does not establish game playability. The MyNES pair was built and was
byte-identical to its existing published files.

Actual-change review confirms no Lib, Emulator, X86-IBMPC or X86-Debug source
change. X86-Core adds a serialized callback-only read of its existing clock;
X86-NXVM adds the optional per-profile output quantum limit, with both owned
Shared test/manifest updates. PC110 owns the synthesis and one mixed output
pipeline. No second host stream, wall clock or scheduler was introduced.
Owner ROMs, media and INI files were not changed. The L1 synthesis limits above
remain open for later device work; physical audibility has not been verified.

All 14 assets were checked for their x86/x64 PE machine identity. PC110 firmware
artifact and negative-identity checks passed after its last rebuild. SHA-256:

| Artifact | SHA-256 |
| --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | 3c20a1d30cbdbdf1a98776fae5899960b4a375b889d758c3c2db2d6fb99a63e4 |
| assets/nxvm/nxvm_0_5_0582_x86.exe | 77fc0e1c1c0a91a159eb3ba3c44c26d22d5af0e6c094dd17764d3537bd3d0180 |
| assets/my5160/my5160_0_5_0582_x64.exe | aa9f0393c08fc5451f908b42f3f88f28cbe8822e551d22806d0b445b109bba45 |
| assets/my5160/my5160_0_5_0582_x86.exe | 46ad374c649c6aefd1233b1c9a2409c9be2c20c242356f6d39961014c2e36009 |
| assets/my5170/my5170_0_5_0582_x64.exe | f4fc5749afc1922c528a1f2978bcc678a797fada177ef04c7d9fe8b48493ea4d |
| assets/my5170/my5170_0_5_0582_x86.exe | ec58117739fc42ba1a5612b6ae726cd50897b38838b7c9fb871d582f60e0deb5 |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | 2b7c0d6c47458de3f0aa3c9c08b5027b0c8472f81a63ac1b076a4622f282f500 |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | c890d09dbbc53387b241b55b59db2b3c5252f81883869ec5b60dc741f8989033 |
| assets/mypc110/mypc110_x64.exe | 937fb8d4f74d907a52036a8aec51a8e3111d32f9feded2f1c2e5fc2fd9fc8e6b |
| assets/mypc110/mypc110_x86.exe | 076b9d11413cb7ac7e247776385f5b86967d29a23badb47116034818daf36ea4 |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | 3301ca497b8c5977ec1acab2f7bed736532331b1cb9c8e9a673cbc93633dd3c8 |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | 7ee0475d785632b4d65b9e7cc99b2e4db427fa9690756f8952ec437000db186c |
| assets/mynes/mynes_0_0_0044_x64.exe | b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8 |
| assets/mynes/mynes_0_0_0044_x86.exe | 718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16 |
