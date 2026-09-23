# M5 T326: Protected Invalid-Opcode Delivery Closure

## Scope And Result

T326 closes the ordinary protected-mode `#UD` vector-6 boundary transferred by
T325 S3. `ExecFinal` now makes an active invalid opcode deliver through the
protected IDT's vector 6 rather than becoming a terminal fault solely because
`EFLAGS.VM` is clear.

The reproducer also exposed an error-frame defect: the shared protected
serializer had requested an error-code slot for every ordinary protected
exception. The local classifier now declares precisely Intel 80386 vectors 8,
10--14, and 17 as error-code exceptions. Thus vector 6 correctly pushes only
restart `EIP`, `CS`, and `EFLAGS`, while retained error-code paths keep their
existing frame contract.

## Verification And Acceptance

The new owner smoke proves a valid protected vector-6 gate for four independent
producer classes: invalid primary opcode, reserved `0F` form, invalid operand
form, and illegal `LOCK` form. Every case proves restart IP, three-dword frame,
handler transfer/progress, IF gating, and no producer publication. An invalid
gate remains a terminal, pre-instruction containment boundary.

The P1 actual-change review found the production change confined to the shared
exception finalizer and protected exception serializer, with a mechanically
enumerated error-code set. Retained VM86 delivery and `#DE/#PF/#MF/#NM/#BR`
routes remain covered by their current owner smokes. Fresh configure, the new
strict-GCC owner target, the specialized verifier aggregate, documentation
governance, and `git diff --check` passed. The complete current gate passed
210/210 tests.

## Transfers

- Real-mode `#UD` IVT delivery remains `TODO(High)`: 41 retained real-mode
  owner smokes intentionally observe a deterministic no-handler terminal
  boundary. A later task must define the IVT policy and migrate the full corpus
  with exact 16-bit frame/restart/IF/TF evidence.
- Debug and breakpoint semantics, task state, VME/PVI, reset policy, and the
  wider trap architecture remain the named 80386DX task/debug/VM86 boundaries.
- No x87 numerical execution, generic IDT redesign, or public interface change
  is claimed by this closure.
