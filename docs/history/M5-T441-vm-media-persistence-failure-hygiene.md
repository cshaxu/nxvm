# M5 T441: VM Media Persistence Failure Hygiene

T441 is admitted from the owner-approved queue candidate
[VM media persistence failure hygiene](M5-T441-vm-media-persistence-failure-hygiene-proposal.md).
It repairs the VM-owned FDD raw-image/sidecar persistence failure boundary
without introducing a filesystem abstraction or changing media formats.

S1 owns the complete current save-call sweep: direct single-file HDD saving and
the paired FDD raw-image/sidecar replacement path. Its acceptance evidence
will state every staging, replacement, cleanup and rollback disposition.

P1 `4e240a84` consolidates staging, releases FDD-derived sidecar state on
failure, and adds the focused regression and `0.5.0441` developer artifact in
the indexed
[S1 evidence](../etc/evidence/t441-s1-vm-media-persistence-failure-hygiene.md).

Coordinator review accepted the actual source, test, artifact and evidence
changes: one production staging implementation remains; every current FDD/HDD
caller is reconciled; no public ABI or filesystem abstraction was added; the
focused and fast gates pass; and the full gate retains only the independent
T344 71-versus-75 constructor baseline.
