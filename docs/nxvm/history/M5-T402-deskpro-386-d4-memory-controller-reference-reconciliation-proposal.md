# DeskPro 386 D4 Memory-Controller Reference Reconciliation

## Purpose

Reconcile the selected original 1986 Compaq DeskPro 386/16 D4 memory-control
surface with the current private Model-40 composition. The work is a bounded
functional hardware repair: diagnostic/control-window decoding, RAM
replacement/compatibility aliases, write protection, reset state and the
cache-control observable. It supplies a clean hardware baseline before a later
board/device timing receiver; it does not claim D4 DRAM-cycle timing or L3.

## Authority And Evidence Tiers

The primary authority is the retained 1986 Compaq D3PE Processor Board Circuit
Descriptions and Technical Reference material. It defines the selected board,
D4 ROM and RAM decode regions, diagnostic aperture and control topology.
86Box's exact DeskPro 386 memory-controller implementation is Tier 2
reference behavior only: it may resolve a readable implementation question
only where the primary material is incomplete, and must be labelled as such.
PCjs generic machine configuration and generic IBM AT behavior are not D4
authority and may not fill a DeskPro fact. No external source, ROM or firmware
is imported.

## Required Scope

Freeze one D4 matrix whose members are: the `80C00000h` diagnostic/control
window and its byte/register subforms; the `E0000h` replacement window;
`FE0000h` compatibility window; `FFEE0000h` high alias; D4 write protection,
parity/diagnostic visibility, reset values, RAM-size/setup behaviour and
external-cache observable. For every member, record original-source evidence,
current owner, focused proof and exactly one disposition: conforming,
implemented/fixed, Tier-2 reference-derived, nonphysical transfer or rejected.

A confirmed shared-Core defect receives its earliest owner and a similar-form
sweep. A Model-40-private mapping or control correction remains in the VM
profile owner and must not create a Core/VM public ABI or profile-specific CPU
path. D4 RAS/CAS/READY/DMA phase, physical cache latency, ISA wait states and
firmware lifecycle remain separate physical receivers.

## Completion Standard

Before closure, the D4 matrix has no unclassified selected functional member;
every confirmed gap has a focused regression and full current-gate result; all
Tier-2 deductions and physical transfers are named; and an independent closure
audit distinguishes functional D4 completion from physical board timing and
Model-L3 readiness.

## Non-goals

No vendor ROM or media import, no generic AT substitution, no source copying,
no new scheduler, no D4 electrical/waveform claim, no general memory-controller
framework, and no DeskPro L3 claim.