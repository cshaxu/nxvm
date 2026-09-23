# T435 S8 - 80386DX Control-Transfer Rebaseline

## Defect corrected

The former `CALL/JMP/RET/IRET/INT` template crossed five unrelated opcodes
with eight generic path labels. That created both impossible entries (for
example a near `INT`) and omitted real variants (such as register versus
memory near indirect `CALL`). It was not a valid manual-to-decoder ledger.

## Manual-derived replacement

| family | retained successful paths | source table |
| --- | --- | --- |
| `CALL` | near relative/register/memory; far real direct/memory; far protected direct/memory; same gate direct/memory; more-privilege gate with zero/parameter-copy direct/memory; task direct/memory | I386DX-PRM-1990 printed pp. 17-40--17-44 |
| `JMP` | near relative/register/memory; far real/protected direct/memory; same gate direct/memory; task-gate and TSS direct/memory | I386DX-PRM-1990 printed pp. 17-86--17-88 |
| `RET` | near and immediate; far real and immediate; far protected same/outer privilege and immediate variants | I386DX-PRM-1990 printed pp. 17-148--17-150 |
| `IRET` | real, protected same-level, protected outer-level, return-to-VM86 and task return | I386DX-PRM-1990 printed pp. 17-78--17-80 |
| `INT3`, `INT imm8`, `INTO` | real, protected same/inner, VM86-to-PL0 and task-gate success; `INTO` non-overflow is an additional non-transfer success | I386DX-PRM-1990 printed pp. 17-73--17-77 |

Operand size remains a legal context axis. It is not duplicated into a
synthetic opcode/path product; the manual tables apply the 16- or 32-bit
operand form to each stated path.

## Denominator change

| metric | before | after | explanation |
| --- | ---: | ---: | --- |
| base keys | 410 | 427 | replace 40 invalid generic entries with 57 real keys |
| legal context keys | 935 | 953 | only admitted control paths receive the existing size axis |
| canonical S2 keys | 1,345 | 1,380 | no invalid path is retained merely to preserve an old count |

All new keys are L3 and nonconforming (`missing-input`) until the later runtime
implementation publishes their decoded form, path and formula inputs. This is
an audit correction, not a timing implementation claim.

Markers: `M5:T435:S8:I386-CONTROL-PATH-REBASELINE:OK`;
`M5:T435:S8:I386-MANUAL-DECODER-PARTITION:IN-PROGRESS`.
