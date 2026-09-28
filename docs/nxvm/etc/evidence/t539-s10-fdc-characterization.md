# T539 S10: FDC Pre-Extraction Characterization

Baseline 02931b886. [Decision record](../architecture/t539-s10-fdc-boundary.md)
contains original-manual identity, read-only reference revisions, all fifteen
defined command dispositions, readiness consumers, concrete ownership and
mandatory production-repair gates. FDC is not yet extracted or requalified.

The existing `core_machine_fdc_smoke.c` gains one table-driven matrix: ten
operations times six input combinations times two legacy policies, 120 cases.
All earlier command, seek, DMA/NDMA, reset, error and media cases remain. The
new cases observe ports, command phase, result IRQ release and DMA requests;
they identify legacy behavior, not the future manual oracle. No copied media,
new executable, fixture framework, public API or production branch is added.

The first DRQ check failed because preceding TC tests had masked DMA channel 2.
The fixture now programs/unmasks its own channel for each independent row.
No production code was changed to satisfy that fixture failure. Final-source
full unit runs pass 347/347 on both x64 (20.41 s) and x86 (19.78 s), including
the existing FDC topology/media/Model-40 families and new characterization.

Commands: `cmake --build build/t539-s3/nxvm-x64 --target run-unit-tests -j 8`
and the corresponding `nxvm-x86` tree. The temporary exact test selection is
not a permanent focused-test suite. Documentation governance and changed local
Markdown links are checked before delivery, together with `git diff --check`.

Tracked C/H diff: production +0/-0, tests +106/-0 (net +106), counted with
`git diff --numstat` against baseline. The addition replaces no behavior; it
characterizes a finite cross-product that the old individual scenarios missed.
Eight existing EXEs, INI, external master assets, Shared manifests and other
products remain unchanged because none of their executable inputs changed.
No integration rerun or firmware-boot acceptance is claimed by this S.

Next repair obligations are concrete: pending seek identity/capacity and SIS
admission; READY versus record availability and physical Track0; old DeskPro
workaround removal without an equivalent renamed flag; manual/status/timing
contradictions. These remain within T539 and block its FDC cutover. Completing
this prerequisite does not move those production defects to unrelated debt.

P1 7fa0f75d5 is pushed. Coordinator actual-diff review accepts the finite
characterization, unchanged production/artifacts, primary-source distinction,
ownership contract and explicitly retained repair gate. Documentation governance,
changed local links and whitespace checks pass. S10 closes as a prerequisite;
S11 is automatically admitted for pending seek/completion ownership and safety.
