# M5 T1 S2 Bochx Manifest Gate Verification

## Boundary

The optional research utility is project-owned C only. It neither includes nor
links Bochs, applies the historical Bochx patch, opens a guest image, or enters
the default CMake graph. A local ignored manifest identified the existing
owner-local Bochs 2.6 archive by SHA-256
`e7d434f2b7f11dc9d6e0eab1080a8ae7ebb79864ca87bf2d514909fa1a99d50a` and
the historical Bochx patch by SHA-256
`7dd8ac21828e180c145c6302681d26bc3559fd540b381584575b1b18fb9546f8`.

## Commands And Result

```text
cmake -S . -B build/mingw-gcc-x64 -DNTVDM64_ENABLE_BOCHX_RESEARCH=OFF
cmake --build build/mingw-gcc-x64 --target help
cmake -S . -B build/bochx-research -G Ninja -DCMAKE_C_COMPILER=gcc \
  -DNTVDM64_ENABLE_BOCHX_RESEARCH=ON
cmake --build build/bochx-research --target ntvdm64-bochx-manifest-check
build/bochx-research/ntvdm64-bochx-manifest-check.exe <local-manifest>
```

The default target list did not contain the research utility. The separate
research build completed and emitted `M5:T1:S2:BOCHX-MANIFEST:OK` for a local
real-mode, one-instruction experiment envelope. The validator required the
Bochs 2.6 identity, patch identity, imported NXVM commit, probe id, comparison
mask, trace-ring limit, and instruction/wall-clock/no-progress budgets.

No paired-step experiment or divergence report was run in S2, so no CPU
capability verdict changed. Such an experiment must use the local manifest gate
and create its own bounded result/cleanup record.
