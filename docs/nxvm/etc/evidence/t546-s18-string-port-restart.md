# T546 S18 String And Port Restart

## Authority and disposition

The original source is Intel's *8086 and 8088 User's Manual* (1978/1981
edition), archive file `intel-8086-8088-users-manual-1981.pdf`, page 2-42.
It states that a repeat string sequence recognizes an interrupt before the
next element, resumes from that point after the interrupt, and on an 8086 or
8088 with multiple prefixes remembers only the prefix immediately before the
string instruction.  The owner-managed archive is outside this repository.

The 80386DX Programmer's Reference Manual, archive SHA-256
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`,
pages 3-27--3-28 and 9-2, specifies the later per-element restart rule: a
fault restores the current iteration and an interrupt observes the completed
iteration's registers before the next element starts.

The repair keeps the existing one-element execution quantum.  For 8086/8088
only, after a successfully completed repeat element and only when NMI or INTR
is actually accepted, the interrupt return IP becomes the final prefix of a
multi-prefix sequence.  Without an accepted interrupt, the existing restart
at the full instruction remains; 80186, 80286 and 80386 retain that existing
path.  No replay state, cache, board condition or public interface is added.

## Handler and port sweep

The early-family `MOVSB/W`, `CMPSB/W`, `STOSB/W`, `LODSB/W`, and `SCASB/W`
branches now use the same checked helper boundary as their 80386 branches.
Thus a failing current element cannot decrement CX or set repeat continuation
after the helper has recorded an exception.  The helper still leaves all
prior completed elements intact.

All INS/OUTS branches already use their checked path.  `_p_ins` verifies the
destination before `transfer_port`, then writes the element and advances DI;
`_p_outs` reads the source before `transfer_port`, then advances SI.  In both
cases `complete_port` is called only after the transfer has fully succeeded.
The existing port-string protected-failure test proves a rejected next element
has no transfer or completion while the prior element remains complete.  This
S does not create an undo or a second port-effect owner: a provider's
`transfer_port`/`complete_port` contract remains the single commit boundary.

## Direct regression

`cpu_execution_bus_smoke` now drives `CS: REP MOVSB` through both accepted
INTR and NMI paths for 8086, 8088 and 80186.  It verifies that exactly one
element completed before entry, CX is one less, the handler target is reached,
and the saved IP is the final `REP` prefix (`0101h`) only on 8086/8088; the
80186 return remains the original instruction start (`0100h`).

The focused string, port-string and bus regressions pass on x64 and x86.
The complete repository-only unit set passes 506/506 on each width through
bounded CTest partitions.  The x86 stack-frame negative verifier was rerun as
its complete final partition after one transient parallel fixture failure; it
then passed with the same source and test bytes.  The relevant x86 manifest,
corpus, negative-verifier and Types gates pass on both widths.

All four receiving PC products were rebuilt as optimized stripped 0.5.0546
artifacts.  SHA-256 identities are:

```
my5160 x64  FC0C643C1DB452BD8EEA0DAB64E53AEB6CF5A07638650FB5FBE1952F15B67FB0
my5160 x86  A419D14D92FB364C58E2CBFF7BBEAB02AE9CBD1CA40277B16BAB06FCEA5A5231
my5170 x64  67757E59759E8663E8471911D508E72C28139A1D2C7268A4F8D55AA682EA7302
my5170 x86  5DE51E3957773002237393A9CA1FC9CB95C098A1A21553B58394138A8F0D78F8
model40 x64 F8D81ADCC2F973FA39EC4AA8390693C1C6F1F7D3C78949380AA9D4D94E7FD132
model40 x86 60564711C45CAE771137C0A1B19122E63238E9DAB61034D5BB6F4040F38FEAFC
default x64 F2998DB8E5789C8236AB0D3EC1ADE29C410A26EDDE176FA020CE7D62F552FD39
default x86 1BFE4BAB7C92DAE417ECC9E5918FB04A1A8E67FD0EC70B2A60D596C1A816D4B4
```

MyNES binaries and the user-owned snapshot remain untouched.  This proof does
not claim a new external-media integration run; it qualifies the shared CPU
repair and its receiving product builds.
