# M5 T192 S3 KBC/BIOS Handoff Verification

## Scope

The sole NXVM host-key route is now platform-normalized event -> session
request boundary -> default-profile set-1 mapper ->
`core_machine_keyboard_submit_scan_code()` -> core KBC FIFO/IRQ1 -> QDKEYB
INT 09h port `0x60` read -> BDA -> BIOS INT 16h. Platform does not choose a
BIOS AX value and QDKEYB no longer accepts a host key directly.

## Evidence

- `core-machine-kbc-controller-smoke` emitted
  `M5:T192:S2:KBC-CONTROLLER:OK`.
- `vm-keyboard-host-ingress-smoke` emitted
  `M5:T192:S3:KBC-HOST-INGRESS:OK`, proving that a host event leaves BDA
  unchanged until the request boundary consumes it.
- Local FDD checks emitted `M5:T70:S2:DOS-PROMPT:OK` and
  `M5:T151:S2:DOS-KEYBOARD:OK` for real DOS `ver<Enter>` consumption.
- FDD/HDD session checks emitted `M5:T7:S1:NXVM-SESSION:OK` and
  `M5:T13:S8:VM-SESSION:OK`; `vm-two-session-isolation-smoke` emitted
  `M5:T73:S1:TWO-SESSION-ISOLATION:OK`.
- `cmake --build --preset current-gates-gcc --parallel 1` passed.
- `nxvm_0_5_0192.exe` printed `Neko's x86 Virtual Machine [0.5.0192]`,
  accepted `EXIT`, and returned zero.

Artifact SHA-256: `A28CF7BD43FABB9D99EA8037BBDEAC6EDA8380E672F37C0C112CAF7003A577D1`.

## Exit Review

T192 has one controller state owner and one host-key route. The closed slice
does not claim break-byte, extended-set, scan-set, translation, AUX mouse,
IRQ12, timing, or native POSIX runtime compatibility; those are explicit
`TODO.md` follow-ups. The historical `MEM` diagnostic smoke still has its
separate running-boundary race and is not used as T192 keyboard evidence.
