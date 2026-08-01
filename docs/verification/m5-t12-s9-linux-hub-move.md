# M5 T12 S9 VM Linux Host Hub Move Verification

Windows MinGW-w64 GCC built `nxvm-m5-t10` and
`nxvm-baseline-cpu-probe-smoke`; the probe reported `#UD(0)` and
`M5:T1:S1:CPU-PROBES:OK`. `git diff --check` passed. The Linux source is not
part of the Windows target, so no cross-platform behavior is claimed.
