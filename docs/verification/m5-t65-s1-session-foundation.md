# M5 T65 S1 Session Foundation Verification

- Source commits: `330befa` and `3f354ed`, followed by the T65 startup-stack
  repair recorded with this verification result.
- Windows GCC/MinGW full build and dependency-DAG gate pass.
- `nxvm_0_5_0065.exe` starts the retained NXVM Console and accepts piped
  `EXIT` after moving the caller-owned live-machine graph off the 1 MiB process
  stack.
- CPU-probe, live-machine, debug-target, and retained Console smoke gates pass.
- The owner-provided FDD fixture matched M1 identity:
  `fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5`,
  1,474,560 bytes.
- A 10-second recorder run mounted that FDD, stayed below its 512 MiB budget
  at 150,564,058 bytes, and matched `INT     21`. The watchdog terminated the
  owned VM process with exit `-1`; the trace was closed and deleted.
- Developer artifact: `build/output/nxvm_0_5_0065.exe`, SHA-256
  `7BEB8AB27B1AA2B922E361AF08BC3D3DD755E0AA8D064E8E3C09D2FA95266E8A`.

T65 removes root/full-PC/probe/control selected-session storage. The remaining
legacy CPU, RAM, port, device, profile, and product aliases are explicitly the
T66 through T72 migration scope.
