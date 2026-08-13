# Product UX

This document defines the high-level interaction of the current NXVM product
and future NXVDM product. Detailed CLI grammar, containment rules, debugger
commands, acceptance cases, and M8 delivery criteria are retained in
[etc/requirements/nxvdm-runtime-detail.md](../etc/requirements/nxvdm-runtime-detail.md).

## NXVM

`nxvm.exe` remains a bootable whole-machine product with its interactive NXVM
Console and debugger. It preserves the direct machine/firmware/media workflow;
it does not acquire a replacement process CLI.

## NXVDM

`nxvdm.exe` is the non-invasive DOS application product. Its primary surface is
`nxvdm run [options] [program [args...]]`, with an explicit debug entry mode.
Normal use launches one guest program; debug use may begin with no program and
pause at a defined machine point.

## Presentation And Debugging

NXVDM provides `auto`, `console`, and `window` display choices. Console mode
is text-only and fails clearly for unsupported guest graphics. Window mode owns
guest display and input. Auto may begin with compatible Console text then make
one supported transition to the guest window.

The guest window owns guest interaction. In debug mode a separate control
Console presents the shared debugger experience; host logs and standard streams
remain distinct from the guest display.

## Host Resources

The user selects visible host drives explicitly or by an exclusion policy. A
guest program can observe and access only that resolved visible set; path,
reparse, UNC, device-namespace, and drive-escape attempts fail as unavailable.
NXVDM does not require installation, system changes, or administrative access.

Exact option behavior, cancellation/exit semantics, and the Windows
compatibility matrix remain supporting detail. The delivery sequence belongs to
[Roadmap](ROADMAP.md), not this interaction authority.
