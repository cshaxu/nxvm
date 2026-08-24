# T447 S5 Platform Lifecycle Boundary Evidence

## Scope

S5 seals the stateful Core input/presentation primitives and VM platform
lifecycle objects behind owner-created opaque handles.  It does not change host
policy, async behavior, profile construction, or the retained T445 adapter
lifetime rules.

## One-owner result

`core_platform_input_source` and `core_platform_presentation_mailbox` now own
their allocations and expose only create, submit/publish/capture, and destroy.
The session owns their handles and tears them down once.

VM execution transport, request transport, platform run context/run handle,
host-surface lease, and the Model-339 platform virtual-time accumulator are
likewise opaque.  Their mutable layouts are private to the platform
implementation; composition holds handles and uses bounded operations.  The
existing `vm_virtual_time_source` callback remains a configuration capability,
not a platform-state layout.

The former test route that destroyed a session-owned mailbox was deleted.  The
display-generation rule remains one publication followed by one committed
generation; tests now observe copied frames through the mailbox contract.

## Failure and lifetime review

Every created session platform handle is released by `vm_session_finalize` or
`vm_session_destroy`.  The Model-339 virtual-time owner is included in the
same destroy path after a later initialization failure.  Model-40 and default
storage rollback also destroy a mailbox/debugger already created before a
later failure.  Platform adapters retain their private run-handle reset and
their existing Linux/Win32 resource finalization order.

## Verification

- `git diff --check` passed.
- Public-header scan found no concrete layout for input source, presentation
  mailbox, run context/handle, host surface/lease, request/execution transport,
  or platform virtual-time source.
- Focused CTest passed 5/5: request transport, execution transport, run handle,
  host-surface lease, and session initialization atomicity.
- The final session-initialization regression passed after the virtual-time
  rollback correction.

This evidence proves only S5's stated owner/opaque-lifecycle boundary.
