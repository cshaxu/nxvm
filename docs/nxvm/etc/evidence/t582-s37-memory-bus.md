# T582 S37 — memory and bus hot paths

## Cache-fill proof with unrelated aliases

`memory_query_cpu_cache` already rejects a mapping intersecting the 16-byte
candidate line. The later `memory_is_plain_span` rejects any nonempty mapping
list, including mappings just proven disjoint, and therefore falls back to
bytewise route queries. A more selective proof might avoid that repeated work
when address decoding is unchanged and all providers are also proven
disjoint; any implementation would still have to establish all alias, raw
provider, span, backing and failure semantics. This is an identified possible
general improvement, not an admitted code change.

A scratch-only counter instrumented the actual x64 PC110 cache-fill query at
the private fixed startup checkpoint. It recorded 2,566,175 fill probes and
zero with a nonempty mapping list or address-clear mask; thus zero reached the
disjoint-mapping opportunity. The run completed ten million instructions in
20,454 ms, at 79,964,294 ticks and PC `0003dbc5`. Every query was instrumented,
so this is a frequency observation rather than an unperturbed performance
measurement. It does not establish Game-stage behavior, x86 behavior or other
machine configurations. It does not justify changing production for the
current PC110 startup objective. No source, media or release EXE was changed.
Log SHA-256:
`1E6B586016FDE001B25423E225402C981B61E63F70533B5E5B0025B35B54B580`.

The first scratch compile failed because the diagnostic declaration preceded
the Types include; it produced no measurement. The fixture was corrected and
the completed run above used the normal execution path.

## Multi-byte ordinary-RAM span frequency and sampled cost

The ordinary RAM path resolves its provider first and, for multi-byte access,
separately checks whether the whole span is free of address masking, aliases
and intersecting providers. A scratch counter on the fixed x64 PC110 startup
checkpoint observed 133,126 multi-byte ordinary-RAM reads, 133,124 with a
plain span; and 5,289,464 writes, 5,273,080 with a plain span. These counts
include the stopping interval and duplicate the proof for measurement, so
elapsed time is not a baseline measurement. The large write count reflects
the startup workload and is not a Game-stage count. Log SHA-256:
`8C82C8D43BAE9F9FF16797EAF2574180193F0A15EE7A3B585A7179C6DABFEEB4`.

A second scratch build sampled the existing span proof one time in 256 without
duplicating its result. It recorded 5,422,534 calls, 21,181 samples and
1,576,280 accumulated host TSC units, roughly 74 units per sampled call. It
reached the same ten-million-instruction / 79,964,294-tick / `0003dbc5`
checkpoint in 18,647 ms. Deterministic sampling, non-serialized TSC and the
extra instrumentation limit interpretation; no host TSC-frequency conversion
or precise whole-program percentage is claimed. The proof is frequent but
small, and folding its partial-overlap checks into route selection would also
add work to routes that do not reach ordinary RAM. Do not replace routing or
add a provider-bound cache merely from call count. Game, x86 and complete
provider-state behavior remain unmeasured. No production code or asset changed.
Log SHA-256:
`655375B4F6A48F1CEC519D02B521242CC3519F395D745BC21DE0E636BBC9A276`.

## CPU write snoop frequency

An x64 task-only wrapper sampled the existing Machine CPU-write snoop callback
at the same fixed PC110 startup checkpoint. It counted 6,634,436 calls; all
matched the existing CPU-owned transaction/range check that returns without
invalidating the CPU's own write. One in 256 produced 25,915 intervals with
1,563,830 accumulated host TSC units, roughly 60 units per sampled call. The
run reached ten million instructions, 79,964,294 ticks and PC `0003dbc5` in
19,996 ms. Counts include instrumentation and the stopping interval; TSC is
not serialized and no exact whole-program percentage or Game/x86 conclusion
is supported.

This does not make `cpu_access` a safe substitute for transaction ownership:
debug and firmware writes also pass it, while aliases and provider side effects
may require snoops at a different physical address. Eliminating a callback
would require explicitly preserving the exact transaction range and all those
owners. The measurement does not justify propagating a new suppression flag
or weakening invalidation as a PC110 usability fix. No production code or
artifact changed. The first scratch command failed to write the script because
the host PowerShell lacked a requested parameter; it was corrected before the
completed run. Log SHA-256:
`E9BD39568FFF8E1D10675F917BD4799D52BDD0A7EF29EA5E2D6E93BFB97DFAE9`.
