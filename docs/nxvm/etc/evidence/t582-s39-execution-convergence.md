# T582 S39 — execution and timing convergence

## Repeated short-branch outcome

The 486 nominal timing evaluator evaluates the old flags for a short
conditional branch. The later generic formula-input capture identifies the
same branch and evaluates its old flags again to publish the branch outcome.
Both consumers are required by their current contracts; removing the second
result would lose observable timing metadata. A shared same-instruction fact
could eliminate duplicate work, but its scope must also account for other CPU
profiles, zero-displacement branches, faults, preview and interrupt-entry
observation. The current code does not expose such a local fact between those
owners without changing their interfaces or adding state.

A scratch-only x64 build counted calls in the original helper and its two
short-branch paths at the private PC110 fixed startup checkpoint. It recorded
911,584 total condition evaluations, 417,301 in 486 nominal timing and
417,301 in the short-branch helper used by generic branch observation. The
run reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5` in
20,730 ms. Counts include the stopping interval, and instrumentation changes
the generated code. This is startup-only; it does not measure the cost of
each evaluation or the later Game workload.

The duplicate is real but applies to about 4.2% of this instruction sample;
its removal alone cannot plausibly establish the order-of-magnitude speedup
needed for the stated PC110 goal. Do not add persistent mirrored branch state
or a second timing path on this evidence. No production source, public API,
owner media or artifact changed. Log SHA-256:
`F41A492DA5BBFF513FE73A5336D584201F333C57923E29D4780BDA72C9A2991C`.

## 486 timing-profile selection order

The shared timing selector tests three Intel profiles before selecting 486.
A scratch-only candidate moved the existing 486 branch ahead of those three,
leaving the V30 route, actual timing evaluator and all formulas unchanged.
No product-specific predicate, second selector or persistent state was added.
The candidate was not qualified for all profile and failure paths.

For baseline/candidate/candidate/baseline at the private fixed startup
checkpoint, x64 took 18,259 / 20,526 / 18,381 / 20,330 ms and x86 took
23,449 / 24,826 / 21,679 / 22,000 ms. Every run reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`. Both widths change direction
between ordered comparisons, and checkpoint equality is not full-state or
profile equivalence. These noisy samples do not justify adopting the reorder
or imply a product speedup. No production source or artifact changed.
Log SHA-256:
`7161638B4A31F04A1212199778BA41E32D8B93B843608457517EF4C41D5A7C55`.

## Disabled instruction-breakpoint scan

The instruction breakpoint matcher, like the already optimized data matcher,
scans four slots even when all DR7 enable bits are clear. Its caller uses the
matched-status mask only when at least one enabled comparator caused a trap.
A scratch candidate returned after initializing that mask when all enables
were clear, while preserving the entire original scan if any was enabled.
The existing comparator test covers all enabled and matched masks for both
instruction and data cases on 386 and 486, but the scratch candidate was not
run through that test or the full receiving gate.

At the fixed startup checkpoint, baseline/candidate/candidate/baseline times
were 19,209 / 19,611 / 18,973 / 18,213 ms x64 and 22,525 / 23,351 /
23,487 / 25,012 ms x86. All runs reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. The candidate was slower in both x64
pairs and x86 changed direction. Host variation is significant, but the
observed potential regression does not satisfy the owner's criterion, so the
candidate was not applied. This is not full-state equivalence or Game testing.
Log SHA-256:
`4A985876116EAD883F8748865E4886D2AF375BF68CD34361157BE3F3AD93B7FB`.
