# M5 Optional Ccache Build Acceleration

## Purpose

Provide one opt-in MinGW/Ninja CMake route that uses the locally installed
`ccache` compiler launcher, while preserving the existing default build route
for every environment without ccache.

## Boundary

Only `CMakePresets.json` and contributor build documentation may change. No C
source, CMakeLists behavior, artifact identity, default preset, global cache
directory or remote cache is introduced.

## Approach And Acceptance

Add one named opt-in configure preset with its own build directory and one
matching current-artifact build preset. Prove a cold build produces misses and
a clean rebuild of the same tree produces ccache hits, then prove the existing
`current-gcc` preset remains healthy. The task does not claim speedup without
measured cache-hit evidence.

## Stop Condition

Stop if ccache cannot cache the current GCC compilation correctly, changes
artifact identity, or requires making ccache a default dependency.
