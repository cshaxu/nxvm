# Invasive Windows Integration

## Current Policy

Invasive Windows integration is research only. It may have design notes, risk
analysis, isolated prototypes, and prioritized TODOs. It does not enter the
default build, runtime dependency graph, installer, or release package.

Examples include INF installation, registry changes, file associations, shell
extensions, Explorer integration, compatibility-layer registration, and any
loader-facing mechanism.

## Prohibitions

- Do not modify Windows system files or protected components.
- Do not replace the system loader, inject globally, or require a kernel driver.
- Do not weaken system security settings or alter other processes.
- Do not perform a system change automatically.

Any future proposal needs an owner-approved decision record, threat model,
Windows-version matrix, reversible installer/uninstaller design, rollback plan,
and redistributability review before it can move beyond research.

Any invasive/component research task must first inventory host ABI, loader,
WOW64, CSRSS, and ConHost relationships. It must not force changes to Machine
or DOS boundaries.
