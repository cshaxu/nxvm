# T390 S15: Operand-Size RCL-CL Qualification

Intel 80386 PRM RCL table (printed page 372) gives `D3 /2 RCL r/m32,CL` as 9/10 clocks. S15 qualifies only the observed operand-size register form at nine Core ticks; memory and other forms remain nonphysical.

Focused timing-ledger proof covers positive register admission and memory rejection. Full 283/283 current gate passes. A redacted Model-40 replay reaches its explicit 2000000 retirement budget with no unallocated success; it does not reach 7C00 and is not a task-completion claim.

S15 artifact SHA-256: `
9A0996D0C207BEA9848EEE42BC268AF58A1922C6CDABBF0A376AE36B22391540
`.

## Coordinator Acceptance

Actual-diff review of P1 confirms one private Core owner and one exact timing row: operand-size `D3 /2` register RCL with CL count is admitted at nine ticks. The decoder retains the existing prefix boundary and rejects memory, native-size and other group-2 forms through the physical classifier. The focused ledger smoke was independently rerun successfully. The retained 2,000,000-retirement replay removes the transferred unallocated terminal but remains a budget-exhausted, pre-7C00 observation, not a board, firmware, corpus-completion, physical-clock or L3 acceptance.
