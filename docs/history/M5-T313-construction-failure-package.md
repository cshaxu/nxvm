# M5 T313: Construction Failure Package

T313 closes the admitted construction-failure boundaries without adding a new
executor, registry, VM lifecycle, product route, or public allocator/debugger
facade. It remains pending coordinator acceptance at S8.

- S2 resolves the configured RAM capacity before one private backing allocation.
  Its deterministic failure seam returns `TYPE_STATUS_NO_MEMORY` with no live
  machine while retained raw `t_ram` fixtures keep their direct allocation path.
- S3 makes directional range registration and create-time device assembly
  transactional under private per-operation allocation failure injection.
- S4 moves RTC publication after both port registrations and clears retained
  FDC/HDC connection/topology state on failed configuration before retry.
- S5 records exact platform-start status in a session-owned outcome and reports
  it at the selected-session Console boundary without treating it as a CPU fault.
- S6 maps every product debugger register/watch enum explicitly before the core
  boundary. Unknown register input is rejected; the retained void watch vtable
  performs a bounded no-op before any core mutation.

- Artifact: `build/output/nxvm_0_5_0313.exe`.
- SHA-256: `19A9E2A898BAC9C1046D994CC891BF3BF151422B95DB42D4F62DC1CDC8F2B752`.
- Focused markers: `M5:T313:S2:RAM-CREATE:OK`,
  `M5:T313:S3:PORT-ASSEMBLY:OK`,
  `M5:T313:S4:CONTROLLER-ROLLBACK:OK`,
  `M5:T313:S5:START-OUTCOME:OK` (Win32 window and Console failure stages), and
  `M5:T313:S6:DEBUG-MAPPING:OK`.
- Closure verification: `51/51` static/governance targets and `145/145`
  current CTests passed under `current-gates-gcc`; the current artifact target
  gate selected `vm-0-5-0313`.
- Product observation: absent. S7 made no product-flow claim and did not run a
  guest or host-window observation.
- S8 corrects the shared port-assembly smoke's evidence publication: it retains
  the single S3/S4 execution flow while printing the S3 marker after its
  range/create/FDC phase and the S4 marker after its RTC/HDC phase. This is a
  test/evidence-only correction; the 0313 artifact remains unchanged.
- Deferred: no new runtime work. T313's next action is coordinator acceptance;
  M6 remains unadmitted and no T314 is allocated by this record.
