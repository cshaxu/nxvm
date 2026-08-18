# M5 T421 DeskPro D4 Speaker-Line Proposal

## Purpose

Implement the original-source-defined DeskPro port-`61h` speaker gate/data
behavior through one shared Core PC/AT mechanism, then prove Model-40 D4 and
IBM 5170 planar composition select that same mechanism.

## Boundary

The work covers only a logical port-B/PIT-channel-2 output and copied
observation. It excludes host audio, waveforms, physical timing, PPI breadth,
firmware/media import and VM-side port providers.

## Completion standard

The task closes with the source truth table, Core ownership, profile-selection
proof, reset and transition tests, shared 5170 regression, documentation and
an explicit transfer for host audio and physical behavior.