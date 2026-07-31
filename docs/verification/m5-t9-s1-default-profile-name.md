# M5 T9 S1 Default Profile Name Verification

The built-in NXVM profile implementation, tests, CMake targets, public symbols,
and current architecture/migration references now use `default_profile` rather
than `pc_at`. The profile still models the historical PC/AT-compatible machine;
historical evidence retains that factual label. The GCC preset reconfigured and
built successfully. No runtime behavior changed and no task artifact is due
until the complete M5 T9 task changes a runnable path.
