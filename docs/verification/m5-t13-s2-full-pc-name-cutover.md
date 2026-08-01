# M5 T13 S2 Full-PC Name Cutover

`vm/product/baseline_full_pc.c/.h` was moved with `git mv` to
`vm/product/full_pc.c/.h`. Its C API and CMake owner now use the corresponding
`full_pc`/`nxvm-vm-full-pc` names. This is a name/path-only composition cutover;
the lifecycle, media, ingress, Console, debugger, and executable source set
are unchanged.

GCC built the `nxvm-m5-t12` artifact and focused targets. The finite CPU probe,
FDD-backed execution-context lifecycle, and FDD/HDD full-PC reset-vector smoke
passed against the approved local fixtures. No-media `help/info/exit` retained
the full Console markers, and delayed `debug/q/exit` retained the `-` prompt.
No raw recorder ran.
