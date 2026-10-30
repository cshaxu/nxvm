# T582 S26 — repeated opcode-record copy candidate

`ExecInit` copies available prefetched bytes into the current instruction's opcode record; `_s_read_cs` later copies successfully fetched bytes into that record again. A scratch-only candidate avoided the second copy for a one-byte read when the destination byte already equaled the source; all other cases retained the original copy. It did not add persistent state, change fetches, or intentionally change instruction/event timing. This is distinct from S11's single-byte copy specialization, which still performed both copies. Complete proof would still need aliasing, self-modifying code, prefetch invalidation, exceptions, nested entry, Debug and all-profile cases.

One baseline-then-candidate fixed PC110 startup pair per width reached 10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`. x64 times were 19,965 / 19,942 ms and x86 were 23,737 / 22,347 ms. The x64 result is effectively unchanged; the x86 difference is an ordered single pair subject to host variation, and the checkpoint does not prove complete state equivalence. There is no evidence here of a material dual-width improvement or of Game performance, so no production branch or full receiving test was added.

Scratch log SHA-256: `6C97949C0C5E05113366F598CC88BEC1FCEAECCA62361361C20920A9DBEF1BFE`.
