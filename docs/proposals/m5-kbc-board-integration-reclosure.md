# M5 8042 Keyboard-Controller And Board-Integration Reclosure

Re-audit 8042 host commands, command byte, buffers/status, A20/reset outputs,
keyboard-line inhibit, keyboard protocol, BAT, asynchronous replies and IRQ1.
Prove the complete sole-owner path from 60h/64h firmware sequences through
keyboard state/deadline, the single KBC output buffer and PIC IRQ1 to actual
5160/5170/AT/Model-40 ROM observation.  This is not a 5170 `301` workaround:
no BIOS/profile injection or command-specific parallel FIFO is allowed.  Apply
the shared [controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; unified state-machine/line-path
repair; cross-profile ROM and complete-gate closure.
