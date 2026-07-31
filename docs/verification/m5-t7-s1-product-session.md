# M5 T7 S1 Product Session Verification

GCC built and ran `nxvm-product-session-smoke` with the owner-supplied local
FDD/HDD fixtures. It emitted `M5:T7:S1:NXVM-SESSION:OK` for each boot target.
The session resolves and freezes `nxvm.machine.pc_at_builtin` and its built-in
provider, composes/freeze the project-owned firmware package and media policy,
validates its `F000:FFF0` ROM stub, creates the explicit retained full-PC
execution bridge, observes its `F000:FFF0` reset vector, and tears both down.

The bridge is intentionally still a product-to-baseline-adapter boundary; it is
not represented as a core device or claimed as a completed device migration.
