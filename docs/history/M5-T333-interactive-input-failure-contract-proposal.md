# Retained T333 Proposal: Interactive Input Failure Contract

This is the retained proposal companion for T333. The approved scope was to
repair EOF/read-error and command-argument-allocation failure handling for the
retained VM Console and core debugger as one interaction-lifecycle mechanism,
without a generic input framework or public ABI change.

The original candidate proposal, detailed S1--S3 plan, non-goals, acceptance
conditions, and xasm-debt separation are preserved in commit `68745ea6`.
T333 completed that plan; the closure record is
[M5-T333-interactive-input-failure-contract.md](M5-T333-interactive-input-failure-contract.md).
