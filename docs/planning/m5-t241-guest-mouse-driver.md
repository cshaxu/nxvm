# M5 T241: Guest Mouse-Driver Corpus

## S1: Contract And Corpus

**Status:** complete.

T241 closes the evidence gap left deliberately by T229.  T229 proves the
bounded host mouse event, 8042 AUX output, PIC IRQ12, and a boot-sector IVT
handler.  It does not claim a DOS guest program can use that route as a small
driver/application interaction.

The T241 corpus is one owner-built `MOUSE241.COM` fixture installed only into a
temporary clone of the supplied DOS FDD image.  After DOS reaches its normal
prompt, the smoke types its command through the retained keyboard route.  The
COM program installs an ordinary IVT `74h` handler, sends AUX reset,
identify, and enable-reporting commands through `64h`/`60h`, consumes their
IRQ12-delivered replies, then consumes one mapped relative packet. Its small
application waits for the complete observed sequence and writes a pass marker
to guest text VRAM before returning through normal DOS process exit; the
retained owner-level port probe provides the exact byte comparison.

Route and ownership remain exactly:

```text
host relative event -> platform ingress -> session boundary -> default profile
mapper -> core KBC AUX -> PIC IRQ12 -> CPU IVT 74h -> guest COM handler/app
```

- The fixture is guest code, not an `INT 33h` implementation and not a claim
  of compatibility with an external mouse driver.
- The host harness may clone/install media, type the command, submit one
  normalized relative event, and observe snapshots.  It never writes guest
  RAM/BDA/DOS APIs or KBC/PIC state after session creation.
- Required observations are AUX reset (`FA AA 00`), identify (`FA 00`),
  reporting enable (`FA`), one `(+5,+3,left)` mapped packet (`29 05 FD`),
  IRQ12 delivery, and a guest-visible application result.
- No wheel, extra buttons, sample-rate/scaling, remote/status commands,
  `INT 33h`, host cursor policy, or extra host-side queue is admitted.

Stop if this requires another KBC/PIC path, a BIOS/DOS service shortcut,
direct host guest-memory mutation, or changes to Console/debugger/start/boot
behavior.  Existing `core-machine-kbc-aux-port-smoke` remains the port-level
contract; the new system smoke supplies the missing DOS-level corpus.

## S2: DOS Fixture And Corpus Execution

**Status:** complete.

`vm-mouse-driver-dos-smoke` clones the owner-supplied FDD image and installs
the self-contained `MOUSE241.COM`; no guest media is changed in place or
committed. The smoke boots DOS, waits for its normal prompt, types the COM name
through the retained guest keyboard route, and waits until the program has
consumed the six AUX initialization replies. It then submits exactly one
normalized host `(+5,+3,left)` event through the platform transport and session
boundary.

The guest handler records every IRQ12 `60h` read in COM-owned storage and the
small guest application waits for all nine bytes before writing `O` to text
VRAM and returning through `INT 21h`. This proves ordinary DOS loading, guest
driver installation, reset/identify/enable flow, packet delivery, and a
guest-visible application result. The byte-exact controller proof remains
`core-machine-kbc-aux-port-smoke`: it locks `FA AA 00`, `FA 00`, `FA`, and the
mapped `29 05 FD` ordering. The system and port probes cover distinct boundaries
over the same KBC/PIC owner.

## S3: Matrix And Closure

**Status:** complete.

`cmake --build --preset current-gates-gcc --parallel 4` passed 35 static and
ownership gates plus all 78/78 current CTest smokes. The matrix includes the
new DOS corpus, retained AUX/KBC probes, FDD/HDD boot, DOS prompt/keyboard,
CGA/EGA, Console, and debugger coverage. Linux source-contract validation
passed; no WSL or POSIX runtime was introduced.

`cmake --build --preset current-gcc --parallel 4` produced
`build/output/nxvm_0_5_0241.exe`, SHA-256
`5E0D6F1D5600503937F831E87709A055EAB35F5815C2BE2484450DB50B396E1B`.
The artifact has the normal retained NXVM Console/debugger/boot behavior; T241
adds corpus evidence only and changes no guest-device implementation.
