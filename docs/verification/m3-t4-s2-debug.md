# M3 T4 S2 Debug Boundary Verification

`nxvm-core-machine-debug-smoke` printed `M3:T4:S2:DEBUG:OK` after strict GCC
build. It verified that inspection is rejected before reset, then uses only
opaque Machine APIs for CPU/memory reads, single-step, and finite continue.
No baseline-global, product, platform, or CLI dependency was introduced.
