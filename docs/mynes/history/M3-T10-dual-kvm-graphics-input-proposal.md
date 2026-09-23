# Dual KVM Design And M3 Planning

Retained proposal at design-task closure. The owner's later instruction replaces
its initial monolithic implementation scope with concrete design and complete
M3 candidate planning, including M2 architecture/boundary corrections.

## Outcome

Define NTSC PPU/controller/OAM, frame adaptation, equal native KVM and input
contracts, plus four outcome-bearing implementation packages. Register primary
research, delivered-source gaps, finite acceptance domains and proof owners.

## Scope And Boundaries

The [concrete design](../etc/m3-design.md) and
[remediation ledger](../etc/m3-remediation.md) record the deliverable.
No runtime/source/test/build changes, shared import or implementation admission.
The original blanket DMA exclusion is replaced by OAM DMA in M3; APU/DMC, PAL,
new mappers, commercial game qualification and persistence remain excluded.

## Closure

Architecture and roadmap reference the supporting contracts; candidate proposals
and queue agree. Every graphics/input exit and architecture repair has a receiving
owner and direct oracle. Actual-change review and documentation governance pass.
Local commit is sufficient under the owner's explicit no-push instruction.
