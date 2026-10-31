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

## Test-trained PGO recheck and compilation-unit isolation

The earlier test-trained PGO experiment suggested a startup gain but also
produced an x86 `stringop-overread` warning. Repeating its 12 existing
training tests on current source produced five profile files and reproduced
the warning as an error under `-Werror=stringop-overread`: the shared IDT read
contains an eight-byte protected-mode path while `_ser_int_real` supplies a
four-byte vector. Ordinary entry callers select real mode before this call;
this static observation does not prove every possible reentrant state safe or
establish a runtime overread. Warning-log SHA-256:
`4E7A2543014CC7DF7DE0E0450C7ED369E98EB6EBC303E413BD17A0DADB0F5C3D`.

A scratch vector widened to eight bytes and initialized to zero compiled
without that warning after retraining the same twelve tests against the same
scratch source. An initial attempt to reuse the original source profile for
the different scratch path failed missing-profile checks and is not counted
as a successful build. The widened vector is not a production fix: normal,
virtual-8086, fault and timing paths and all CPU profiles have not been
qualified. Successful compilation alone does not authorize PGO.
Training-log SHA-256:
`588FFEEE90FE4AF3A7DC165112FA21DEB57FB21C1EDC3F4F1CA8CDD88ED69B35`.

Separately, current unmodified source was compiled both normally and with the
fresh test-trained x86 profiles, and linked into the same fixed-work harness.
In one sequential order, ordinary / PGO timing selector only / PGO timing
model only / PGO instruction unit only / all five PGO units took 23,059 /
25,954 / 23,669 / 24,220 / 23,480 ms. All reached 10,000,000 instructions,
79,822,202 ticks and PC `00039119`. The unmodified PGO build retained the
compiler warning; no warning was suppressed in a production build. A single
order and different host conditions are not comparable proof of the earlier
speedup or a late-Game result, and matching the checkpoint is not full state
equivalence. These results do not support identifying one profitable unit or
adopting PGO; no production source, build setting or artifact was changed.
Isolation-log SHA-256:
`8C1B5CE8A23777CCDCE0DA5195226225FB2E4BD5B88EA841B0E626E9B7959571`.

## Fixed nominal timing table prototype

A scratch-only 486 nominal-timing prototype moved opcode groups whose
existing nominal cost is always one tick into one 256-byte lookup table,
removing their original range and switch cases rather than duplicating timing
ownership. Dynamic opcode cases, prefix handling, memory penalties and bus
timing remained on the existing path. The temporary source was not qualified
against all CPU profiles, faults or timing observations.

At the private fixed startup checkpoint, ordered baseline / candidate /
candidate / baseline times were 18,628 / 18,342 / 18,588 / 19,803 ms x64 and
24,141 / 25,221 / 22,245 / 24,123 ms x86. Each reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`; this is not complete-state
equivalence. x64 favored the candidate in both pairs, while x86 changed
direction. The candidate adds a data lookup and table; the samples do not
prove a stable cross-width benefit or Game improvement. It was not applied to
production, and no full receiving gate was claimed. Log SHA-256:
`8A40DF8E43D46FE210EF13C19AF07DD2CEC00010980A115D26D1170E58E7AA94`.

The same already-built x86 executables were then launched suspended, pinned to
one host logical processor, and resumed, in baseline / candidate / candidate /
baseline order with fresh private media copies. Fixed-work times were 22,653 /
22,209 / 21,724 / 21,562 ms, all at the same instruction, tick and PC
checkpoint. The direction still reverses between the two pairs. Pinning avoids
CPU migration but not frequency, thermal or background-load variation; it
does not prove equivalent guest state or Game performance. The added table and
lookup are therefore still not justified. No production code or artifact was
changed. Log SHA-256:
`287CD73EAC10C9087878EC5B0B16DFD9ED3DE42BA100A0D6382F83A40722E2E7`.
