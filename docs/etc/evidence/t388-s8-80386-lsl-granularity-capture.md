# T388 S8: 80386 LSL Granularity Capture

`M5:T388:S8:80386-LSL-GRANULARITY-CAPTURE:OK`

Intel 80386 PRM section 17.2.2.3 gives protected successful `0F 03 /r LSL`
byte-granular register/memory rows `20/21` and page-granular rows `25/26`.
The private decoder state clears `source_lsl_granularity_valid` and
`source_lsl_page_granular` in `ExecInit`. `LSL_R32_RM32` sets them only after
successful descriptor validation and ZF=1 on the 80386 profile. The sole
post-refresh publisher consumes that capture; unavailable paths retain the
unallocated/nonphysical boundary.

The owned LAR/LSL smoke proves register byte/page `20/25` and memory byte/page
`21/26`, and keeps invalid-selector, prefix, fault and non-80386 coverage.
This is not a descriptor-semantic, bus/device, physical-clock or L3 claim.

Primary: [Intel 80386 PRM (1986), section 17.2.2.3](https://bitsavers.org/components/intel/80386/230985-001_80386_Programmers_Reference_Manual_1986.pdf).

`verify-t388-80386-lsl-granularity-capture` binds the reset, success capture,
exact rows and focused smoke. It is a recurrence detector only.