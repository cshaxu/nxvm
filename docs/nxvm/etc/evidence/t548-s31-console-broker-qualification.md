# T548 S31: Console Broker Qualification Repair

## Scope and owner

The x64 complete repository-only unit run exposed one native Console contract
failure in `library.console_broker_display`.  The owner approved this narrow
Shared Lib correction in the active S31 packet.  It changes no public API,
ABI, thread, state machine, retry policy, firmware, media, INI or snapshot.
The repaired Shared source revision is `793ab2456`.

## Cause and retained owner

`SetConsoleScreenBufferInfoEx()` can report success while a native Console host
retains a smaller backing buffer.  The Console broker had restored the visible
viewport without first restoring and checking the saved backing capacity.  The
private Console broker remains the sole owner: after metadata succeeds it reads
the actual buffer, restores the saved capacity when necessary, verifies it,
then fits the saved viewport to actual host limits.  A required capacity or
viewport failure returns the existing `LIB_STATUS_IO_ERROR`; unsupported
palette behavior remains unchanged.

## Regression coverage

The existing Console contract fixture now proves:

- metadata may silently reduce a saved `120x60` buffer to `120x29`; the broker
  restores the saved capacity and then restores a `120x30` viewport;
- a refused backing-buffer restore returns `LIB_STATUS_IO_ERROR` and leaves the
  existing temporary geometry visible rather than updating completion state;
- a host-visible limit clips the viewport, not the saved backing capacity; and
- palette metadata changing geometry retains the prior text-output behavior.

The same failure class was searched only in the Console display restoration
owner; no second native display restoration path exists in tracked production
source.  The change does not claim manual desktop or external-media integration
coverage.

## Verification

| Check | x64 | x86 |
| --- | --- | --- |
| Lib complete unit package | 36/36 | 36/36 |
| Complete repository-only unit suite | 515/515 (228.82 s) | 515/515 (50.11 s) |
| Current dependent Release artifacts | five x64 PE files | five x86 PE files |

All ten deployed artifacts passed the existing PE architecture verifier.  The
current SHA-256 identities are:

| Artifact | SHA-256 |
| --- | --- |
| `assets/my5160/nxvm_xt_0_5_0546_x64.exe` | `A8350E0165D7D863D0E9AC92079F48A05B089A1465CB15F87A74D0D400471F0F` |
| `assets/my5160/nxvm_xt_0_5_0546_x86.exe` | `F1FCA11E5D946A0A3D3DB829B0F8BDB29067CAEE7402FE59081E33C77864162C` |
| `assets/my5170/nxvm_at_0_5_0546_x64.exe` | `316A2420306789812BE280BDAFC17B953E122A8533C8A4F7644F5A1F763ED6A2` |
| `assets/my5170/nxvm_at_0_5_0546_x86.exe` | `993E68FF79990E959F14AC3D371F7583EDC398146A0666F2878DCDF3EBCB444F` |
| `assets/mydeskpro386/nxvm_model40_0_5_0546_x64.exe` | `9B8A3C52E55EC4581C6176C8518EE0FC3EE15145A0729B47769EB1FC4D492F81` |
| `assets/mydeskpro386/nxvm_model40_0_5_0546_x86.exe` | `D154AF783E048F10AF707E6BFF66F99CCEE7E125DBCBF65DDDB2F578F1414BE7` |
| `assets/nxvm/nxvm_default_0_5_0546_x64.exe` | `B2D3E7823EB9AEC19657764E9C126772EFF21EC6FDCA0A1890B91607EF7CDF13` |
| `assets/nxvm/nxvm_default_0_5_0546_x86.exe` | `7AAFFAA880967DA4F532672D97B12874C145FD8CC5D3C69AA2C04FE5EE388DF8` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `035A1785650B2BA911C69513D5BBAC0DC6530CC26537636274014ADBA0F1BBB9` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `EA57D0D0F577F98CB73FFCF73E02F141ED31005DDA9A38C8F8C7ED760F8EF613` |
