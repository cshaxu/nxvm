# Optimized Artifact Without Compiler Debug Information

## Purpose

Correct the current artifact configuration: its optimized executable must carry
no compiler debug information while retaining NXVM's runtime debugger as a
normal product feature.

## Boundary

Use one Release publication route for the existing artifact identity.  Do not
remove debugger commands, trace, pause, step, break/watch, recording, fault
diagnostics, or their tests.  Debug build trees remain test-only observability
routes and must not publish the artifact.

## Completion

The artifact guard accepts only Release; the release cache has no debug-info
flag; PE section inspection finds no debug section; debugger-focused smoke
tests and the Debug current gate pass; and the same 0457 artifact hash is
updated.
