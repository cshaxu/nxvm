# Keyboard serial endpoint and 8042 flow-control repair

## Purpose

Repair the shared keyboard-input model exposed by Human Test 395: a valid short `dir` then Enter may leave Enter typematic active because the current model can discard a keyboard break sequence while the CPU-visible controller-output path is congested. The candidate replaces the conflated path with a Core-owned serial keyboard endpoint and a separate, CPU-visible 8042 output path. It restores user-visible input correctness without claiming unproven DeskPro-specific buffer sizes or physical timing.

## Evidence and failure mechanism

A Console capture recorded a complete non-repeating `d`, `i`, `r`, Enter make/break stream; each host event had repeat count one. The fault is most likely immediately after cold boot, disappears after the guest has run long enough, and does not occur for isolated Enter. The host normalizer and F9 lifecycle are separate concerns: F9 must remain host-only, and Console/Window must emit only key state transitions.

The current KBC accepts a native sequence only when its single FIFO has room for all bytes. Set-2 Enter break is `F0 5A`. If that FIFO is congested, the Core call fails and VM can lose the break. Enter make then remains active and KBC emits typematic bytes. Retrying a complete VM request against that same CPU-facing FIFO is invalid because typematic can refill it and starve the pending break.

## Required ownership and model

```text
host adapter -> profile maps host key to native Set-1/Set-2 bytes
             -> Core attached keyboard serial endpoint
             -> Core 8042 output arbitration
             -> CPU-visible 60h output/status/IRQ1
```

Core owns the endpoint, native byte order, break parsing, key state, typematic, serial backpressure, 8042 output buffer, 60h/64h status, translation, command responses, AUX arbitration and IRQs. The endpoint owns pending serial bytes; receipt of a complete break cancels matching typematic before those bytes become CPU-visible. The controller pauses endpoint delivery when output capacity is unavailable rather than discarding keyboard-origin data.

VM composition owns ordered host ingress and retries only when the endpoint explicitly declines a complete mapped sequence for bounded endpoint capacity. The keyboard mapper remains translation only. Machine profiles may later select source-backed keyboard parameters but must not special-case 5170 or DeskPro to repair this generic mechanism.

## Functional requirements

- Atomically accept a complete mapped host sequence or report a distinct retryable endpoint-capacity result without changing endpoint state.
- Preserve order between makes, breaks, keyboard responses, controller responses and AUX data through an explicit, tested arbitration rule.
- Cancel typematic immediately on matching native Set-1 or Set-2 break acceptance, even if CPU output is full.
- Route typematic through the endpoint so it cannot overwrite, reorder or starve physical breaks.
- Retain current 8042 translation as controller behavior; endpoint state uses native keyboard bytes.
- Define reset, scan enable/disable, scan-set change, LED/typematic command, delayed-response and AUX interactions with pending endpoint bytes.
- Preserve existing 60h/64h, IRQ1/IRQ12 and command-response contracts unless a separately sourced correction is admitted.

## Implementation sequence

1. Inventory all KBC producers/consumers: native input, typematic, translation, output reads, command responses, AUX, reset and port status.
2. Add private Core endpoint state with bounded native-byte ring and atomic sequence admission.
3. Move native break parsing and typematic state transitions to endpoint admission.
4. Add KBC advance arbitration that drains endpoint bytes to the existing CPU-visible output machinery in order and under explicit capacity rules.
5. Expose endpoint-full separately from disabled scanning, invalid input and controller faults; update VM ingress to retry only that result at its head.
6. Retain the Win32 normalizer, F9 host isolation, focus release and Console session-handoff hygiene; remove temporary traces and invalid controller-FIFO retry logic.

## Evidence and accuracy policy

IBM PC/AT documentation establishes the observable 8042 output-buffer/status and keyboard-controller boundary, but not the precise internal buffer depths or firmware phases of the keyboard, 8042 or Compaq board. Use Intel, IBM and Compaq primary sources for exact claims. 86Box, PCjs, MAME, Bochs and QEMU may support explicitly labeled reference-derived bridge experiments for observable ordering only. Do not claim physical DeskPro L3 timing, an exact FIFO depth or an exact controller microarchitecture without authority.

## Verification

- Core matrix: Set-1/Set-2 make and break, translation on/off, extended keys, typematic cancellation, endpoint-full atomic decline, output-full flow control, reset and keyboard commands.
- 8042 matrix: 60h/64h status, IRQ1, delayed responses and AUX coexistence while endpoint bytes remain pending.
- VM matrix: only endpoint-capacity retries; no loss, reorder, retry spin or stale lifecycle ingress.
- Product matrix: cold start then immediate `dir + Enter`, burst input, isolated Enter, F9/start/restart, Console/Window, focus loss and concurrent window sessions.
- Bridge records against retained 86Box/PCjs behavior where reproducible, with source tier stated.
- Focused CTest, current fast/full gates, documentation governance and actual-diff review.

## Non-goals and stop conditions

Do not import keyboard firmware, ROMs or external source. Do not claim exact physical wire timing, DeskPro-specific capacity or board L3 timing. Stop and create a receiver if work requires unverified 8042 firmware emulation, protected material, reverse Core/VM dependencies or unsourced physical parameters.