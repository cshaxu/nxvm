# Design Principles

These principles guide decisions when a detailed contract does not already
decide the issue. The architecture documents define the enforceable boundary.

1. Keep two real products: bootable `nxvm.exe` and non-bootable `ntvdm64.exe`
   share one foundation, without making either a compatibility afterthought.
2. Keep `core` product-neutral. `core/machine`, `core/platform`, and
   `core/product` are independent; only VM or VDM composition integrates them.
3. Give every mutable datum, host resource, and execution path one explicit
   owner. Prefer object references, providers, and leases over ambient state.
4. Put policy at the product boundary. Core models guest mechanics and shared
   capability; profiles describe topology and bounded firmware overrides.
5. Preserve established NXVM behavior during structural work. A user-visible
   change needs explicit approval and before/after evidence.
6. Admit compatibility by evidence. A focused probe and real program behavior
   outweigh an aspirational CPU, DOS, device, or graphics claim.
7. Design bounded slices. New hardware, DOS, platform, and research work must
   name its non-goals and stop conditions before implementation.
8. Keep distribution clean. Protected media, Microsoft components, external
   firmware, invasive integration, and research tools are never implicit
   runtime dependencies.

See [System Architecture](architecture/overview.md),
[Source Policy](source-policy.md), and [Debt Tracking](debt-tracking.md).
