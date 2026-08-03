# M5 T94 S1: VDM Composition Ownership

## Result

DOS-minimal composition is session-owned for its current scope. Each minimal
composition allocates one wrapper and one DOS-minimal machine; its profile
pointer names the immutable DOS-minimal descriptor. Presentation allocates its
own input queue and holds a non-owning reference to exactly one DOS-minimal
session. It must be destroyed before that session. No VDM source includes or
references VM code, and the VDM static scan found only helper functions plus
the immutable profile descriptor.

No product source change was required. The focused smoke tests were extended
to construct two sessions and prove that input and text snapshots do not cross
between them.

## Verification

- `ntvdm64-vdm-minimal-session-smoke`:
  `M5:T94:S1:VDM-SESSION-ISOLATION:OK`.
- `nxvm-dos-minimal-presentation-smoke`:
  `M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK`.
- `nxvm-dos-minimal-profile-smoke`:
  `M3:T3:S2:DOS-MINIMAL-PROFILE:OK`.

This is a current-scope closure only. VDM has no product run loop, host thread,
or CLI yet; M8/M9 must define those ownership and synchronization contracts.
