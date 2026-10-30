# T582 S26 — CPU reference-read destination check

The private `_kma_read_ref` helper invokes `_kma_gpr_destination` on its
output before copying. Its six current call sites pass the instruction
record's `crm` or `cr` temporary, or a local port-output temporary; none is
an intentional guest-register destination. Register writes use the distinct
write helper and must retain the 486 GPR dependency observation. This is a
source-level observation of current call sites, not a proof about arbitrary
aliasing of privately constructed execution contexts.

A task-only candidate removed the destination check only from this read
helper. At the private-media 10,000,000-instruction startup checkpoint,
baseline/candidate/candidate/baseline elapsed times were
18,355/19,107/19,652/19,648 ms on x64 and
24,053/23,564/23,892/23,163 ms on x86. Every run reported 79,964,294 guest
ticks and PC `0003dbc5`. The small and mixed changes do not establish a
reliable gain or equivalent complete state, and the candidate was not tested
for all profiles and dependency observations. Removing one expression does
not reduce source line count or asymptotic complexity; the candidate is not
adopted on this evidence. No production code, tests, binaries or owner media
changed. PC110 usability remains unresolved.
