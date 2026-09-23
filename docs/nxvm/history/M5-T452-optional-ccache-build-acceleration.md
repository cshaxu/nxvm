# M5 T452: Optional Ccache Build Acceleration

T452 adds one verified opt-in compiler-cache route for repeat MinGW builds. It
does not change the default developer build route or any production source.

## Accepted Subtask

S1 adds the isolated opt-in presets, proves cache reuse and byte-identical
artifact output, then confirms the default route is unchanged. The retained
[evidence](../etc/evidence/t452-s1-optional-ccache-build-acceleration.md)
records the complete disposition.

## Closure

The default `mingw-gcc-x64/current-gcc` route remains unchanged. The retained
proposal companion documents the optional scope; no production source, CMake
target or build gate changed.
