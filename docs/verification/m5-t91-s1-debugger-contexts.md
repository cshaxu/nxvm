# M5 T91 S1: Core Debugger Contexts

## Result

The debugger command workspace is `core_product_debug_context`, owned by its
caller. Assembler and disassembler storage is created for each call. Their only
remaining file-static state is a thread-local active-context pointer, which
does not own parser or target data.

## Verification

- `nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\\fdd.img`:
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm-vm-debug-pause-boundary-smoke D:\\fdd.img`:
  `M5:T45:S1:PAUSE-BOUNDARY:OK`.
- Runtime banner: `Neko's x86 Virtual Machine [0.5.0091]`.

`build/output/nxvm_0_5_0091.exe` SHA-256 is
`84FF8D17DDFF917E7E096E880FFCF2BEC1EB132AACE7A07A7502C17301A4D595`.
