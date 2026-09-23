# T355 S2: Opt-in Windows HDD checkpoint harness

## Contract

`run-windows31-hdd-checkpoint` is the sole S2 entry point.  It builds the
existing `vm-windows31-hdd-admission-probe` and invokes it only when the owner
sets `PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE` to an existing local file.  The
cache input defaults to empty.  The runner checks that empty value before it
examines either the probe executable or a media path, and checks a supplied
path exists before invoking the probe.

The command is deliberately not a CTest and is not present in any
`PROJECT_CURRENT_*_SMOKE_TARGETS` list.  It therefore cannot enter
`current-gate` through the canonical T344 registration partition.  It records
no input path, hash, bytes, or host timeout in repository content.

## Invocation

An operator may configure a local build with:

```text
cmake -S . -B build/... -DPROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE=<local-file>
cmake --build build/... --target run-windows31-hdd-checkpoint
```

The probe remains a host-observation diagnostic: its result is an HDD/INT13
checkpoint only.  It does not establish guest time, Windows Setup completion,
or broad compatibility.

## Verification record

S2 verifies the default empty configuration fails with the stable
`M5:T355:S2:WINDOWS31-HDD-CHECKPOINT requires` marker, without a media path.
It also verifies a nonexistent explicit value fails before probe invocation;
neither check accesses approved media.  A future S3 may run the command only
with owner-supplied local media under the source policy and must record only
its bounded checkpoint result.

## Similar-issue sweep

The S1 inventory remains authoritative for all four `vm_windows31_*` probes.
S2 selects only `vm-windows31-hdd-admission-probe`; the checkpoint, setup, and
INT13 trace probes remain non-current and unchanged.  The CMake
`PROJECT_CURRENT_*_SMOKE_TARGETS` and media lists remain unchanged, so no
Windows probe is promoted through a transitive current-gate route.
