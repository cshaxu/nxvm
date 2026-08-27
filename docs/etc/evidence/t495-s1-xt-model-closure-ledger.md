# T495 S1 IBM 5160-268 Model Closure Ledger

`M5:T495:S1:XT-MODEL-CLOSURE-LEDGER:OK`

This is the durable, finite convergence ledger for the T495 final-model
audit. It consumes accepted evidence; it neither reopens a component ledger
nor authorizes a repair. A row is final only when T495's later decision audit
records either the listed proof or an explicit transfer to the named receiver.

| ID | Selected model relation and sole route | Accepted evidence | Timing/remaining boundary | T495 decision evidence |
| --- | --- | --- | --- | --- |
| M1 | Immutable `ibm-5160-model-268` request -> resolver -> copied Core plan | T483 S1--S3; T484 S3/S5; XT profile smoke | No generic 8088/AT alias or mutable session option exists. | Resolve/fixed-selection and negative-option proof. |
| M2 | 8088 execution, four-byte queue and one Core CPU/transaction owner | T486 S5; T484 S3 | Group-3 ranges and WAIT service remain explicit CPU L2 boundaries; no profile timing path may estimate them. | 8088 focused ledger and profile construction proof. |
| M3 | 256 KiB RAM plus owner-supplied immutable XT ROM mappings -> Core reset entry | T484 S5/S21; XT BYOB session smoke | ROM bytes and provenance remain external; no selected physical CPU/board timebase. | Valid/invalid manifest, reset-vector and failure-atomicity proof. |
| M4 | 8-bit board construction, primary ports and absent RTC/CMOS, slave PIC and second DMA | T487 S5; T484 S5 | ISA/refresh/contention/RESET edge phase has no qualified common physical axis. | Topology/port-absence and cold-reset proof. |
| M5 | Single 8259A at `20h`/`21h`, IRQ logical delivery through Core | T488 closure; T484 S5 | Logical PIC order is Manual L3; elapsed-axis delivery placement remains L2. | PIC/XT topology and interrupt-order regressions. |
| M6 | One 8237A, XT primary page ports and 8253 PIT/refresh route | T489 S5; T490 S6; T487 S5 | DMA physical conversion and board phase are L2; 8253 logical personality/IRQ0 are complete. | DMA/PIT topology, reset and no-secondary-port proof. |
| M7 | XT 8255 keyboard/Port-B parity and NMI request -> Core PPI/board owners | T491 S5; T484 S8/S19 | External keyboard-device timing is L1; numeric keyboard clock absent; electrical behavior is excluded. | Make/break, IRQ1, PPI reset and parity-NMI proof. |
| M8 | 8272A FDC, fixed 360 KiB logical media, DMA2/IRQ6 -> Core FDC/media owner | T492 S5; T484 S9/S10/S21 | XT supplies zero conversion input, therefore FDC delay is explicit L2; HLT/HUT/rotation are external. | FDC ports/no-`3F7h`, reset/ready, DMA/media and boot-consumer proof. |
| M9 | CGA ports and `B8000h` -> sole VADP state -> copied display snapshot | T493 S5; T484 S11 | Logical mode/VRAM behavior is complete; raster/board phase and external pen producer are bounded. | CGA port/aperture/snapshot and AT-isolation proof. |
| M10 | Xebec `320h`--`323h`, Type-2 logical media, DMA3/IRQ5 -> sole Core HDC/media owner | T494 S5; T484 S15--S18/S21 | Controller ROM execution, signal values and drive service are source-excluded physical boundaries; no ATA alias. | Positive-count read/write, terminal/error, Xebec-port and session-media proof. |
| M11 | Cold reset, firmware mapping failure and session construction rollback retain one Core/VM owner chain | T484 S21; T487 S3; T489--T492 reset evidence | No invented reset duration; external reset-edge phase remains source-bounded. | Reset/retry, invalid manifest and controller-reset regression. |
| M12 | Core event order and guest-progress observation; VM only consumes copied observation/presentation | `current-capability-baseline`; T469--T472 retained contracts | Deterministic order is accepted; IBM 5160 has no qualified host-pacing physical axis, so Standard must not claim wall-clock equality. | Cross-device event-order, HLT/cancel and no-host-tick-injection proof. |

## Completion predicate

T495 may declare **model-functionally ready with bounded L3/L2 timing** only
if M1--M12 each have current, concrete owner-path proof and none produces an
AT alias, second state owner, unqualified wall-clock claim or hidden device
defect. It may not declare a physical/cycle-exact or host-wall-clock-ready
result. A missing proof transfers its exact row to the earliest component or
XT board-time receiver; T495 itself remains audit-only.

The final decision S must also retain the three non-XT fixed-disk regressions
as isolation checks: default-at ATA PIO, IBM 5170 Model 339 WD1003/ST-506 and
DeskPro Model 40 Compaq/WD. They are not evidence for Xebec behavior.
