# M5 T13 S3 Request-Transport Name Cutover

`vm/platform/baseline_request_transport.c/.h` and its focused smoke were moved
with `git mv` to VM request-transport names. CMake now names the same owner
`nxvm-vm-request-transport`. Queue contents, capacity, lock ordering, copied
keyboard ingress, consumer callback, close/discard behavior, and execution
boundary behavior were not changed.

GCC built the user artifact and focused targets. The request-transport smoke,
finite CPU probe, FDD-backed lifecycle, FDD/HDD reset-vector smoke, no-media
Console, and delayed debugger checks passed. No raw recorder ran.
