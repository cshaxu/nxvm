# T323 S3: Protected Same-CPL 16-Bit Gate Entry

## Scope And Production Result

This record closes T323 S3's Intel 80286/80386 protected, non-VM86,
same-CPL 16-bit IDT-gate slice. The owner-local
`core-machine-protected-16-gate-s3-smoke` uses prepared GDT, IDT, code, stack,
and PIC state; it does not claim 32-bit frames, CPL transitions, TSS stack
switching, VM86, task/call gates, or reset policy.

The focused matrix reproduced three coupled local serializer defects in
`_ser_int_protected_16`: it accepted only `INTGATE_16`, used one boolean both
for software-origin DPL policy and error-frame shape, and cleared IF without
clearing TF according to the actual gate type. The repair admits
`TRAPGATE_16`, passes `software_origin` and `error_frame` independently,
applies the DPL check only to software origin, writes the optional error-code
word only for an exception, clears IF only for an interrupt gate, and clears
TF for either gate. The 32-bit same/outer serializers and all descriptor,
stack, PIC, and exception-finalizer helpers remain unchanged.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286 and 80386 ordinary 16-bit interrupt gate | `INT ib` reaches a 16-bit interrupt gate and HLT handler. The frame is three words (IP, CS, FLAGS), SP falls by six, IF and TF are clear live, the saved FLAGS retain CF/IF/TF, and non-stack GPRs are unchanged. | Complete. |
| 80286 and 80386 ordinary 16-bit trap gate | The matching `INT ib` route uses the same three-word frame, clears TF, retains IF live, and preserves non-stack GPRs. | Complete. |
| 80386 attributes | `66`, `67`, and `66 67` prefixed `INT ib` execute through the 16-bit gate, retain a word frame, and save the prefix-correct continuation IP. | Complete for declared prefixes. |
| Prefix and LOCK rejection | On 80286, `66`, `67`, and combined forms are `#UD` with complete CPU/cache nonpublication. On 80386, `LOCK INT` is likewise `#UD` before entry. | Complete. |
| Rejected gate entry | 80286 CPL3 software origin against DPL-zero gate, invalid gate type, and non-present gate terminate through the established no-IDT boundary without target/frame publication; complete CPU/cache state remains the prepared state. | Complete for declared boundary. |
| Existing error-code consumer | A protected existing `#GP` source enters a 16-bit vector-13 gate. The four-word image is error code, delivered event point IP, CS, FLAGS; target handler and delivered-`#GP` diagnostic are observed. | Complete for the retained source boundary; it adds no new fault producer. |
| External IRQ0 | A pending IRQ0 runs after a successful NOP through a 16-bit interrupt gate, saving continuation IP one, HLTing the handler, and publishing PIC ISR/IRR ownership. The three-word frame has no error word. | Complete. |

## Verification And Transfer

The target uses target-local GCC/Clang `-Wall -Wextra -Wpedantic -Werror` and
is registered exactly once as `current.core-machine-protected-16-gate-s3-smoke`.
Its marker is `M5:T323:S3:PROTECTED-16-GATE:OK`.

The refreshed T323 developer artifact is `build/output/nxvm_0_5_0323.exe` with
SHA-256 `8C92EF746B5AFA46DC64EFA9095FDC0906CF57FDDBE8981FE05A383CA210D049`.
The declared transfers remain 32-bit/outer/VM86 entry, CPL3 external-event DPL
bypass composition, TSS stack switching, task and call gates, IRET, generic
exception policy, and PIC redesign. The protection-and-privilege Queue package
owns their later composition.
