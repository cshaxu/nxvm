# T405 S1: x86 GCC Host-Build Compatibility

`M5:T405:S1:X86-GCC-PROVENANCE:OK`

`M5:T405:S1:X86-GCC-COMPATIBILITY:PASS:OK`

## Result

The current NXVM product builds and runs as a 32-bit Windows host process. This
is host portability evidence only; it changes neither guest CPU semantics nor
any DeskPro physical-timing or L3 result.

## Toolchain And Build Provenance

- GCC: MSYS2 `gcc.exe (Rev3) 16.2.0`, target `i686-w64-mingw32`, POSIX thread
  model.
- Generator: Ninja 1.13.2 through CMake 4.3.3.
- Isolated configuration: `cmake -S . -B build/t405-x86-gcc -G Ninja
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/mingw32/bin/gcc.exe`.
- The emitted `build/t405-x86-gcc/vm-0-5-0405.exe` reports `file format
  pei-i386` and `architecture: i386` through the matching x86 `objdump`.

## Repair And Sweep

The old CMake guard rejected every non-64-bit host before compilation. The
shared `type_native_unsigned` carrier was also hard-coded to 64 bits although
`type_virtual_address` stores host pointers. T405 derives that native width
from the standard 32/64-bit host ABI macros, adds a compile-time pointer-width
assertion, and retains explicit CMake rejection of non-32/non-64 pointer ABIs.

The required repository sweep covered pointer carriers, `size_t`, allocation,
file offsets, capacity conversion and formatting. The only x86 compiler hits
after the carrier repair were HDD/FDD provider additions of validated 64-bit
media offsets to host addresses. Each now validates the 64-bit range first and
narrows only at the `size_t` address boundary. Product RAM remains capped at
64 MiB, while file positions retain the existing 64-bit facade.

## Verification

- x86 build ran `vm-0-5-0405`, `run-current-smokes`,
  `verify-current-specialized-gates`, then
  `ctest --test-dir build/t405-x86-gcc --output-on-failure -L current-gate`:
  286/286 passed.
- The current gate includes the IBM 5170 Model 339 profile composition/reset
  smoke and the private DeskPro Model 40 composition/integration reset and
  controller path smokes; no ROM or guest media was supplied.
- Existing x64 GCC was reconfigured and replayed with the same
  `vm-0-5-0405`/specialized/current-gate commands: 286/286 passed.
- Preserved x64 developer artifact:
  `build/output/nxvm_0_5_0405.exe`, SHA-256
  `6A2234EBBD08910686B6A9EB708DBCE3269DFA0379CB9C89ABF3CB9230FC8E48`.

No external toolchain content, ROM, firmware, guest media, or third-party code
was imported or committed.