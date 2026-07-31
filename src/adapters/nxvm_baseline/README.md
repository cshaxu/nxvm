# NXVM Baseline Reference Adapter

M3 T1 S2 leaves the provenance-preserved `src/nxvm-baseline/` executable as
the temporary full-PC regression adapter while the shared core receives its
first instance-owned CPU, RAM, and port state. It is intentionally not linked
into `nxvm-core`: the legacy machine remains global and cannot safely be
presented as a reusable Machine instance.

T3 replaces this temporary boundary with an explicit `nxvm.full_pc`
composition. Until then, M1's recorded FDD/HDD checkpoints validate the
unmodified baseline only; core microtests validate the new instance shell.
