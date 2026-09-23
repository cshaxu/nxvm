# Refresh The Shared Four-Root Import

The owner admitted a final M1 source refresh on 2026-09-20. Copy only immutable
SoftPC Git blobs at `0fb40f482f5aa81464e46e360d0b3f26853ba0ae`: 91 Lib, 23
Common, 46 Lib-test and 20 Common-test files. Preserve destination mapping
`src/lib`, `src/common`, `test/lib`, and `test/common`; all four roots remain
byte-identical to source. The owner-authorized MIT grant remains applicable.

Review the eleven-path upstream delta, regenerate provenance/manifests only from
the pinned blobs, and run strict C11 aggregate plus independent x64/x86 suites.
No App/Core/NES code, product debug policy, optional x86 component, ROM, guest
media or sibling write is in scope. Stop on a required local source edit, notice
ambiguity or incomplete verification. This refresh supplies the final M1 corpus;
its acceptance triggers M1 closure rather than another shared-feature task.
