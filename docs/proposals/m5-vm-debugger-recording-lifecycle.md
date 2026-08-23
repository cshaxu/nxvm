# M5 VM Debugger State And Recording Lifecycle Repair

## Purpose

Make VM debugger state instance-owned and recorder lifecycle failures visible,
so one session cannot silently share cursor state or report completed recording
while its file remains open or failed.

## Required Scope

Move mutable debugger cursor state into the existing debug instance/context,
audit every record write/start/stop/finalize path, and guarantee close on
session destruction. Preserve the retained console commands and recording file
format while making I/O/close failure semantics observable at their current
owner boundary.

## Dependencies And Completion

Completion requires two-session isolation, start/write/stop/finalize failure
tests, a recorder-call sweep, and current gates.

## Non-goals And Stop Conditions

Do not redesign the Core debugger context, add a second command interpreter,
change trace format, or add asynchronous recording. Stop if command-status
surface changes require the existing command-authority task.
