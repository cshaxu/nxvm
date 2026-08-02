# M5 T44 S1 Full-PC Authority Closure

The live full-PC smoke starts the retained execution loop, resets it, and
checks every CPU, memory, port, device, BIOS, QDX, and debug accessor against
the one `vm_composition_live_machine` object before finalization. The static
authority gate rejects old full-PC storage definitions in `src/`.

Windows GCC, `M5:T44:S1:FULL-AUTHORITY-CLOSURE:OK`, retained debug-target,
retained Console, dependency-DAG, and live-authority gates passed.

Artifact: `build/output/nxvm-m5_t44.exe`.
SHA-256: `E26F6F27E0AF7F14CA8D8F4C1B3866B23A7B3F4334C894043116F6E778B16B40`.
