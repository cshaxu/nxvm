# M5 T520: Model 40 Session Lifecycle Consolidation

T520 removes the parallel Model 40 construction, rollback and finalization
transaction.  `session.c` is now the only VM owner of the plan, media registry,
display provider, Core machine, presentation mailbox and debugger.  Model 40
retains only copied board topology plus D4/controller plan materialization.

The change deletes the Model 40 storage initializer and rollback rather than
wrapping them.  Default PC/AT, XT and IBM 5170 continue through the same
generic lifecycle.  A fresh x64 Release build passed unit `304/304` and the
external-ROM/media integration matrix `44/44`, including 5170 360 KB/1.2 MB
and Model 40 1.2 MB boot rows.  The final static caller sweep leaves no
Model-40 shared-resource lifecycle path.

One stale KBC unit expectation surfaced in the fresh run.  T519 had already
made keyboard serial input a bounded device-side queue; the test still assumed
those bytes occupied the AUX FIFO.  Its receiver now independently tests the
two real queue capacities; no KBC production code changed.

The stripped optimized artifacts are `nxvm_0_5_0520_x64.exe`
(`8FC114F25384081ECAB9436F2B4EF96ECF6F19B8CCD8B2802845F924A916BF53`) and
`nxvm_0_5_0520_x86.exe`
(`BAEA015F0CC4A922270553825EE6011D3D56BB8B92C7007F8131A58A84FF0927`).

The detailed lifecycle inventory and fresh-gate record are retained in
[the T520 ledger](../etc/evidence/t520-s1-session-lifecycle-ledger.md).
