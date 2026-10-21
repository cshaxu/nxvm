# T7 S56 — COP Provider-Role Classification

## Decision

S55 could prove only that a selected COP provider returned a terminal status.
The local COP implementation has several existing range roles which reject a
read or write with `LIB_STATUS_INVALID_STATE`; status and direction alone do
not identify a safe receiving owner.

The existing COP range-role enum is therefore the one safe discriminator.  It
is now exposed only as a copied provider-failure category.  The Core observer
still supplies its physical span solely to the local COP callback; that callback
uses its existing range lookup and retains only the role.  No address, offset,
command word, register, byte or trace is copied into the public observation or
the external probe.

## Static Inventory

The local read owner succeeds for the existing read/write, random, result and
command roles.  Other range roles intentionally remain explicit failures until
they have a separately evidenced behavior.  Command writes also explicitly
fail for an active pending operation, an invalid width/alignment, or an
unclassified command.  Consequently, a command-role failure is an owner
selection, not permission to make all command writes succeed.

## Verification

The Board unit now proves that separate configuration and unmodelled-register
read failures preserve their respective safe roles, access direction and
status, through the installed X86 Core route.  The complete repository-only
MyRaidenII unit set passed on both widths (11 executables per width).

One bounded registered owner-ROM wrapper invocation per width reached the same
safe `command` role.  The x86 invocation additionally established a write with
`LIB_STATUS_INVALID_STATE`; the x64 role result is sufficient for the
cross-width role agreement required by this packet.  The wrapper-created
staging child was empty after both invocations.  No raw probe output or guest
field is retained.

## Transfer

The next receiver is a COP command-admission audit.  It must distinguish the
existing command-local failure categories through a safe semantic category
before changing command behavior.  It must not turn an unknown command,
misaligned write or pending-operation conflict into a generic successful
command, and it must not infer a command value from external media.
