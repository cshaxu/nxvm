# T390 S15: Operand-Size RCL-CL Qualification

Intel 80386 PRM RCL table (printed page 372) gives `D3 /2 RCL r/m32,CL` as 9/10 clocks. S15 qualifies only the observed operand-size register form at nine Core ticks; memory and other forms remain nonphysical.

Focused timing-ledger proof covers positive register admission and memory rejection. Full 283/283 current gate passes. A redacted Model-40 replay reaches its explicit 2000000 retirement budget with no unallocated success; it does not reach 7C00 and is not a task-completion claim.

S15 artifact SHA-256: `
9A0996D0C207BEA9848EEE42BC268AF58A1922C6CDABBF0A376AE36B22391540
`.
