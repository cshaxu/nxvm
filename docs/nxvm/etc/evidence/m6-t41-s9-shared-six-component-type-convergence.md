# M6 T41 S9 Shared Six-Component Type-System Convergence

## Result

The six Shared component trees now use Lib's fixed-width and runtime vocabulary
where it is available.  The x86 DEBUG `command_runtime.h` facade is deleted;
its duplicate scalar aliases and CRT forwarding macros no longer hide the
actual Debug, Lib Types, or x86 interfaces.  XASM32 prefix aliases are direct
`lib_u8` fields.  Shared tests use the Lib memory, text, allocation, fixed
width and atomic vocabulary rather than locally importing equivalent C forms.

`lib/types/types_interface.h` is the only C-string and allocation adapter.  It
now provides value-normalized comparisons plus direct substring/copy helpers,
so consumers do not retain a second CRT facade.  The global vocabulary gate
now includes `src/lib` and `test/lib`; it has no consumer exemption.

The MyNES build presets were corrected to name the current `mynes-0-0-0041`
target, so both architecture artifact builds exercise the target that they
publish.

## Native-boundary disposition

| Boundary | Reason |
| --- | --- |
| `src/lib/types/types_interface.h` | Single Lib owner of C headers, C string functions, allocation and `size_t` adaptation. |
| `src/lib/types/atomic.h` | Single Lib owner of compiler/standard atomic primitives. |
| `int main(void)` in smoke programs | ISO C executable ABI. |
| `test/x86/xasm32/xasm32_bounds_smoke.c` | Guard-page probe requires the native Win32 virtual-memory ABI. |
| Lib Win32 adapter smoke fakes | Their callback declarations deliberately match the native APIs being tested; the rest of each fixture uses Lib scalar types. |
| `test/common/machine_fixture.*` | A Common executor-concurrency test fixture uses native events/atomics to prove real host scheduling; all ordinary text/memory calls use Lib. It is test-only and introduces no Common production platform dependency. |
| Deliberate negative CMake fixtures | They contain rejected spellings to prove the static gate rejects them. |

## Verification

- `verify_t317_test_type_vocabulary.cmake`: 875 tracked source/test/script
  files pass, including Lib and Lib tests.
- x86 DEBUG boundaries and x86 corpus verification pass.
- Shared x64 Lib/Common/x86 CTest selection: 57/57 pass.
- MyNES Release tests: x64 53/53 and x86 53/53 pass.
- All four NXVM fixed profiles were rebuilt in Release for both x64 and x86;
  each deployment validates its PE architecture.
- `git diff --check` passes.  The standalone full Shared unit aggregate remains
  blocked only by the pre-existing absent `tools/session-readiness-state.tsv`.

## Artifacts

| Product | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| MyNES `0_0_0041` | `8D56BEE4028450BB3FF0C01A49EEC07801345BF4EE13F08F708FF060BD68FFE4` | `B451795A9FC5C659D4050370A55E11CE3F90DA6B98DC82789E71210F4722B89D` |
| NXVM default `0_5_0535` | `254C731FDBBD5CDD1214140E93807A0610045F923086EC8111162A0EBFC3ECD2` | `4E7686D487BEFB5F9037D8C8E4F394CB6039AC8FD5BE188769D0ED5B884A834C` |
| NXVM XT `0_5_0535` | `041CB38237EA5C894AB307289FB4068CE8D94FB5F349766F1E7769E91494E768` | `5C661E522C3FE77AF5B056569B4E96AEFB07CEB6B7EC50ED5039BD996B986F12` |
| NXVM AT `0_5_0535` | `FFDA19F7BD51942BEC88E8049FFBDC8C618B624C83B547CBBF8E58D3E0AA3EBA` | `98F58DF340533A31AA5156574C1B7AB2BE0DD2BB35BF736B8F2094C1A37C9FE1` |
| NXVM Model 40 `0_5_0535` | `487D4E4D0E1DFD71FF6B67FAD6A0DCBC072DA2BD5AD1A76C4969C12183807E61` | `D3BE363274A60CDE5786866D25CBE42C7966A7423D3CEFDAB002602934931CB5` |
