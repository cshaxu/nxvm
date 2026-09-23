# Execution And Management Reliability

Uses [shared context](../etc/m2-execution-management-context.md). Entry: accepted
[cycle/interrupt refinement](M2-T8-cycle-interrupt-refinement-proposal.md), full CPU and debugger.

## Outcome

Deliver the stable M2 no-graphics MyNes baseline: users can repeatedly replace
cartridges, run/debug/reset/stop and exit with consistent results, bounded failure
behavior and usable verified Windows x64/x86 executables. This package implements
remaining robustness fixes and complete product scenarios; it is not audit-only.

## Coverage And Work

Complete ROM header/length/flag/allocation/I/O cross-products; every command in
each supported completed/pending/error state; preservation after rejected media;
debug cancellation/lease/rendezvous failure; frame-invalid and source cleanup;
sink/wait/join failures and dependency retention; queued/partial cooked input,
single-reader prompt behavior and exit without an extra command. Exercise these
through production Common, not mocks alone. Fix exposed mechanisms at their owner.

Reconcile BUILD/ROM/MAP/CPU-ALL/CPU-ADDR/CPU-ALU/CPU-CTRL/RUN/DBG/APP/LIFE/HOST/
PROMPT against the complete design matrix, with a disposition for every admitted
case. Prior packages' passing proof is retained and rerun where appropriate;
CPU/debug omissions cannot be hidden behind this package's broader title.

## Acceptance And Milestone Handoff

Run repeated insert -> paused inspect -> step -> run -> pause -> failed replace
preserving hardware -> valid replace -> reset -> eject -> quit scenarios. Prove
the terminal host-error shutdown path and startup failures, not just happy-path
loops. All repository suites pass on x64/x86; the shared standalone suites stay
valid with unchanged source identities. Report real cases, product boundaries,
artifact hashes/architecture/version and known exclusions without external ROM
or gameplay claims. Cleanup task-created build residue under Execution.

Exit provides M2 closure evidence against Roadmap and the full acceptance ledger.
If any required class lacks proof, repair the class or leave the package open;
do not transfer an M2 requirement to graphics. M3 is the next milestone receiver
for PPU/controller/equal KVM, but admission and its task planning remain separate.
