# M5 T372: IBM 5170 Model-L3 Closure Audit

## Task Record

T372 audits the selected IBM PC/AT 5170 Model-339 baseline after T366--T371.
It may only classify readiness or return gaps to their earliest owner; it never
substitutes an audit for missing source-backed phase implementation.

## Active Progress

### S1: readiness matrix

S1's [matrix](../etc/evidence/t372-s1-model339-readiness-matrix.md) finds the
selected baseline not ready: logical ownership/replay is evidenced, but every
board-specific phase conversion remains unallocated.

S1 is accepted at `67ec5177`. S2 must qualify primary-source gaps and any
eligible same-profile reference-model observation before selecting a value.

### S2: reference-model admissibility

S2's [admissibility matrix](../etc/evidence/t372-s2-reference-model-admissibility.md)
finds no current numeric reference candidate: 86Box corroborates only the
512-KB board limit, MAME's 8-MHz default retains incompatible memory and an
undetermined ISA clock, and PCjs documents candidate controls but no
revision-pinned complete observation.  Physical phase work therefore remains
open; S2 is accepted at `8a5431fd` and makes no 5170 model-L3 claim.

### S3: PCjs observation qualification

S3's [qualification record](../etc/evidence/t372-s3-pcjs-observation-qualification.md)
finds that public PCjs configuration documentation is insufficient for a
revision-pinned, policy-compliant physical observation: runnable routes require
third-party firmware and the field-upgrade path requires DOS/guest media. S4
corrects the initial overbroad media wording: local media may be a separately
authorized, uncommitted research input, but S3 had no such contract and stops
before running or tracing PCjs. Physical evidence work transfers to the next
T372 continuation. S3 is accepted at `ad90f474`.

### S4: primary-fact ledger

S4's [primary-fact ledger](../etc/evidence/t372-s4-model339-primary-fact-ledger.md)
corrects the local-media boundary and confirms that Type-3 board and controller
documentation supplies no direct Model-339-to-project-clock conversion. No
physical scalar or L3 claim is admitted; the next continuation must seek
board-specific timing evidence or an authorized isolated experiment. S4 is
accepted at `aa967d2c`.

### S5: local-ROM experiment readiness

S5's [readiness record](../etc/evidence/t372-s5-local-rom-experiment-readiness.md)
contains the owner-authorized external-ROM containment and Model-339
configuration gate. It establishes no reference observation, scalar or L3
claim; a later S must provision a revision-pinned reference runtime. S5 is
accepted at `9198fda5`.

### S6: reference-runtime non-admission

S6's [non-admission record](../etc/evidence/t372-s6-reference-runtime-nonadmission.md)
finds that the owner-authorized external reference runtime did not produce an
observable end-to-end run for the selected configuration.  The owner directs
that it no longer be used.  No trace, scalar, board fact, or Model-339 L3
claim is admitted; existing primary-source and project-probe receivers remain
open. S6 is accepted at `9330c7f6`.

### S7: Bochx bridge qualification

S7's [qualification record](../etc/evidence/t372-s7-bochx-bridge-qualification.md)
accepts the isolated bounded-experiment gate but finds that the historical
paired-step adapter depends on retired NXVM interfaces.  A later optional
developer-tool migration may restore it; no bridge execution, platform fact,
timing scalar or Model-339 L3 claim is admitted here.
