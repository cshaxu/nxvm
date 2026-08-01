# M5 T14 S2 Dependency Migration Map

The approved core-machine, platform, product, composition, and profile
contracts were translated into the current source/CMake migration map in
`docs/planning/m5-t14-s2-migration-map.md`. It records the observed source
edges, mixed targets, migration order, gates, and approved stop conditions.

`tools/Verify-DependencyDag.ps1` compares all forbidden include edges to the
checked-in temporary allowlist. It reported 50 known migration edges and no
unexpected or stale entry. CMake's `NTVDM64_VERIFY_DEPENDENCY_DAG` check
accepted the current explicitly named legacy mixed targets and rejects any new
mixed-owner target at configure time. `verify-dependency-dag` runs the source
check as a build target.

Verification on 2026-08-01:

```text
cmake -S . -B build -G Ninja
cmake --build build --target verify-dependency-dag
Dependency DAG source allowlist matches 50 known migration edges.

cmake --build build --target nxvm-core-contract-smoke
build\\nxvm-core-contract-smoke.exe
M3:T1:S1:CORE-CONTRACT:OK
```

No runtime source, NXVM Console/debugger behavior, boot sequence, media
behavior, or task artifact changed. The next active work removes the approved
core-to-VM/default-profile edges under the map's gates.
