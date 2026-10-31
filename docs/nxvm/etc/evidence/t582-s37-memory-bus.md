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
