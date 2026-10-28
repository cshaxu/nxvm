# T580 S82 — repository integration qualification

The cached release graphs were configured with MyNES enabled, without a second
build tree. S81 records 713 unit tests passing on each host width. The default
PC profile registered 47 integration tests: 42 non-desktop and five desktop.

The first x64 non-desktop run passed 41/42. Native MIDI output failed while
opening the default device; an immediate independent rerun failed too. A
read-only WinMM probe observed one enumerated output and error 1 (undefined
external error) when opening either the mapper or device 0. The x86 complete
non-desktop run subsequently passed 42/42, including native MIDI, and another
x64 native-MIDI run passed. These observations do not establish the cause of
the transient failure and do not justify changing Shared Lib or the host.
WinMM device-capability inspection identified the enumerated device as
Microsoft GS Wavetable Synth; identification alone does not establish why an
open failed.

The x64 and x86 desktop test groups each passed 5/5. These automated desktop
tests do not verify PC110's actual screen or an RDP session. Both widths passed
PC110 real-firmware boot/resume and FDC integration on the first run.

The default-profile x64 full non-desktop rerun again passed 41/42 and again
failed native MIDI output. Direct WinMM open also again returned error 1. This
is an intermittent native host capability failure; the cause is not yet
identified and it remains a failed full-suite gate despite the later individual
pass. No test was suppressed or changed.

My5160 profile integrations passed 3/3 on each host width; My5170 passed 5/5
on each; DeskPro passed 6/6 on each. DeskPro's guest boot took 201.60 seconds
on x86 and 159.20 seconds on x64. The two existing build trees were reused and
their selected profile was restored to Default after testing. The following
relinked binaries passed the corresponding product/INI and architecture gates:

| Artifact | SHA-256 |
| --- | --- |
| my5160_0_5_0580_x64.exe | 094c63667d110bc5824d68af0d737387346732011d3a792aa6214ed63af2af1f |
| my5160_0_5_0580_x86.exe | 791c238cb4cf1c2a2cd573d7c97532644d5d40ee5448dba67cfc0d5ed75fb567 |
| my5170_0_5_0580_x64.exe | 4211c3c1de9b0763468a798f70309ff56c75c22e35934ef01dbc926b09a1b054 |
| my5170_0_5_0580_x86.exe | 0a4dd48918f4dba86ed533031d1a84cc43654ab4f82ec6bbb4b05c046d625403 |
| mydeskpro386_0_5_0580_x64.exe | f2af678f056fc1224e503e4fd7f2709c844ebb8a7b451c0ed1c27996181e5574 |
| mydeskpro386_0_5_0580_x86.exe | a47003e6f72757525e7f0c0b33b358add33f28b78dc5a6c0b9aba6b8299150d5 |

This record does not claim complete qualification while the x64 full-suite
native MIDI failure remains unresolved. No PC110 desktop or RDP session was
observed; automated desktop tests cover separate Lib and MyNES cases.
