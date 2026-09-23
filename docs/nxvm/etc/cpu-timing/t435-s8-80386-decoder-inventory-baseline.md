# T435 S8 - 80386DX Decoder Inventory Baseline

This is the first reproducible input to the S8 audit, not its closure.
`core-machine-80386-decoder-inventory-runner` scans all primary
opcode/next-byte pairs under `CORE_MACHINE_CPU_PROFILE_80386`, then scans the
complete `0F` second-opcode/ModR/M space. Its committed JSON inventory records:

```text
M5:T435:S8:I386-DECODER-LEXEME:63021:253
```

The inventory contains 63,021 accepted primary candidates, 253 accepted
primary opcodes and 66 escaped `0F` opcode families. It deliberately records
lexical acceptance only: a faulting, privilege-failing or otherwise
non-successful execution remains outside the eventual timing denominator.

The first manual cross-check already establishes two mandatory S8 repairs:

1. Chapter 17 printed p. 17-183 gives `D7 XLATB` an exact five-clock row, but
   the existing S1/S2 register does not contain it.
2. The existing system template creates a generic `R/M/PM/VM86` cross-product.
   Chapter 17's system encodings and privilege conditions must instead be
   partitioned per instruction; no syntactically generated combination may be
   retained without a legal decoder and successful-manual disposition.

The later S8 verifier must consume this inventory in both directions and make
those two repairs, plus every further difference, explicit before S8 closes.
