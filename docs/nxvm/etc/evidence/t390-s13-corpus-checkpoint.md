# T390 S13: Corpus Checkpoint Boundary

The project-owned Model-40 capture already required linear PC 7C00 for success, but its 250000-retirement cap could be mistaken for an adequate corpus result when callers ignored its nonzero exit. S13 renames that terminal to `retirement-budget-exhausted`, raises the bounded research ceiling to 2000000, and verifies the caller observes the process exit code.

The owner-managed redacted replay reaches a source-timing-unallocated terminal at 264100 successful retirements, before either budget exhaustion or 7C00. The tool returns exit code 1; this is an explicit transfer, not a successful corpus result. The next terminal is an operand-size 32-bit register RCL with CL count and requires its own Intel-row qualification. No external asset identity, path, hash, byte record or trace enters Git.

Full current gates, documentation governance and diff hygiene pass. S13 artifact SHA-256: `
9810495E3258E8395050C55452E4DEC20FD361239F2D9F143076DB2CBC7BA227
`.

## Coordinator Acceptance

Coordinator review accepts the bounded capture correction: checkpoint success remains the only success condition, budget exhaustion is explicit, and the replay exit status is observed. The new unallocated terminal transfers to the next T390 receiver. No physical profile, board timing or L3 claim is accepted.

S14 corrective review accepts the evidence-only repair: malformed control character, trailing whitespace and literal digest placeholder are removed. Documentation governance and diff hygiene pass; this changes no runtime behavior or acceptance boundary.