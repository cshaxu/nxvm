# M5 T540 S16 VADP Memory Routes

VADP now prepares its replacement chip before publishing any memory or port
route. Core owns a single mapping table and publishes the selected CGA or
planar provider together with the EGA write observer as one owner-scoped
operation. Observer-only EGA configurations use the same operation. VADP no
longer borrows `t_ram`, registers raw memory callbacks, or inspects backing
memory directly. Snapshot backing is copied through Core's observational
inspection operation; its operational memory read path is unchanged.

## Failure and receiving proof

- Core records provider and observer counts before publication. If either a
  provider or the observer cannot be appended, both counts revert. A repeated
  owner is rejected before publication. The transaction test covers first
  provider failure, second provider failure after an append, observer failure,
  owner removal and successful retry.
- If the subsequent port batch fails, VADP removes only the candidate memory
  owner and destroys that candidate; the old chip and unrelated memory and
  port owners remain. Existing EGA/VGA/Compaq allocation-position and occupied
  port tests retain this proof and retry successfully.
- Successful configuration swaps the sole chip only after both batches are
  published. Finalization removes its routes by owner. CGA text and graphics,
  EGA planar and Compaq copied snapshot tests remain the receiving tests for
  observational display capture. The EGA planar DOS integration checkpoint
  passes on both widths.
- The `verify-vadp-memory-routes` gate rejects raw `t_ram`, raw registration,
  unregister and physical inspection in `vadp.c`/`vadp.h`; the obsolete raw
  combined registration helper is deleted. D4's replacement/parity memory
  owner remains S17, and ROM/reset aliases remain S18. Neither is silently
  generalized into a second mapping table or a device framework.

Actual tracked production/API changes add 109 and remove 54 lines (net +55):
the typed Core transaction and copied inspection add the new boundary while
the raw VADP helper is removed. Ten NXVM owner-local tests add 209 and remove
184 lines (net +25), primarily replacing their independent `t_ram` fixtures
with the Core machine's one memory owner and extending transaction failure
coverage. The new static gate is 32 lines. No Shared, MyNES, firmware, media
or INI content changes.

## Verification

- Complete repository-only unit suites: x64 **467/467** and x86 **467/467**.
- Focused CGA/EGA/Compaq unit receivers: **12/12** per width. External EGA
  planar DOS checkpoint: **1/1** per width. The four-machine external
  integration matrix remains the T540-level gate, not an S16 claim.
- The complete specialized gate target passes **74/74** build steps, including
  the new memory gate; documentation governance and `git diff --check` pass.
  Four x64 and four x86 0540 Release products were rebuilt. `objdump` confirms
  each PE architecture and no `.debug` sections. An initial x64 unit run under
  concurrent build load had one `library.kvm_window_modal` failure; its isolated
  rerun and the subsequent complete x64 rerun passed. It is not counted as a
  permanent pass without the clean full rerun.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `1C57D572354B73538614B3D03902C7755CAA68217DC23BAFA526B68AC67A8F30` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `8F2B941E707BFDCEF9FCBE267F9ACFCBCDFFAB788623B37412A97CE7C08E9C73` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `C44CD1D851E244E234A5C105BBA4E49F70B81433EBC9D7F7AA10138A9F441084` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `65648205B2769CFE926FCCDF08509E9A8E73ECCDBDD4B2187600E4A839C34DF6` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `00821E6A54B3435608EB1B7CBC65AD87F2CB513AE4AB7EF9D7F970C9E506379C` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `B50546480517BB944A551934003CF10AC451E6090C17C7D62C55A567607B954E` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `FA36CB863EA5A80A0C84CB3C4EC91D2D0EE81E882FC352A0C2A92D3045C1B0DB` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `3C738D49BDF3000EC4AB939866ADDDC461C1DD730B66B9908EC74A92D653D619` |

This structural boundary makes no new video-function or timing-grade claim.
