# T435 S3: Core CPU Timing Pipeline

## Decision

S3/B0 installs one Core-private instruction-timing execution skeleton. It does
not promote any current timing value to conforming: the S2 manifest baseline
remains 3,295 nonconforming keys until the profile batches supply actual
per-key results.

## Owned execution boundary

`cpu_timing.c` owns the only successful-retirement selection order. Its input
is the already decoded Core CPU state; its result contains the Core decoder
form key, tick result, formula-input bitset, retirement origin and unallocated
state. `machine.c` calls it once per instruction round, adds only the already
committed external-cycle time, and sends both immediate and delayed retirement
through `core_machine_publish_successful_retirement()`.

The publication seam emits the result through the existing copied retirement
observation: `timing_key_id`, `source_ticks`, `formula_inputs`,
`source_timing_form_id`, `timing_origin`, and timing disposition. A classified
result has `source_timing_unallocated=false`; an existing unallocated S2 gap
remains observable and is rejected by physical retirement. S3 does not falsify
that state to make an unfinished key appear conforming.

The individual calculator functions retained in `machine.c` are private legacy
rule evaluators only. They have no priority chain and do not publish a result.
The priority chain, origin assignment and result construction occur only in
`cpu_timing.c`.

## Manifest and result pipeline

`Verify-CpuTimingManifestContract.ps1 -EmitCanonicalKeys` now emits exactly
the materialized inventory consumed by result tooling: 649 8086, 602 80186,
812 80286 and 1,232 80386DX records, 3,295 total. Each record carries its
profile, L3/named-L2 level, key, legal context, source rule, current route,
batch, regression identity and nonconforming status.

The build invokes `Export-CpuTimingManifestCatalog.ps1` to generate that exact
3,295-key catalog into the Core build tree; `cpu_timing.c` compiles it and has
a 3,295-entry assertion. S4--S7 bind decoder forms and context inputs to those
catalog entries; B0 does not fabricate such bindings before the profile's
source rule is implemented.

`Verify-CpuTimingResults.ps1 -ResultPath <file>` consumes that exact inventory
and requires one real result for every key. It rejects missing, duplicate or
unknown keys, absent ticks/formula inputs/form/origin, an unallocated result,
or a failed result. It is intentionally not run as a passing S3 closure gate:
there is no fabricated all-key result file before S4--S7 implement the rules.

## Boundary proof and exit evidence

- `machine.c` has one call to `core_machine_cpu_timing_select()` and one raw
  retirement-observation publication, inside the shared helper.
- `cpu_timing.c` has the sole selector and contains no READY/HOLD, DMA,
  prefetch, device or external-cycle policy. External-cycle accumulation stays
  in `machine.c` before the shared publication seam.
- The migrated static gates verify the new owner while preserving historical
  evidence of the pre-B0 selector.
- The 14 focused CPU timing and retirement regressions pass; the four manifest
  verifiers and shared materializer derive the frozen counts above.

Markers: `M5:T435:S3:CPU-TIMING-SINGLE-SEAM:OK`;
`M5:T435:S3:FOUR-PROFILE-RESULT-PIPELINE:OK`.
