# T390 S34: Model-40 Physical-Retirement Scope Correction

## Reproducer and owner boundary

The physical retirement contract is a generic Core publication guard: it blocks
a successful retirement only when its source timing is unallocated.  It does
not encode a firmware semantic checkpoint.  S28 selected that contract in both
Model-40 construction paths after accepting only the finite C0 batch.  S33
then observed a later post-C0 successful, source-classified retirement.  That
combination meant later classified forms could publish elapsed and device time
without their own admitted corpus qualification.

The defect is therefore premature VM-composition selection, not a Core CPU
classifier defect.  Adding a VM checkpoint callback or mutable stage state to
Core would violate the composition boundary and still would not establish a
new timing row.

## Correction and proof

Both Model-40 construction paths now select the existing deterministic
retirement contract.  Core retains ownership of generic deterministic and
physical publication; VM only selects the safe existing contract while the
later corpus remains unqualified.

The private Model-40 composition regression proves the actual constructed Core
uses deterministic retirement and that a synthetic source-unallocated
successful form retires only under the deterministic contract.  The public
BYOB composition regression proves its configuration also selects deterministic
retirement without exposing the opaque Core handle.  The existing generic Core
physical-contract regressions remain unchanged: S34 does not weaken their
nonpublication behavior for a future properly admitted physical consumer.

A fresh current-source build emitted `vm-0-5-0390` with SHA-256
`AF45AB7BF8D76CBFAD2EEE1C53BB9710CF408997CA9C78948196EE880AB140F8`.
The full current CTest gate passed 284/284.

## Similar-issue sweep

The complete Model-40 session-construction search found exactly the two
physical-contract selections, one public BYOB path and one private path; both
are corrected.  The generic Core physical-contract tests intentionally retain
physical selection because they prove the generic contract itself, not the
unqualified Model-40 profile.  No public interface, CPU execution path,
source-timing row, device timing, asset, media, or raw trace changed.

## Transfer

A later T390 subtask may select physical retirement only after a complete
admitted corpus has an enforceable qualification boundary; source-classified
status alone is insufficient.  Until then, CPU/source-timing work continues
under deterministic retirement, and DeskPro board-time/L3 work remains
blocked.