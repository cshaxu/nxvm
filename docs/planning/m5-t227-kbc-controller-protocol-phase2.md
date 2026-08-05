# M5 T227: KBC Controller Protocol, Phase 2

## Scope

T227 extends the T226 byte path with a deliberately bounded 8042/keyboard
command contract. It owns no host input queue and adds no AUX mouse/IRQ12.

## Contract

- The admitted keyboard command subset is `ED` LED write, `EE` echo, `F0`
  scan-set query/select, `F2` identify, `F3` typematic write, `F4`/`F5`
  scanning enable/disable, `F6` defaults, `FE` resend, and `FF` reset.
  Each command's ACK/response order is port-probed. Invalid input returns
  `FE`; no unsupported command is silently accepted.
- The default profile has a set-1 physical mapper. Therefore `F0 00` queries
  set 1 and `F0 01` selects it; set 2/3 are explicitly rejected with `FE`.
  The controller command-byte translation bit is retained and observable
  through `20h`/`60h`, but it performs no invented conversion while the
  admitted physical set is already set 1.
- The KBC owns pending command parameters, active scan set, translation flag,
  LED state, typematic configuration, and elapsed-tick deadlines. Profile and
  platform never inspect or mutate them.
- Typematic initial/repeat and bounded command-response deadlines advance only
  through the core elapsed scheduler. Their frozen values originate in the VM
  profile and are copied into the one core-machine KBC instance at
  construction. The default PC/AT profile retains zero command-response delay
  until a calibrated hardware value exists, preserving ROM startup behavior;
  the owner-local probe exercises a nonzero delay. Host `Sleep` is a test
  watchdog only.
- Every command response, error, and delay has a focused owner-local port
  probe. Unimplemented commands respond with the documented RESEND/error value
  rather than impersonating wider compatibility.

## Evidence

## Subtasks

- **S1:** complete. The contract admits only the documented set-1 subset;
  non-set-1 conversion and AUX remain non-goals.
- **S2:** complete. State lives exclusively in `core/machine/kbc`; the core
  scheduler advances typematic and optional command response deadlines while
  host ingress and ROM BIOS boundaries remain unchanged.
- **S3:** complete. `core-machine-kbc-controller-smoke` covers command bytes,
  set query/rejection, LED/typematic, scanning enable/disable, defaults,
  resend/reset, delayed responses, and a full typeahead FIFO with preserved
  command responses. `vm-dos-prompt-smoke`, `vm-dos-keyboard-smoke`, retained
  `EDIT.COM`, Console/debugger, and `current-gates-gcc` pass 59/59.

Artifact: `build/output/nxvm_0_5_0227.exe`, SHA-256
`9DE95F8E2CC55B554404D5EC9B170526E5F983F85E03A3E4FBFD5F5F39374E65`.
