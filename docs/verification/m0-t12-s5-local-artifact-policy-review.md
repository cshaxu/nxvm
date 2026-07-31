# M0 T12 S5 Local Artifact Policy Review

## Result

Completed runnable subtasks now place an ignored local Windows executable in
`build/output/` as `ntvdm64-m<M>_t<T>_s<S>.exe`. Verification records identify
the source commit, SHA-256, and whether the output is a developer baseline or a
product artifact. Design-only work creates no artificial executable, and local
artifacts cannot bundle protected media or serve as release evidence.

The first artifact is M1 T2's NXVM baseline developer executable, recorded in
`docs/verification/m1-t2-s1-nxvm-baseline.md`.
