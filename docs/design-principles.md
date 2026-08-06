# Design Principles

Use the public [Architecture Governance skill](https://github.com/cshaxu/skills/blob/main/architecture-governance/SKILL.md)
for general layering, ownership, contract, and abstraction discipline. This
document records decisions specific to ntvdm64; the detailed enforceable source
boundary remains [Module Layout](architecture/module-layout.md).

1. Keep `nxvm.exe` and `nxvdm.exe` as the executable products. `core` is their
   shared foundation; `mantle` is the future VDM composition component; the
   independent owned `dos` component provides the default NXVDM backend.
2. Keep `core` product-neutral. `core/utils` is a strictly neutral lowest
   utility layer; `core/machine`, `core/platform`, and `core/product` are
   independent peers. Only VM or VDM composition integrates product peers.
3. Keep `dos` independent of all other components. `mantle` composes core
   capabilities for multiple DOS-runtime implementations; `vdm` supplies the
   NXVDM product boundary over mantle and dos. Profiles describe topology and
   bounded firmware overrides.
4. Preserve established NXVM Console, debugger, startup, and full-PC behavior
   during structural work. A user-visible change requires explicit approval and
   before/after evidence.
5. Admit NXVDM compatibility only through a focused probe and real program
   behavior; aspirational CPU, DOS, device, or graphics claims are not proof.
6. Protected media, Microsoft components, external firmware, invasive
   integration, and research tools are never implicit runtime dependencies.

See [System Architecture](architecture/overview.md),
[Source Policy](source-policy.md), and [Debt Tracking](debt-tracking.md).
