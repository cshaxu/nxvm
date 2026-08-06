# M5 T244: Registered Undefined-Instruction Transition

## Status

**Complete.** The contract, focused core implementation, current-matrix
evidence, and developer artifact are complete.

## Objective

Allow a future product-owned consumer to register a narrowly bounded reaction
to an architected `#UD`, without reopening global CPU state or establishing a
reserved opcode, selector namespace, BOP, SVC, or DOS interface.

## Frozen Contract

* Only `core_machine` stores registrations. Registration is valid only in the
  `INITIALIZED` configuration window and freezes with execution providers.
* A pattern is 1--15 exact bytes. Registration rejects invalid length, null
  consumer/context, identical patterns, and either prefix relation. Pattern
  order cannot decide an ambiguous match.
* On an ordinary `#UD`, core captures the current instruction-start state and
  its own fetched byte window. No consumer is called when no registered exact
  pattern matches, so existing NXVM `#UD` diagnostics, fault, and stop behavior
  remain unchanged.
* A consumer receives only a copied input: instruction-start `CS:EIP`, a
  copied byte window/length, `EAX/EBX/ECX/EDX/ESI/EDI/EBP`, and `EFLAGS`. It
  receives no `core_machine`, execution context, RAM pointer, port, profile,
  platform, or product handle.
* The consumer returns exactly one of: `UNHANDLED`, `HANDLED_RESUME`, `STOP`,
  or `FAULT`. `UNHANDLED` follows the retained ordinary `#UD` path.
* `HANDLED_RESUME` is core-owned: core consumes exactly the matched byte count
  and atomically applies only candidate `EAX/EBX/ECX/EDX/ESI/EDI/EBP` plus
  `CF/PF/AF/ZF/SF/OF`. It cannot alter EIP directly, ESP, segment caches,
  descriptor state, CRx/DRx, FPU, memory, or CPU mode.
* `STOP` leaves the instruction-start architectural state intact and uses the
  normal core stop result. `FAULT` leaves it intact and reports a supplied
  nonzero detail through the normal core fault result. Core, not the consumer,
  owns both transitions.

## Implementation Shape

`core/machine/undefined_instruction_transition_interface.h` will expose the
copied input, bounded patch, outcome, and registration operation. The registry
will be private core-machine state. CPU decode will consult it only at the
existing `#UD` completion boundary; it will not add dispatch-table entries or
special opcodes. Core machine lifecycle code will bind/freeze the registry and
translate STOP/FAULT to existing run outcomes.

The implementation must capture the same instruction-start physical fetch
window used for matching before any transition. If the current decoder cannot
prove a physical window for an instruction form, it must retain ordinary `#UD`
rather than match a guessed linear byte sequence.

## Verification Plan

S2 must add a focused core smoke proving:

1. unregistered `#UD` is unchanged;
2. valid registration succeeds only before freeze; duplicate and prefix
   conflicts fail;
3. a matching pattern consumes the exact length and applies only the allowed
   patch atomically;
4. forbidden patch fields are rejected without partial CPU mutation;
5. `UNHANDLED`, `STOP`, and `FAULT` preserve instruction-start state and return
   their defined core outcomes; and
6. normal NXVM boot, Console, debugger, media, and current GCC/CTest gates do
   not gain a default registration.

S3 closes the task with the full current matrix and
`build/output/nxvm_0_5_0244.exe` plus SHA-256.

## S2 Implementation

The registry is private `core_machine` state and is bound to the existing CPU
completion path. Only real-mode `#UD` presently qualifies: core re-reads the
15-byte candidate through the physical-memory route before matching, so a
protected/paged fault cannot be matched from a guessed linear window.
`HANDLED_RESUME` advances `EIP` only by the core-written matched length and
masks the patch to the six admitted status flags. No NXVM profile registers a
consumer.

`core-machine-undefined-instruction-transition-smoke` proves retained
unregistered `#UD`, duplicate/prefix rejection, frozen registration rejection,
exact one-byte consumption, flag masking, and STOP/FAULT handoff. It uses only
the core registry and a copied response; it adds no guest service convention.

## S3 Evidence And Closure

The full current GCC gate passed all 34 static/ownership checks and 82/82 CTest
smokes. `build/output/nxvm_0_5_0244.exe` SHA-256 is
`B859AEE01A0561BD5C70777F47A48335923E8D056E028A8CE3E15ABCFFA8BA52`.

## Stop Conditions

Stop for owner direction if implementation needs an opcode reservation, guest
memory mutation, a raw executor borrow, a product/platform callback during a
CPU quantum, a second decode loop, or an unbounded fetch/matching rule.
