# M5 T118 S1: VM Session Convergence

## Result

`vm_session` is now the one real NXVM session. It owns the core machine,
VM-only devices, selected default-profile binding, platform/product contexts,
request transport, and control state. `session_control` owns command state and
execution boundaries; `session_runner` owns the outer guest scheduling loop.

The forwarding-only `full_pc`, default-profile session, runtime-profile
session, and session-model sources were deleted. The profile registry remains
a profile/firmware registration mechanism; it is no longer a second session or
execution path. VM and VDM composition file names are concise within their
respective composition directories.

## Verification

- MinGW-w64 GCC 16.1.0: `nxvm-current-gcc` and the expanded
  `nxvm-current-gates-gcc` passed.
- Migrated direct-session smoke targets passed: session, default-profile,
  full-PC-profile, and default-profile registry checks.
- FDD boot: `M5:T70:S2:DOS-PROMPT:OK` using `D:\\fdd.img`.
- Retained Console `help`, `info`, `exit` script passed.
- Artifact: `build/output/nxvm_0_5_0118.exe`
  SHA-256 `BB3012C4AA68C30D795B05D205B30457139D518F719794A24AB3FA88AB35BF60`.
