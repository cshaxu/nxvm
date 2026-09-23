# T452 S1: Optional Ccache Build Acceleration

`ccache 4.13.6` is present on the development host but the default
`mingw-gcc-x64` preset has no compiler launcher. T452 adds the separate
`mingw-gcc-x64-ccache` configure preset and `current-gcc-ccache` build preset.
The default presets and their build directory remain unchanged.

With `CCACHE_DIR=build/t452-ccache-store`, the isolated ccache build performed
296 cacheable compiler calls: 203 hits (200 direct and 3 preprocessed) and 93
misses. This is evidence of repeat-build reuse, not a claim about a particular
wall-clock speedup on every host.

The ccache executable, its copied output artifact and the default current
artifact all have SHA-256
`2FB60FBF8C711B2044C5338B3F59781BA5F32EA56BA885598662115FE885CCFE`.
The default `current-gcc` build then passed with no work required, and
documentation governance plus diff hygiene passed. The task-local experiment
tree and cache are removed at closure; ordinary opt-in users receive their own
isolated `build/mingw-gcc-x64-ccache` tree and normal ccache location.
