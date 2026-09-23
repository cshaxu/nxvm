# M5 T458: VM Runner Presentation Cadence

T458 owns the shared VM runner defect where a bounded instruction-control
quantum also forces full display snapshot work. It separates host presentation
cadence from guest execution while preserving the existing Core display owner,
copied mailbox boundary and immediate mode/lifecycle updates.

The active S1 packet in `states/CURRENT.md` is the execution contract.

## Closure

S1 implementation `aa79f0e6` replaces normal per-256-instruction maximum-size
snapshot work with one VM-owned 16ms host-clock cadence. Core still owns display
state and snapshot capture; forced mode/lifecycle publication remains immediate.
The focused VRAM-writer regression, Release and Debug focused smokes,
documentation governance, static gates and the final 293/293 Debug current gate
pass. The Release artifact is `nxvm_0_5_0458.exe`, SHA-256
`F9A77CDE35A4DEC18CC566800D59205EC43A41C4B30F533E2EF132E0DE14F71D`, with no
`.debug` section.

The [retained proposal](M5-T458-vm-runner-presentation-cadence-proposal.md)
and [S1 evidence](../etc/evidence/t458-s1-vm-runner-presentation-cadence.md)
record the shared caller sweep, benchmark and explicit non-goal: optional turbo
is deferred because it changes guest wall-clock pacing. No profile-specific
display path remains in scope.
