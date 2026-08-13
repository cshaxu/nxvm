# Bochx Local Experiment Gate

This directory contains project-owned research glue only. It does not contain
Bochs, Bochs-derived patches, BIOS files, guest media, generated traces, or a
product bridge. The optional target is absent unless CMake receives
`-DNTVDM64_ENABLE_BOCHX_RESEARCH=ON`.

`ntvdm64-bochx-manifest-check` validates the required identity and budget fields
for one bounded CPU experiment. A local manifest is ignored by Git and must
name an owner-local Bochs 2.6 source root plus the historical Bochx patch ZIP.
It checks that the source root identifies itself as Bochs 2.6 and that the
named patch exists; SHA-256 values are declared experiment identity and must be
verified by the operator before use.

Required `key=value` fields are:

```text
schema=ntvdm64.bochx-experiment.v1
bochs_source_root=<absolute local Bochs 2.6 source root>
bochs_archive_sha256=<64 lowercase hexadecimal characters>
bochx_patch_path=<absolute local Bochx patch ZIP>
bochx_patch_sha256=<64 lowercase hexadecimal characters>
nxvm_commit=<40 lowercase hexadecimal characters>
probe_id=<project-owned probe id>
cpu_mode=real_mode
start_linear_pc=0
instruction_budget=<positive decimal>
wallclock_ms=<positive decimal>
no_progress_limit=<positive decimal>
trace_ring_events=<positive decimal>
comparison_mask=<nonempty mask description>
cleanup_owner=<nonempty owner>
```

The command emits `M5:T1:S2:BOCHX-MANIFEST:OK` only after all fields pass.
Applying the local patch and building/running a paired-step bridge remain a
separate bounded experiment with its own divergence report and cleanup record.
