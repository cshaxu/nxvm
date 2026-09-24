# M4 RDP Text Input

## Outcome

Core accepts KVM text records for mapped controller characters. Return/linefeed
produces Start; WASD/J/K produce their normal controller action. This covers
RDP and virtual-keyboard paths that emit text without a key transition.

## Boundaries

Lib remains the neutral input producer. Core owns the NES controller mapping.
Physical key transitions retain their source-owned held state, including
left/right modifier tracking. Every mapped make, whether physical or text,
also queues its button as a controller-protocol transient: it remains visible
until the next `$4016` falling-strobe capture, then releases. This guarantees a
short RDP make/break is not lost between NES samples; physical held input still
remains live until its real release. This is not an executor slice or CPU-cycle
duration. Physical and text records share the same button mapping.

## Acceptance

A Core/controller test injects each mapped text scalar and every physical
binding including `KVM_KEY_ENTER`, observes its button in the next controller
capture, and proves it is absent in the following capture. It also proves a
physical held key survives repeated captures until its explicit release.
Existing RDP virtual Enter and physical keyboard regressions remain green on
x64/x86.

## T28 S1 Corrective Design

The former fixed cycle pulse treated Return differently in practice because its
visibility depended on the timing of an arbitrary Core run slice. T28 S1 removes
the text-pulse fields and their cycle expiry. `core_controller` keeps a separate
transient-button mask alongside held buttons. On the normal strobe falling edge,
it captures held-or-transient input, then clears only transient input. Core
Driver registers a transient for every mapped make and retains source-owned
held buttons until release; no UI frame publication, presenter mode or guest
pacing path participates in input delivery.
