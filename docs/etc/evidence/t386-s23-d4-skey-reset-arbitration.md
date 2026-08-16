# T386 S23: D4-SKEY And Reset Arbitration

`M5:T386:S23:D4-SKEY-A20:OK`

`M5:T386:S23:D4-RESET-ARBITRATION:OK`

`M5:T386:S23:CORE-VM-RESET-OWNER:OK`

## Source Boundary And Functional Contract

The primary Compaq D3PE Processor Descriptions (1987-01-05), inspected
transiently under the source policy, identify D4-SKEY as an 8042 A20-line PAL.
It intercepts the `D1h` command-to-port-`64h` then data-to-port-`60h` sequence
to accelerate LOWA20 changes. Its documented functional output is the same
8042 output-port A20 value; its acceleration and propagation time are board
physical timing and remain outside S23.

The same source defines three reset inputs. A failed processor exception
shutdown asserts the shutdown latch; program-controlled restart pulses the
9042 RSTAR line into that same arbitration; power-up/loss produces the external
cold-reset condition. Pulse width, clock/hold synchronization and power
sequencing remain timing/electrical behavior. S23 implements only the logical
arbitration result: a Model-40 D4 platform consumes a CPU shutdown event through
the existing cold-reset lifecycle. Existing 8042 output-port and pulse commands
continue to request that same lifecycle; externally initiated `core_machine_reset`
remains the power/cold-reset functional entry.

## Owner Decision

Core CPU execution owns failed-double-fault shutdown production. Core Machine
owns cold-reset lifecycle and consumes reset requests. The existing Core 8042
owns `D1h` output-port A20/reset mutation and program-reset pulses. The D4
platform configuration is the Model-40-selected consumer that elects CPU
shutdown-to-reset; it introduces neither another A20 state nor a second reset
path. VM Model-40 composition selects D4 and retains only board identity.

A non-D4 machine retains its preexisting terminal failed-`#DF` behavior. This
is intentional: a CPU shutdown event is not by itself a universal board reset
policy.

## Regression And Sweep

- `vm-model40-d4-skey-s23-smoke` drives actual `64h:D1h` / `60h:data` port
  writes in a private Model-40 session, proves both LOWA20 transitions and
  verifies output-port reset is consumed by the normal Machine cold-reset path.
- `core-machine-d4-platform-s4-smoke` proves a D4-configured machine consumes
  the CPU shutdown event into cold reset, restoring reset EIP and existing D4
  reset state.
- `core-machine-call-gate-privilege-entry-smoke` preserves the non-D4 failed
  double-fault terminal outcome.
- The sweep covers all request/consume sites for CPU reset and shutdown, KBC
  `D1h` output-port handling and pulse commands, Model-40 D4 composition, and
  IBM/default isolation. No test-only product API, firmware/media asset or
  second A20/reset state exists.

## Retained Transfer

D4-SKEY electrical response, RSTAR/reset-pulse duration, hold arbitration,
power-good sequencing, and board clock placement remain DeskPro board timing.
T386 still retains selected 1.2 MB FDC logical behavior, fitted 40 MB fixed
startup media, CECG residual behavior, then its functional-closure audit. This
record makes no firmware execution, physical-media, generic-variant, timing or
L3 claim.

## Verification And Acceptance

P1 verification passed: the three focused regressions passed; the T345 ownership
verification passed with 224 rows (164 owner tests); 60 specialized/governance checks
passed; and the serial current gate passed 277/277. The rebuilt
`vm-0-5-0389` artifact SHA-256 is
`0CB4173F06E91C3BD5F9493497F2EC5480F361A23493ED7A7326000DADFB3F20`.
T345's explicit owner-test and S2 inventory counts were reconciled to the actual
current-gate matrix (164 and 161 respectively); no check was relaxed. P2 records
the independent acceptance and task-state handoff.