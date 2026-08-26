# T477 S2 DeskPro Child Resolver

`M5:T477:S2:DESKPRO-CHILD-RESOLVER:OK`

`M5:T477:S2:DESKPRO-CHILD-NEGATIVE:OK`

## Retained Mechanism

`vm_profile_model40_child_declaration_create` accepts only a supplied
`pc-at-5170` parent and constructs one immutable child declaration. The child
owns exactly `CORE` and `POLICY`: its Core input is the frozen 80386/no-FPU,
1 MiB deterministic Model-40 contract, and policy selects BYOB firmware plus
session media. Resolver-owned inherited device, port, memory, IRQ and DRQ data
remain copied from the parent with their parent provenance.

`vm_profile_model40_child_resolve` creates the existing 5170 declaration and
resolves the child through the same generic resolver. It copies result data;
post-resolution mutation of the source declaration cannot change the result.
There is no session call site yet, so S2 changes no runtime profile path.

## Explicit Exclusions

The declaration does not contain BYOB byte buffers/paths/hashes, D4 backing
memory, mounted FDD/HDD state, media registry bindings, display slots,
debugger/transports or the FDC terminal callback. Those are mutable session
resources with existing single owners. VADP, controller and CPU state remain
Core-owned after a future copied plan is materialized.

## Verification

The existing `vm-ibm-5170-root-resolver-smoke` now proves both the original
root and child identity/parent/field provenance, copied 80386 and 1 MiB Core
data, BYOB/session policy, copy isolation and wrong-parent rejection. The
focused rebuilt executable emitted both T477 markers. The full current gate
and documentation governance are the S2 closure gates.
