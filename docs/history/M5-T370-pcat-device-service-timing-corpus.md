# M5 T370: PC/AT Device Service-Timing Corpus

## Task Record

T370 follows the closed T369 logical bus-stage package. It owns the
source-backed command/service timing corpus for every selected IBM PC/AT 5170
Model-339 baseline device that makes externally visible progress. It neither
claims a physical clock nor closes the 5170 L3 baseline.

## Active Progress

### S1: Selected device-service owner and source ledger

S1 establishes the exact per-device command, state, DRQ/IRQ, acknowledgement,
reset/cancellation, clock-domain and consumer inventory before a timing value
or state-model repair is admitted.

S1's [owner ledger](../etc/evidence/t370-s1-pcat-device-service-owner-ledger.md)
accepts no timing value. It assigns the four bounded device groups required
before later implementation work and retains all physical-clock conversions as
explicit transfers.
