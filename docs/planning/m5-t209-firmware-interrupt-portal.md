# M5 T209: Firmware Interrupt Portal

## Goal

Remove the default profile's QDX `F1 <command>` opcode override. Retain the
same temporary self-firmware services through a frozen, profile-registered
private `INT` portal, without changing the default NXVM `80386 + no FPU`
selection, Console, debugger, or full-PC boot workflow.

## Contract

`core/machine` executes architecturally normal `INT imm8` decoding. During the
configuration window, a profile may register a bounded private-vector provider
with a ROM-origin range. Registration freezes with the machine topology. At
execution, core dispatches that provider only when both the vector and current
instruction origin match the frozen declaration. Otherwise core takes the
ordinary real/protected interrupt path and honours the guest IVT.

The default PC/AT profile uses only these internal ROM portals:

| Vector | Service |
| --- | --- |
| `INT F0h` | stop after the boot-failure key path |
| `INT F1h` | BIOS IRQ1 keyboard translation |
| `INT F2h` | BIOS `INT 10h` text service |
| `INT F3h` | BIOS `INT 16h` keyboard service |
| `INT F4h` | HDD sector read helper |
| `INT F5h` | HDD sector write helper |

These are not guest APIs, CPU instructions, IVT overrides, host shortcuts, or
an external-ROM contract. The provider receives the profile-owned context
directly; it must not mutate the raw CPU instruction table or use a singleton
extension-context lookup. It may act only through the existing profile
firmware/device bindings. Core fixes lookup, conflict rejection, origin check,
freeze, and fall-through semantics; profiles only register handlers.

This table records the T209 completion state. Successor tasks retired F1/F3
(T210) and F0 (T211); the current default profile retains only F2/F4/F5 pending
their separately admitted video and HDD work.

## Breakdown

### S1: Contract and probes

Record the frozen registry, private-vector allocation, ROM-origin rule,
standard-INT fall-through, and test matrix. No runtime behavior changes.

### S2: Core registry and dispatch

Add the public configuration-time firmware-interrupt provider contract and
core-owned frozen registry. Decode `INT imm8` once; invoke a matching private
portal before ordinary IVT delivery. Reject duplicate/out-of-window mutation.
Prove an unregistered or non-ROM `INT F?` follows normal IVT behavior.

### S3: Default-profile migration and 8086 regression

Replace QDX firmware assembly, handler table, raw instruction-table override,
and extension-context dependency with registered portal handlers. Keep each
service at its existing firmware/device owner. Add an explicit `8086 + no FPU`
instruction-profile probe alongside retained default-80386 prompt, keyboard,
MEM, Console, debugger, and two-session regressions. The owner-local FDD/DOS
image may be used only as a bounded expected-negative 8086 diagnostic if it
contains a post-8086 instruction.

**Current finding:** the portal-specific 8086 core smoke passes. The full FDD
prompt vector reaches `C1 EA 04` (`SHR DX, 4`) at linear `0000:AA98` after the
portal stage. `C1 /5 ib` is a real 80186+ immediate-shift form, so strict 8086
correctly raises `#UD`; the earlier raw-image `C6 06` observation was not the
runtime guest-RAM instruction. The explicit 8086 FDD case is therefore a
negative compatibility diagnostic, not a DOS-prompt acceptance gate. Retain a
focused 8086-rejects/80186-accepts `C1 EA 04` probe; default 80386 FDD prompt
remains the product regression.

### S4: Closure

Remove the QDX assembler/disassembler form and sources, reject remaining QDX
source residue and VM/profile raw instruction-table mutation with a source gate,
run current GCC/CTest gates, record the evidence, and produce
`nxvm_0_5_0209.exe`.

## Stop Conditions

Stop for any changed default CPU selection, guest-IVT interception, second
interrupt/firmware state owner, mutable registration after freeze, host/device
shortcut, Console/debugger/startup UX change, failure of the default-80386 FDD
prompt regression, or failure of the focused 8086/80186 profile-gate probe.

## Verification Evidence

Applicable rules: core is product-neutral; profile firmware owns its temporary
services; configuration freezes before execution; no forwarding/second state
owner is introduced; retained NXVM UX is unchanged.

- `cmake --build --preset current-gates-gcc --parallel 4`: passed. The static
  T209 closure gate passed and CTest executed 45 current smokes successfully,
  including default-80386 FDD DOS prompt, keyboard, MEM, Console/debugger,
  two-session, focused 8086/80186, and portal IVT-fall-through probes.
- `cmake --build --preset current-gcc --parallel 4`: produced the task-level
  developer artifact `build/output/nxvm_0_5_0209.exe`.
- SHA-256: `A24DCAA1908BD60309D960D7261A315DF027DFB580B81532B8BF6E42668E9A99`.

The artifact is developer evidence only; it contains no guest media and does
not change NXVM's retained Console, debugger, or boot workflow.
