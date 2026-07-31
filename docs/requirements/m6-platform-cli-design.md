# M6 Platform And CLI Design Requirements

M6 is a design milestone for M7. It converts the future `run` requirements into
an implementable and security-testable Windows Platform contract.

It must define normal no-program usage failure, program-path to guest-drive
mapping, exact guest/host/cancellation exit-status values, and every option
validation error. It must define a handle-based filesystem-containment algorithm
for visible drive roots, DOS path normalization, dot segments, reparse points,
UNC/device denial, race handling, and Windows 7 through Windows 11 tests.

It must also define the Console/window lifecycle and input ownership state
machine, implemented display-mode table, debug command grammar, no-program
debug `continue` cleanup, `Ctrl+C` and `Ctrl+Break` routing, redirected-handle
behavior, and resource restoration on every exit path.

M6 closes on an approved design, threat model, and automated/manual acceptance
matrix. M7 does not invent product semantics omitted here.
