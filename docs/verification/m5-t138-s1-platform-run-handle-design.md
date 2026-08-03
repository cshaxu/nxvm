# M5 T138 S1: Platform Run-Handle Design

The approved contract and current-owner map are recorded in
[`m5-platform-run-handle-contract.md`](../planning/m5-platform-run-handle-contract.md).
The review found the three expected unsafe legacy patterns: window-display
worker-owned cleanup, backend-owned Console cleanup, and detached Linux
workers. T139 and T140 must replace them without changing retained NXVM
Console or window startup behavior.
