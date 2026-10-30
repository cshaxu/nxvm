# T582 S26 — unconfigured auxiliary PIT clock

The PC110 board plan does not enable an auxiliary PIT, but board advancement still updates its auxiliary clock before checking whether the device is configured. The configuration flag is assigned when the board is assembled. A scratch-only candidate moved that clock advance into the existing configured-device branch; configured PIT advancement kept the same ordering. This would change the internal unused clock's delivered-tick count and was not proved equivalent for every diagnostic or future configuration path.

In one baseline-then-candidate fixed-work run per width, every run retired 10,000,000 instructions at 79,964,294 guest ticks and PC `0003dbc5`. x64 times were 19,935 / 20,651 ms, and x86 times were 23,386 / 27,306 ms. These ordered observations are subject to host variation and do not prove a general regression or complete guest-state equivalence. Neither indicates a gain, so no source change or expanded tests were warranted. This does not establish interactive PC110 performance.

Scratch log SHA-256: `A6B702889CD6F15D1CBF27F2F0C41E9239DFA18075917BBBF730C11E6308000B`.
