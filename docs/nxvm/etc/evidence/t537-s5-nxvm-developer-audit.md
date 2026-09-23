# M5 T537 S5: NXVM Developer Audit

## Scope

This is an `NXVM`-scoped task under the three-scope rule.  It reads sibling and
shared paths only to classify dependencies; its changed files are NXVM
documents, NXVM artifact documentation, NXVM tool documentation, and the two
selected NXVM release artifacts.  It changes no MyNES or shared component,
configuration, governance, test, artifact, or asset file.

## Findings And Disposition

| Finding | Disposition |
| --- | --- |
| The current Architecture authority still named retired `core/*` roots and described versioned profile EXEs as ignored. | Repaired in the NXVM authority: the live roots are `app-nxvm/{devices,machine,profiles,product}` and the adjacent versioned EXE/INI pair is the sole product artifact. |
| The Coding authority named nonexistent current profile/test paths and also called product artifacts ignored. | Repaired to match the actual NXVM-owned tree; historical records remain unchanged. |
| `tools/nxvm/` had no product-local entry point. | Added a concise NXVM-only BYOB build/test guide, including the direct product-only CMake invocation and emitted-artifact rule. |
| The NXVM artifact guide mentioned the peer MyNES artifact root. | Removed the unnecessary sibling reference. |
| Root CMake presets still default `REPOSITORY_BUILD_MYNES` to enabled, so a normal root NXVM preset configures both product graphs. | **Transferred to S6 Shared.** The root preset/configuration is Shared-owned; this NXVM task does not alter it. The NXVM guide provides a product-only command line using `-DREPOSITORY_BUILD_MYNES=OFF`. |

## Verification

Both fresh Release builds passed CMake's selected-profile manifest, size and
SHA-256 validation against the owner-provided external root
`O:/repos.hobby/nxvm-assets/profiles-nxvm`, with MyNES disabled in the generated
build graph.

| Route | Result |
| --- | --- |
| Default PC/AT x64 repository-only unit | 336/336 passed |
| Default PC/AT x64 external integration | 20/20 passed |
| Default PC/AT x86 repository-only unit | 336/336 passed |
| Default PC/AT x86 external integration | 20/20 passed |
| x64 artifact | `pei-x86-64`, SHA-256 `E781B34D92511ED477C0D686BB71A4F40C0783F62AECD130BBB73D9A1ABA152D` |
| x86 artifact | `pei-i386`, SHA-256 `D5B533CE2AEA967D07FFA4EC673D73964368A761043AB6E1039C25FDBE055DB5` |
| Documentation governance | Passed for NXVM and MyNES |
| Actual-diff review | NXVM-owned files only; the Shared finding above remains unmodified |

The pre-existing S3 matrix remains the current evidence for the untouched XT,
5170 and Model 40 x64/x86 product routes.  S5 changed no emulator, CMake or
asset-routing code that could invalidate it.
