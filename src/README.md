# Source Layout

Shared-core code lives under module directories with headers beside their C
implementation. The imported NXVM baseline remains isolated in
`src/nxvm-baseline/` until tracked refactor subtasks move behavior with
provenance and focused tests. Microsoft component research is outside `src/`
until an M11 Go decision authorizes a bounded implementation.
