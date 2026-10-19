# MyNES Tools

These tools are owned by the MyNES product. They may use the root shared
Lib/Emulator corpus, but must not introduce a second copy of its behavior.
Install CMake >= 3.23, Ninja and the appropriate MinGW compiler. No local
toolchain paths or game ROM bytes are stored in this repository.

Before running each architecture, prepend its compiler directory to the process
PATH so compiler helper executables resolve matching DLLs. For example, in a
fresh PowerShell for x86: $env:PATH = (Split-Path $env:MYNES_CC_X86) + ';' + $env:PATH.
Do not persist this into the system PATH. The presets verify pointer width.

Run from the repository root:

```powershell
cmake --preset mynes-gcc-x64-release
cmake --build --preset mynes-current-x64
cmake --build --preset mynes-tests-x64
```

For x86, the preset selects `i686-w64-mingw32-gcc`; set `MYNES_CC_X86` only to
override the compiler used by the standalone shared-component verifier, then
use the `mynes-gcc-x86-release` and `mynes-current-x86` presets. CTest is serial
because native Console probes must not compete. Assertions stay enabled in
Release; tests time out rather than hanging a run. Native GUI/Console tests
need an interactive Windows host. A headless failure is not a passing test.

Independent Lib/Emulator source/test entries use the same build policy:

```powershell
pwsh -File tools/mynes/Verify-SharedStandalone.ps1 -Architecture x64 -UseCcache
pwsh -File tools/mynes/Verify-SharedStandalone.ps1 -Architecture x86 -UseCcache
```

The tool selects the matching compiler, uses stable standalone trees under
build/.ninja, injects the repository's real CMake configuration contract and
executes the owned suites serially. Native integration still requires an
interactive host; a headless failure is not a pass. Logs/results go to the
temporary build/standalone-verification-<architecture> directory, not a cache.
The caller records necessary results and cleans it at phase end. It restores
its process PATH. Root paths are inputs, not another component implementation.

Plain/cache-enabled presets share each configuration's build directory.
Cache, task-output budgets and cleanup follow the single shared
[Build Tree Hygiene](../../docs/rules/EXECUTION.md#build-tree-hygiene) authority.

Performance probes are explicit diagnostics, not timing assertions in CTest.
Build `mynes-performance-probe` in an optimized MyNES-only cache, then run
`Measure-Performance.ps1 -BuildDirectory build/.ninja/mynes-gcc-x64-release
-AssetDirectory ../nxvm-assets/roms-mynes -OutputDirectory build/performance`.
Use the matching x86 cache for that width. `-ProcessorIndex` selects one allowed
logical CPU (default zero); the probe inherits that affinity before execution
and the tool restores its own original affinity. `-ProbeExecutable` can select
a pre-change reference retained beneath ignored `build/`. Both variants must
use the same input, compiler, flags, processor and workload. Report conversion
separately from Core cost; these unpaced measures do not describe displayed FPS.
`-Workloads` and `-Modes` select a bounded diagnostic subset without changing
the full default matrix. Pair reference/candidate runs per workload. The CSV's
`probe_cpu_ms` is one complete process CPU total, repeated on its batch rows;
it includes warmup and checksum work, so never sum it five times or call it
per-stage time. It helps distinguish descheduling from computational cost.
