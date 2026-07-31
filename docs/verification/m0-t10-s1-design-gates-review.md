# M0 T10 S1 Design Gates Review

## Result

The roadmap now requires M2 Machine/DOS architecture design before M3 refactor,
M4 DOS design before M5 implementation, and M6 Platform/CLI design before M7
implementation. M1 records the current NXVM head at start and uses only the
recorded local fixture identities. Runtime CLI behavior now rejects normal
no-program invocation, has no debugger load command, and assigns guest Ctrl+C
to the active guest display surface.

## Status

This is governance only. M1 remains the next eligible and inactive subtask.

M0 T12 S3 subsequently supersedes the earlier no-debugger-`load` decision with
the constrained empty-paused-debug-session `load` state machine. This record
otherwise remains a historical review of the M0 T10 decision.
