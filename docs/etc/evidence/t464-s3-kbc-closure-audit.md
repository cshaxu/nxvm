# T464 S3 KBC Closure Audit

## Closure Map

The accepted source ledger has sixteen rows. R1--R4, F2/F4--F6 and T1--T4
map to the single `kbc.c` register, buffer, serial, IRQ1, A20/reset and
ownership paths, with focused controller/AUX/serial smokes. F1 and T5 retain
board L3-or-L2 cadence because neither original manual nor the corroborating
models supplies a selected Core tick conversion. F3 serial endpoint timing and
R2 serial error latches retain the same truthful L2 boundary. R3 diagnostic
dump retains L2 because its IBM-defined stream depends on selected 8042 ROM,
RAM and PSW state; F5 consumer phase retains board L3-or-L2 and its six-us
pulse is L4. F7 electrical tables remain L4.

## Owner Review

`t_kbc_data` is the sole owner of command, input, test, output, FIFO and
keyboard state. The new input/test setters update that owner only; they add no
port provider, clock, callback, NMI state, A20 copy, reset route or VM-owned
mirror. Command-byte bit 2 now directly owns status-system visibility, deleting
the redundant `system_flag` state. The command/status path remains one direct
0060h/0064h implementation.

## Required Verification Record

S3 records the exact full-gate count, stripped Release artifact SHA-256 and
actual-diff review after those operations complete. A failure blocks closure.

## Completed Verification

The stripped Release `nxvm_0_5_0464.exe` was built by `current-gcc`; SHA-256 is
`92BF67903AF9F7725F9BFC8182373FDF615DD5026EDA1857C87BF8249912F674`.
The isolated serial `ctest --test-dir build/mingw-gcc-x64 -j1 --output-on-failure`
run passed 294/294 in 113.34 seconds. Its dedicated ignored log is
`build/mingw-gcc-x64/t464-serial-ctest.log`.
