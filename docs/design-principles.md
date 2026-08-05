# Design Principles

Use the public [Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md)
for general layering, ownership, contract, and abstraction discipline. This
document records decisions specific to ntvdm64; the detailed enforceable source
boundary remains [Module Layout](architecture/module-layout.md).

1. Keep two first-class products: bootable `nxvm.exe` and non-bootable
   `ntvdm64.exe` share one foundation without making either a compatibility
   afterthought.
2. Keep `core` product-neutral. `core/machine`, `core/platform`, and
   `core/product` are independent; only VM or VDM composition integrates them.
3. Put VM/VDM policy at the product boundary. Core models guest mechanics and
   shared capability; profiles describe topology and bounded firmware
   overrides.
4. Preserve established NXVM Console, debugger, startup, and full-PC behavior
   during structural work. A user-visible change requires explicit approval and
   before/after evidence.
5. Admit ntvdm64 compatibility only through a focused probe and real program
   behavior; aspirational CPU, DOS, device, or graphics claims are not proof.
6. Protected media, Microsoft components, external firmware, invasive
   integration, and research tools are never implicit runtime dependencies.

See [System Architecture](architecture/overview.md),
[Source Policy](source-policy.md), and [Debt Tracking](debt-tracking.md).
