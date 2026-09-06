# T523 S8 Core Guest-Boundary Owner Ledger

## Disposition

| Former misleading path | Retained owner path | Boundary |
| --- | --- | --- |
| `core/platform/display_frame.h` | `core/machine/guest_display_frame.h` | A Core-derived guest frame value; it is not a native presentation frame. |
| `core/platform/input.*` | `core/machine/guest_input.*` | Core owns the guest-input source and its value transport. |
| `core/platform/presentation_mailbox.*` | `core/machine/guest_presentation_mailbox.*` | Core owns the copied guest-frame publication mailbox. |
| Core platform test targets | Core machine guest-boundary test targets | Tests mirror the sole guest-state owner. |

## One-Way Boundary

VADP and guest input produce Core-owned values. The Core mailbox publishes a
copied guest frame. VM binds those values to `lib/ux`; `lib/ux` alone owns
native presentation and host wake mechanics. Neither side maintains a second
guest frame, input source, mailbox, native handle, or product route.

## Verification

- Focused guest-input, guest-mailbox and VM-to-UX frame tests pass.
- Refreshed auxiliary-mouse, host-cancellation, input transport and opaque
  lifecycle boundary verifiers pass.
- Retired `core/platform` guest-boundary names and symbols have no source,
  test, CMake or verifier consumer.
- Repository-only unit: 311/311 pass in 17.11 seconds.
