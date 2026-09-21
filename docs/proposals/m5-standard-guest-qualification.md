# Standard Guest And Windows Qualification

## Goal

Qualify the selected 386DX Standard machine. DeskPro is eligible if selected
with its actual board identity and firmware; 5170 remains a separate AT product.

## Dependencies And Batches

Freeze its real board/firmware/media set and complete the needed CPU/controller
contracts first. Record finite named checkpoints for DOS boot, keyboard,
media access, protected-mode/extender transitions, then Windows 3.1 installation,
Standard Mode and 386 Enhanced Mode lifecycle in that dependency order.
Record any unsupported prerequisite explicitly; do not broaden the hardware
configuration dynamically to pass a later checkpoint.

After the configuration cutover, use NXVM.ini and external readonly
masters/overlays through the production asset route. Preserve the original
integration universe until the product
retirement map explicitly assigns each row; replace old board-specific scenarios
with the corresponding selected-board proof, not an empty or weaker gate.
XT/AT boot and controller regressions remain required; 386-specific Windows
checkpoints are not imposed on XT as though all four machines were interchangeable.

## Exit

Each selected scenario reaches its accepted checkpoint with input and lifecycle
proof. Budgets contain tests but are not success predicates. Required unit and
integration suites pass, artifacts are reproducible, and the final hardware/
timing audit agrees with actual code. Protected media is never committed.
