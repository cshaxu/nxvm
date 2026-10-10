# T548 S29 — Shared Monitor Unit Ownership Ledger

## Scope and method

This review compares monitor behaviors, rather than include graphs or similar
test names, across Lib, Emulator, Product Surface and the current consumer
receivers.  A lower-layer test remains canonical when it proves that layer's
contract directly.  A consumer test remains when it proves an adapter,
extension, machine-specific command or real composition increment that the
lower layer deliberately cannot select.

## Canonical shared receivers

| Layer | Receiver | Canonical behavior | Disposition |
| --- | --- | --- | --- |
| Lib | `console_broker_cancel_smoke`, `console_broker_reader_failure_smoke`, `console_broker_smoke` and `console_blocking_gate_smoke` | Native cooked-reader lifetime, cancellation, failure and blocking ownership.  They do not parse monitor commands. | Retain in Lib. |
| Lib | `console_broker_display_smoke`, `lib_console_io_contract_smoke`, `lib_console_event_gate_smoke` and `lib_console_smoke` | Console buffer/viewport, text delivery, input event and host-console mechanics.  Example monitor-looking strings are fixture payload, not command semantics. | Retain in Lib. |
| Emulator Session | `session_monitor_smoke` | One pending reader, command result delivery, prompt re-arm, delayed monitor text and runtime-completion handoff. | Retain in Emulator. |
| Emulator Session | `presentation_plan_smoke`, `control_state_matrix_smoke`, `session_frame_smoke` and `control_reconciler_integration_smoke` | Monitor/raw-console presentation state, lifecycle reconciliation and copied-frame sequencing. | Retain in Emulator. |
| Emulator Product | `product/monitor_smoke` | Fixed command grammar, no-argument rule, snapshot argument forwarding, fixed-command precedence over extensions, generic unavailable response, canonical help/window formatting and lifecycle text. | Retain as the sole generic monitor grammar/format receiver. |
| Product Surface | `surface/command_smoke` | IBM-PC adapter binds one generic monitor provider to PC Debug, PC hotkey rows and App extension commands.  It does not re-prove parsing or format internals. | Retain as a Product delegation receiver. |
| Product Surface | `surface/entry_smoke` and `surface/keyboard_smoke` | PC process-entry composition and PC hotkey-to-guest-input adaptation. | Retain as Product-specific integration. |
| Product Debug | `debug_output_smoke`, `debug_linear_smoke` and `debug_machine_smoke` | Nested DOS-style Debug command language, prompt and paused-machine lease behavior.  This is intentionally distinct from the top-level monitor grammar. | Retain in Product. |

## Consumer comparison

`test/app-mynes/unit/product/command_smoke.c` calls the same monitor provider,
but its assertions additionally select ROM insertion/ejection grammar,
snapshot persistence, NES Debug admission and NES runtime callbacks.  Its
generic-looking lifecycle checks prove those App bindings through the one
provider; they are not a second parser test and cannot be replaced by the
shared mock provider.  PC-family App tests similarly select their fixed
factory/profile/firmware composition and are outside this monitor batch.

No shared receiver has an equal-or-stronger consumer duplicate, and no
consumer receiver has an equal-or-stronger shared replacement.  Therefore S29
does not move, delete or weaken a unit assertion.  This is the required
one-to-one behavior assignment: each generic contract has one lower-layer
receiver, while each upper-layer receiver proves only its adapter or product
increment.

## Verification and boundary result

The review is documentation-only.  It changes no C/H, CMake registration,
manifest, public interface, production behavior, firmware, media, INI,
snapshot or executable input.  Existing routes and test counts are preserved.
The next S30 review owns MyNES App behavior as a separate product batch;
S31 alone performs the complete dual-width T548 qualification.
