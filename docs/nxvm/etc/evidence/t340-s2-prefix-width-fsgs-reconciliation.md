# T340 S2: Prefix, Width, And FS/GS Reconciliation

## Reviewed Mechanisms

| Mechanism | Source route and static audit | Actual owner proof | Disposition |
| --- | --- | --- | --- |
| Prefix scan and last-prefix state | `ExecIns` repeats only while `_kdf_check_prefix` accepts the preceding byte. `PREFIX_{ES,CS,SS,DS,FS,GS}` update the same override slots; `PREFIX_{OprSize,AddrSize}` set the same two booleans; `PREFIX_REPNZ`/`PREFIX_REPZ` replace one repeat selector. `64`--`67` are gated at 80386 before the loop continues. | `current.core-machine-prefix-attributes-s64-smoke` proves all segment selectors, last segment and repeat prefix, independent and combined attributes, profile rejection, legal/illegal `LOCK`, and IRQ ordering. | Accepted. No parallel prefix decoder or state publisher exists. |
| Operand-size selection | `_GetOperandSize` is the single CS-default-size XOR operand-prefix expression. The 471 source uses were sampled at primary arithmetic, stack, string, primary data, and `0F` integer/selector routes; all use the one expression rather than private width state. | Prefix, GPR MOV, moffs, segment MOV, FS/GS stack, and LxS smokes cover 16/32 destination width, exact EIP, partial register publication, memory source/destination, and faults. | Accepted for S2 form ownership; state-controlled frame/table consumers transfer to T341. |
| Address-size selection | `_GetAddressSize` has 29 uses, all in the shared ModRM/moffs/string/loop/I/O address or index construction routes. `_kdf_modrm` and `_d_moffs` select the 16-bit or 32-bit effective-address layout before memory publication. | Prefix smoke distinguishes `0x0100` from `0x00010100` for independent `67` read/write, and retains independent `66` plus combined proof. Operand-address, moffs, GPR MOV, and string owner smokes cover SIB, BP/EBP/ESP SS defaults, index/count and fault/restart boundaries. | Accepted. There is no separate 32-bit memory publication path. |
| FS/GS selection and form consumers | `PREFIX_FS` and `PREFIX_GS` publish the same override fields used by ordinary memory routes. `_s_load_fs` and `_s_load_gs` both delegate to `_s_load_sreg`; `INS_0F` owns FS/GS push/pop and LFS/LGS/LSS dispatch. | `current.core-machine-fs-gs-stack-smoke`, `current.core-machine-lss-lfs-lgs-smoke`, `current.core-machine-sreg-mov-smoke`, and prefix smoke prove form gates, width, selector/cache publication, source faults, SS-only inhibition, and ordinary FS/GS memory selection. | Accepted for form/width selection. Descriptor privilege/cache and VM86 semantics transfer to T341. |
| `LOCK` classification | `PREFIX_LOCK` owns the primary/secondary whitelist and publication rejection; pre-386 legality remains the retained T328 owner. No handler-local replacement policy was found in the S2 mechanism search. | Prefix smoke proves allowed memory ADD and rejected MOV with no CPU/cache/memory publication; T328 retains 8086/80186/80286 profile matrix. | Accepted; no global policy change. |

## Similar-Issue Sweep

The completed sweep used these exact queries against tracked source and build
registrations:

```powershell
rg -n "PREFIX_(FS|GS|OprSize|AddrSize)|_GetOperandSize|_GetAddressSize|_kdf_check_prefix|PREFIX_LOCK" src/core/machine/cpu_instructions.c
rg -n "core-machine-(prefix-attributes|fs-gs-stack|lss-lfs-lgs|sreg-mov|gpr-mov|moffs|operand-address).*smoke" CMakeLists.txt
```

It found one production prefix loop, one operand-size expression, one
address-size expression, one FS/GS selector-loader route, and one `LOCK`
classifier. No duplicate form mechanism, profile-gate mismatch, or
validation-to-publication divergence was reproduced. Privileged table/frame,
VM86, paging, task, CR/DR/TR, and debug hits are excluded by the active packet
and transfer once to T341.

## Executed Verification

The following exact current-gate owner tests passed: operand address, prefix
attributes, FS/GS stack, LSS/LFS/LGS, moffs, GPR MOV, and Sreg MOV. The full
current gate, documentation governance, and whitespace verification are
recorded by the S2 delivery commit.

## Result

S2 finds no runtime defect and adds no duplicate smoke. The remaining T340
work is S3's non-privileged `0F` integer/control reconciliation; state-owned
rows remain T341 transfers.
