# T418 S1 Instruction-Boundary Locality

M5:T418:S1:INSTRUCTION-BOUNDARY-LOCALITY:OK

Original D3PE source says a CPU-inserted idle state terminates PAGE HIT and
returns to INITIAL. Core cannot observe that physical overlap phase. T418
therefore adds a conservative generic-AT rule at the existing execution-round
owner: each new instruction clears the previous CPU locality key. This avoids
claiming that adjacent logical accesses from later instructions are a D4 page
hit. It is not an assertion that every original D4 instruction boundary was an
idle state.

Focused regression proves the next instruction's write does not inherit the
first prefetch page key (+6 instead of the prior +5 aggregate), while direct
same-round locality, cancellation, DMA HOLD, refresh pulse and reset coverage
remain. No interface, scheduler, transaction path or VM dependency changes.

Artifact: `vm-0-5-0418`, `build/output/nxvm_0_5_0418.exe`, 3,212,215 bytes,
SHA-256 `E431A7412EB1DF2215CADDBC3A03A7D2179851CD3511F74C7C44F89F08B108A9`.