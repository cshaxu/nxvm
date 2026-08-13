# T341 S4: Ordinary 80386 Debug State And Vector-1 Delivery

## Scope And State Owner

S4 closes ordinary 80386 hardware debug matching as one CPU-execution
mechanism. `ExecIns` owns pre-instruction instruction-breakpoint matching;
the memory-access record owned by the executor feeds post-instruction data
matching; `ExecInt` owns deferred trap delivery and its ordering against a
maskable PIC IRQ; and `ExecFinal` owns synchronous instruction-breakpoint
fault rollback and vector-1 frame delivery. This is deliberately separate
from the retained host debugger's `flagWE` observation path: that host path
may request an execution stop, but it neither reads DR state nor publishes
guest `#DB` state.

The production sweep used:

```powershell
rg -n "DR[0-7]|debug_(match|trap|complete|deliver)|flagWE|flagRE|flagWR|"
  "Exec(Ins|Int|Final)|_e_(intr|except)_n|VCPUINS_EXCEPT_DB|"
  "TASK_SWITCH_TSS32_DEBUG_TRAP" src tests CMakeLists.txt docs
```

It found one guest match/classification path in `cpu_instructions.c`, direct
guest DR transfer in the already accepted `MOV DR` owner, the retained
host-debug observer, TF delivery, and the incoming-TSS task-debug trap. No
second guest DR matcher or vector-1 delivery owner exists.

## Validation To Delivery Graph

| Event | Validation and publication | Delivery and retained proof |
| --- | --- | --- |
| Instruction execution breakpoint | Before decode, `ExecIns` compares the CS-linear instruction address with enabled `DR0`--`DR3`. Only `RW=00` and `LEN=00` are defined execution matches. A match sets the corresponding `DR6.Bn` and takes fault rollback through `ExecFinal`. | `ExecFinal` rolls back to the restart IP, gives the saved flags image `RF`, and uses the existing vector-1 real/protected delivery owner. `core-machine-tf-db-s60-smoke` covers real and protected frames; `core-machine-vm86-delivery-smoke` covers VM86-to-protected delivery. |
| Data breakpoint | The successful memory-access log is inspected only after instruction execution. `RW=01` selects writes and `RW=11` selects reads or writes; `LEN=00/01/11` selects one, two, or four aligned bytes. Any enabled overlapping register sets its `DR6.Bn`; undefined `RW=10` or `LEN=10` is not assigned an artificial 80386 behavior. | Deferred delivery uses the same vector-1 owner after the access has committed. The owner smoke proves write plus TF multi-cause and a global-enable read/write length-four overlap. |
| TF and RF | `ExecInit` snapshots TF/RF before the instruction. A successful instruction with sampled TF adds `DR6.BS`; RF suppresses the next instruction-breakpoint fault and clears after a successful non-IRET instruction. | The sampled TF trap is delivered before a pending maskable IRQ. The real interrupt-gate fixture proves vector 1 is entered and the pending IRQ remains pending after IF is cleared by that gate. |
| Task debug trap | The 32-bit TSS debug word is read and validated by the task transition plan before commit. | After commit it sets `DR6.BT` and uses the established task vector-1 route. A 32-bit task switch clears DR7 local enable/exact bits and retains global bits. `core-machine-task-switch-smoke` proves both facts. |

DR7 local and global enable bits both activate an ordinary breakpoint in this
non-cached executor. A 32-bit task switch clears only the local enable and
local exact bits; global enable/exact state remains. The implementation does
not claim a later-CPU exact-breakpoint timing model, debug-register reserved
bit normalization, VME/PVI, or persistent-cache behavior.

## Focused Evidence

- `current.core-machine-tf-db-s60-smoke` proves instruction faults, write and
  read/write data traps, length-four overlap, local/global enables, RF
  suppression, TF plus data `DR6.B0|BS`, real/protected frames, and TF before
  a pending IRQ.
- `current.core-machine-vm86-delivery-smoke` proves a VM86 execution
  breakpoint becomes protected vector 1 with a restart frame and `DR6.B0`.
- `current.core-machine-task-switch-smoke` proves incoming TSS debug `BT` and
  32-bit task-local DR7 reset without changing global enable/exact bits.
- `current.core-machine-debug-mov-s59-smoke` remains the sole form/privilege
  proof for direct `MOV DR`; it does not claim breakpoint matching.

All four are current-gate entries. The full current gate, documentation
governance, and diff check are required for S4 acceptance.

## Boundaries And Transfer

`RW=10`, `LEN=10`, execution breakpoint nonzero length, reserved DR image
bits, VME/PVI, post-80386 debug extensions, persistent translation cache, and
the host debugger UI are outside this ordinary 80386 mechanism. The first
four are architecturally undefined/reserved rather than untested defined
80386 rows; the remaining items retain their existing Queue or product
boundaries. No provider, public ABI, or artifact behavior changes here.
