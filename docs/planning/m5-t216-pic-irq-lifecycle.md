# M5 T216: PIC IRQ Lifecycle

## Goal

Give every device IRQ one explicit PIC-owned lifecycle. A device owns only its
source signal; the core 8259A pair owns IRR, ISR, masks, EOI, priority, and the
master/slave cascade. No device may mutate CPU interrupt state.

## Contract

`core_machine_pic_irq_source` is a device-owned binding to one core PIC line.
It can assert or deassert a physical IRQ signal and retains only its own
asserted state. The core PIC owns all controller-visible state. Sources never
write IRR/ISR, call CPU interrupt delivery, or manufacture a vector.

The PIC's ICW1 LTIM mode defines line behavior. In edge mode, a low-to-high
source transition latches IRR until acknowledge. Deasserting that source does
not erase an already latched edge. In level mode, an asserted source contributes
to an explicit line count; refresh re-presents that line after EOI while it
remains asserted, and deasserting the final source withdraws it. Source state
is accounted for separately from IRR so masks, acknowledgement, and EOI do not
lose the physical-line fact.

IRQ8--IRQ15 are owned by the slave PIC and reach the CPU only through master
IR2 cascade state. Master/slave IRR, ISR, mask, priority, non-specific/specific
EOI, and vector selection remain PIC behavior. The CPU only polls and
acknowledges the PIC through its existing core execution binding.

## Breakdown

### S1: Contract And Port Probe

Add the core source contract and a port-level PIC smoke. Prove IRQ1, IRQ6, and
IRQ14 assert/deassert behavior; edge latching, level re-presentation, masks,
master/slave cascade, IRR/ISR reads, EOI, and priority selection. No VM device
behavior changes in S1.

### S2: Source Migration

Bind PIT, KBC, FDC, and HDC through explicit source objects. Replace direct
PIC mutation with source assert/deassert at their natural completion, status
acknowledgement, reset, and finalize boundaries. Preserve each device's port
contract and existing host/input path.

### S3: System Regression And Closure

Run the focused PIC ports smoke, IRQ1/KBC, IRQ6/FDD, IRQ14/HDC, FDD/HDD boot,
DOS prompt, Console, and debugger matrix. Source-scan rejects direct device
calls to legacy PIC mutation or CPU interrupt state. Only S3 produces
`build/output/nxvm_0_5_0216.exe`.

### S4: Level Cascade Regression

Extend only the existing PIC port smoke with level-mode IRQ14. Verify slave
IRQ6 and master IRQ2 enter ISR, slave then master EOI completes, refresh
re-presents IRQ14 while its source remains asserted, and deassertion withdraws
the final request. S4 changes no runtime behavior and produces no artifact.

## Rules And Stop Conditions

Applicable rules: core owns shared PIC state; VM devices own only their own
state; composition binds cross-module services while configuration is open;
one CPU interrupt delivery path; no core-to-VM dependency; retained NXVM UX
and boot behavior. Stop for a second IRR/ISR owner, a device-to-CPU shortcut,
loss of asserted level state across EOI, an unmodelled cascade route, or any
FDD/HDD/Console/debugger regression.

## Result

Completed. `core_machine_pic_irq_source` is the sole device-facing IRQ
boundary. PIT IRQ0, KBC IRQ1, FDC IRQ6, and HDC IRQ14 now own source asserted
state only; the core PIC pair owns IRR, ISR, mask, priority, vector selection,
EOI, and cascade. Status/sense/reset/finalize boundaries withdraw their source
where the device has completed or acknowledged its request.

The new port probe covers edge latching after deassert, IRQ1 versus IRQ6
priority, masking, IRQ14 master/slave cascade and EOI, plus LTIM level
re-presentation after EOI. `current-gates-gcc` passes, the current CTest
matrix passes 52/52 tests, and a source scan finds no legacy direct PIC
mutation. Developer artifact: `build/output/nxvm_0_5_0216.exe`, SHA-256
`08466E84D0661BA55B1564563F1A2FE7E03D6927AFBCE55C27B3FEAE0971156B`.

S4 additionally proves level-mode IRQ14 cascade: slave IRQ6 and master IRQ2
enter ISR, slave then master EOI completes, refresh re-presents the still
asserted source, and source deassertion withdraws the final request. It is a
regression-only extension; the T216 artifact remains unchanged. The full
current CTest matrix again passes 52/52.
