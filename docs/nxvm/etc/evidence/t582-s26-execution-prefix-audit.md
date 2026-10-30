# T582 S26 — execution prefix classification candidate

`ExecIns` classifies the current opcode as a prefix before dispatch and checks it again when deciding whether to continue. The classifier is a pure function of opcode and CPU profile. A scratch-only candidate computed the result once and reused the local result, including the V30 post-dispatch check; no production source or public interface changed. Before adoption it would still be necessary to establish that execution callbacks cannot change the profile during dispatch, and to verify exception and prefix behavior for every supported profile.

In a single baseline-then-candidate fixed PC110 startup run per architecture, both variants retired 10,000,000 instructions and reached 79,964,294 guest ticks and PC `0003dbc5`. Elapsed time was 20,203 / 21,146 ms for x64 and 23,660 / 25,089 ms for x86. Both candidates were slower in this ordered comparison; host variation and code layout prevent treating it as a proven general regression, and matching the checkpoint is not complete state equivalence. The result provides no reason to take this candidate into production or repeat the full test suite. It does not establish PC110 usability.

Scratch log SHA-256: `AA2D10AE75BFE159D6B20F6BC080149A853FFCB21FFABDAAAEAFFCE028ED71A3`.
