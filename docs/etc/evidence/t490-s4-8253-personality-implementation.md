# T490 S4 IBM 5160 8253 Personality Implementation

`M5:T490:S4:8253-PERSONALITY:IMPLEMENTED-PENDING-GATE`

S4 consumes the complete S3 batch at the existing `t_pit` owner. It adds a
two-value immutable shared-PIT personality to `core_machine_config`, validates
and copies it during Core construction, and makes the IBM 5160 declaration
select 8253. `SC=11` reaches the existing Read-Back implementation only for an
8254; for 8253 it returns without changing PIT state. The existing default
initializer remains 8254, and the auxiliary PIT remains its prior 8254 route.

No XT-only PIT, port provider, scheduler, state cache, profile runtime setter
or VM dispatch path was added. `pit.c` changes from 550 to 587 lines and
`pit.h` from 111 to 119 lines; the 37 added implementation lines consist of
the selected-chip type, construction argument, validation and one control-word
guard. The obsolete unconditional `CORE_MACHINE_DEVICE_PIT` 8254 label is
removed.

## Verification

The focused CTest selection passes 5/5:

- `core-machine-pit-8253-smoke` proves an XT-style `SC=11` write leaves the
  programmed 8253 control/count/latches unchanged;
- `core-machine-pit-readback-smoke` preserves the 8254 status/Read-Back path;
- `core-machine-pit-waveform-smoke` retains all mode/GATE/count behavior;
- `core-machine-pit-irq0-s2-smoke` retains channel-0 IRQ0 delivery; and
- `vm-xt-5160-268-profile-smoke` proves the immutable profile selection.

The full configured CTest gate currently stops at
`core-machine-retirement-observation-s3-smoke`. It was reproduced on both the
S4 tree and a clean temporary worktree at pre-S4 commit `04ff4da5`; it therefore
predates this PIT change. The temporary worktree and build tree were removed.
This unrelated current-gate failure blocks S4 acceptance; it is not repaired
inside this PIT unit. Its earliest receiver must be a separately admitted
retirement-observation/current-gate repair after the owner decides its scope.

The retained T490 boundaries are unchanged: XT PPI port-61 board bits belong
to the queued 8255 unit; physical-axis conversion remains L2; chip startup
state remains L1; electrical timing remains L4/out of scope.
