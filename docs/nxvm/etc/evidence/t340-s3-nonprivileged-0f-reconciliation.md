# T340 S3: Non-Privileged 80386DX `0F` Reconciliation

## Metadata And Dispatch Crosswalk

| Secondary family | Metadata and sole dispatch owner | Actual proof | Disposition |
| --- | --- | --- | --- |
| `80`--`8F` near conditional branch | 80386 metadata; `JO_REL32` through `JG_REL32` | `current.core-machine-control-transfer-smoke` | Accepted: 16/32 operand form, condition, target, EIP, and access/fault matrix remain T303's actual proof. |
| `90`--`9F` SETcc | 80386 metadata; `SETO_RM8` through `SETG_RM8` | `current.core-machine-setcc-smoke` | Accepted: all conditions, register/memory publication, prefix/profile rejection, and source fault proof are one owner matrix. |
| `A3/AB/B3/BB`, `BA /4`--`/7` | 80386 metadata with `BA` ModRM extension gate; `BT/BTS/BTR/BTC` and `INS_0F_BA` | `current.core-machine-bit-test-smoke` | Accepted: register/memory/index/immediate forms, defined CF, width/address selection, rejection and fault publication are grouped by the shared bit/memory writer. |
| `A4/A5/AC/AD` | 80386 metadata; SHLD/SHRD immediate and CL handlers | `current.core-machine-double-shift-smoke` | Accepted: direction, count-zero, defined FLAGS, width, memory fault, and rejection proof remain one double-shift owner matrix. |
| `AF` | 80386 metadata; `IMUL_R32_RM32` and its signed multiply/FLAGS writer | `current.core-machine-imul2-smoke` | Accepted: register/memory, 16/32 form, CF/OF, profile/`LOCK`, and source-fault evidence remain at the shared IMUL owner. |
| `BC/BD` | 80386 metadata; `BSF_R32_RM32` and `BSR_R32_RM32` share scan/conditional destination publication | `current.core-machine-bit-scan-smoke` | Accepted: zero/nonzero ZF disposition, destination retention, width, memory, and fault behavior are one bit-scan matrix. |
| `B6/B7/BE/BF` | 80386 metadata; MOVZX/MOVSX read and width/publication handlers | `current.core-machine-movx-smoke` | Accepted: byte/word source, zero/sign extension, 16/32 destination, profile/`LOCK`, memory and fault proof are one MOVX matrix. |

## Excluded Secondary Forms

`00/01/02/03/06`, `20`--`26`, and the FS/GS/LxS form group are selector,
table, control-register, or S2-owned form mechanisms.  They are not evidence
for S3 and transfer once to T341 where their remaining privileged state or
descriptor-table composition is required.  Metadata-invalid and later-CPU
entries remain architectural `#UD`/outside-80386 classifications, not missing
80386 handlers.

## Similar-Issue Sweep

The source and registration sweep used:

```powershell
rg -n "insTable_0f\[0x(80|81|82|83|84|85|86|87|88|89|8A|8B|8C|8D|8E|8F|90|91|92|93|94|95|96|97|98|99|9A|9B|9C|9D|9E|9F|A3|A4|A5|AB|AC|AD|AF|B3|B6|B7|BA|BB|BC|BD|BE|BF)" src/core/machine/cpu_instructions.c
rg -n "core-machine-(control-transfer|setcc|bit-test|double-shift|imul2|bit-scan|movx).*smoke" CMakeLists.txt
```

The dispatcher has one reachable handler per valid assigned metadata form;
`INS_0F_BA` alone owns the valid `/4`--`/7` extension selection.  No duplicate
integer writer, private FLAGS path, or unregistered replacement smoke was
found.  The only hits outside this S are T341's privileged/system state forms.

## Executed Verification And Result

The seven named current-gate owner smokes passed.  The S3 delivery also runs
the complete current gate, documentation governance, and whitespace check.
No runtime defect was reproduced, so S3 adds no duplicate test or production
change. T340 can proceed to its closure audit, and all remaining state rows
remain T341 transfers.
