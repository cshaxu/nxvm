# DeskPro 386 Model 40 Selected-Device Functional Closure

## Purpose

Close functional completeness for the devices selected by the accepted DeskPro
386 Model 40 capability audit before its board timing is refined. This
candidate follows the Model-40 CPU closure and precedes the DeskPro board bus
and device timing closure.

## Required scope

For every Model-40-selected platform, input, storage, display, interrupt/DMA,
NMI, reset, memory/ROM, and bus-facing component, close the ledgered register,
state-machine, IRQ/DRQ, error/recovery, reset, and consumer gaps. Reuse a
shared repair only after its validation and state-publication semantics match;
otherwise repair the earliest shared owner and prove the Model-40 binding.
Leave board waits, service durations, arbitration, and phase placement to the
following timing candidate.

The selected display is Compaq Enhanced Color Graphics plus Color Monitor. It
uses the shared VADP core through one small, source-backed Compaq personality
contract; no copied VADP core or IBM EGA default is permitted. IBM EGA remains
outside this candidate and belongs solely to current-product device-capability
L3 closure.

## Non-goals and stop conditions

No generic 386 clone, 486/VME/PVI/x87 expansion, unselected adapter, exact
board timing, Windows claim, or firmware/media import. Stop and return a
shared defect to its earliest owner; do not attach a local delay merely to make
an incomplete device appear ready.

## Evidence standard

Require a Model-40 selected-device functional matrix, state/reset and
IRQ/DRQ traces, focused consumer regressions, a shared-owner sweep, and an
explicit handoff of timing questions to the DeskPro board candidate.
