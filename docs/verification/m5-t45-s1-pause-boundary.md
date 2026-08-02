# M5 T45 S1 Debugger Pause Boundary

The full-PC loop now acknowledges pause requests between guest instructions and
retains its execution owner while paused. Explicit, breakpoint, trace, and
single-step reasons share that boundary; `continue` resumes the same loop and
`stop` still terminates it. The retained Console requests an acknowledged pause
before entering the debugger when the VM is running.

Windows GCC, `M5:T45:S1:PAUSE-BOUNDARY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, dependency-DAG, and live-authority gates
passed.

Artifact: `build/output/nxvm-m5_t45.exe`.
SHA-256: `349A8D92ECDFC4EC6026DC02865908B5CB40EC579AB393012D27C4190DB6DB42`.
