# M5 T329: Task-Transition State Machine

## Scope And Result

T329 closes the bounded Intel 80286/80386 protected task-transition state
machine. S1--S6 establish direct 16-bit and 32-bit TSS entry, task-gate and
far-CALL entry, nested backlink/NT and `IRET` return, fault-order preflight,
and incoming non-null LDT images with task-local `TI=1` resolution.

S7 completes the retained transition composition without creating a second
paging or debugging model. The 32-bit task planner preflights the source
outgoing-state span (`0x1c`--`0x63`), the incoming state plus TSS debug word
at `0x64`, and all descriptor/cache state under the outgoing task's CR3. Its
non-faulting commit writes the outgoing TSS image and descriptor changes,
then publishes incoming CR3, CPU state, caches, TR, and `CR0.TS`. A set TSS
debug bit invokes vector 1 after the task state is committed, so its frame
correctly starts at the incoming task EIP rather than reverting to the source
instruction boundary.

## Verification And Artifact

`core-machine-task-switch-smoke` proves the accepted S1--S6 matrix plus:

- source-CR3 preflight and outgoing-CR3 image capture, incoming CR3 commit,
  and target instruction fetch through a distinct target page mapping;
- missing target-TSS page delivery through vector 14 before any outgoing TSS,
  busy descriptor, TR, LDTR, or CR3 publication; and
- one post-commit TSS debug trap through vector 1, with the target EIP/CS/FLAGS
  restart frame and target handler progress.

The focused marker is `M5:T329:S7:TASK-PAGING-DEBUG:OK`. Fresh GCC
configuration, documentation governance, `git diff --check`, and direct
current-gate CTest execution passed 211/211 tests. The aggregate wrapper was
also started, but its host command timeout produced no failing test; direct
CTest is the recorded runtime gate.

Commit `ae91e592` produced current developer artifact `vm-0-5-0329` at
`build/output/nxvm_0_5_0329.exe`. Its SHA-256 is
`87982567ACDAC83253A8F6102330F5976B150F2C5AD3CB926D0D6BE4AA41C069`.

## Transfers

T329 does not close generic page-walk/TLB policy, ordinary debug-register or
breakpoint behavior, VM86 task breadth, VME/PVI, TSS I/O bitmap behavior,
arbitrary task chains, failed-double-fault reset policy, or x87 execution.
Those remain in their named architecture-state, Queue, or external
coprocessor boundaries. The closure map records the bounded task-state
completion and these explicit transfers.
