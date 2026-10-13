# T555 S8 Cross-Profile Controller Ledger

## Scope and sources

S8 closes the remaining selected DeskPro 386 controller route without changing
the Shared Lib, Emulator, or x86 corpora.  The governing source is *Compaq
Deskpro 386 Technical Reference*, Volume I, printed pages 2-35--2-36, 2-58,
and 2-83--2-84, previously rendered and recorded in
`t555-s3-rtc-keyboard-source-ledger.md`.

The later D3PE processor description is retained as a source conflict, not as
an implementation substitute.  It describes a direct output-port SLOWD*
connection.  The selected Volume I Figure 2-15 labels output-port bits 2 and
3 unused.  S8 therefore must not reintroduce the retired output-bit route.

## Current-owner reconciliation

| Source member | Current owner/path | Initial state | S8 disposition |
| --- | --- | --- | --- |
| A3 enable system-speed control | `x86_kbc8042_write_command()` | Unhandled | KBC command extension enters the frozen Model40 board binding; no output-port repurpose. |
| A4 toggle COMMON/DIP-4 selection | `x86_kbc8042_write_command()` | Unhandled | Same selected board binding; its fixed input-port bit 4 is the only switch observation. |
| A5 special read | KBC output path | Unhandled | KBC owns the read datum and must keep OBF/IRQ deasserted; Model40 supplies only the selected bits. |
| SLOWD auxiliary-PIT counter 2 | `d4_platform.c` | Unwired | Model40 owns the gate/trigger and profile count; the existing auxiliary PIT owns one-shot state. |
| extended CPU hold | Core scheduler | Unwired | Board reports the existing hold input; Core advances time and DMA while withholding CPU retirement. |
| A1/A2/A6 | no sufficient selected-source definition | Undefined | Explicitly retained as non-implemented; no guessed opcode behavior. |

## Required invariants

- The KBC remains the only parser for port 64h commands and the only owner of
  data/status/IRQ delivery.
- Model40 is the only profile that installs the command extension and speed
  hold route.  XT, 5170, and default AT retain their existing generic KBC
  behavior.
- A refresh request remains a normal Core refresh transaction.  The auxiliary
  one-shot can extend the CPU hold after that transaction; it does not create a
  second refresh scheduler.  DMA keeps its existing arbitration path during
  the extension.
- The count is a selected L2 model on the existing 4 MHz auxiliary-PIT ratio;
  no physical DCLK or MCU timing claim is made.

## Verification plan

Direct KBC tests will prove A3/A4 dispatch and A5 no-OBF/no-IRQ behavior.
Model40 tests will prove the fixed switch observation, auxiliary-PIT mode-1
trigger, reset clearing, CPU non-retirement, and DMA progress.  A real
Machine receiver will traverse guest ports, KBC, Model40 binding, PIT, Board,
and Core.  The four App route matrix must prove Model40 positive selection and
the other three Apps' non-applicability before repository-wide qualification.

## Implemented owner chain and model boundary

`x86_kbc8042` remains the sole parser for commands written through port 64h.
Its optional board binding handles only a selected personality command. A3 and
A4 therefore do not create an output-buffer byte or an IRQ; A5 stores its one
special read datum in the KBC-owned response slot, explicitly without setting
OBF or delivering IRQ1. Reset/flush discards that datum along with ordinary
pending output.

Only the Model40 profile binds the extension. Its D4 platform consumes A3 and
A4, forms A5 data from the existing input-port and command-byte facts, and
gates existing auxiliary PIT counter 2 from the existing refresh output. The
board exposes that counter's output as the existing CPU-HOLD input. Core
advances guest time and leaves normal scheduler/DMA service available while
that input is asserted; it does not retire a CPU instruction in that interval.

The selected source establishes command purposes, the 4 MHz DCLK source and
auxiliary counter-2 mode 1, but does not publish a measured one-shot count or
complete A3/A4 electrical polarity. `speed_pit_count = 18` and the fixed
input-port-bit-4 selection interpretation are explicit L2 software models.
They do not claim physical DeskPro timing. A1, A2 and A6 remain unimplemented
because the selected material gives no sufficient behavioral definition.

## Direct and receiving proof

- KBC controller contract: a generic unbound controller leaves A3/A4/A5
  unhandled; the bound fixture proves A3 has no OBF/IRQ, A5 returns a special
  byte without OBF/IRQ, and reset flushes that special response.
- Model40 D4 contract: A1 remains unhandled; A3/A4 select the L2 speed gate;
  A5 reflects the documented keyboard-size and command-byte IRQ bits.
- Real Model40 receiver: guest 64h/60h command traffic proves A3/A4 remain
  response-free and A5 returns the selected D4 value without OBF; changing
  the command byte changes only the documented A5 bit.
- Refresh/HOLD receiver: primary refresh triggers the mode-1 auxiliary PIT;
  the hold predicate asserts after the selected count and clears on reset.
  Core's run loop consumes that predicate before CPU retirement, retaining the
  existing scheduler path used by DMA.

## Final automated qualification

Both original Ninja/ccache release trees were rebuilt. Complete CTest passed
`568/568` on x64 (546 unit, 22 integration; 251.74 s real) and `568/568` on
x86 (546 unit, 22 integration; 90.41 s real). Core source/test manifests,
Core corpus boundary, product artifact-root and NXVM documentation governance
checks pass. The four selected profile pairs were rebuilt and PE-width
verified without changing adjacent owner INIs:

| Artifact | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_default_0_5_0555_x64.exe` | `8FE959E69B8787D597E50E6E61C1A1011FD0AD00CB963D5B2557280EFE71D0C5` |
| `assets/nxvm/nxvm_default_0_5_0555_x86.exe` | `DA7FFE5E67B207B04C9104864CBDCCA88FE326C269F46B42B3078152B7202AFF` |
| `assets/my5160/nxvm_xt_0_5_0555_x64.exe` | `17A21B13A12C23909773534E54288B79676557D9EE765B40C38C4C8F76FCFC88` |
| `assets/my5160/nxvm_xt_0_5_0555_x86.exe` | `B1DA07E7C2A6F04EA9A233576F3DD583E106B3F23F1C8E940B5EA7BCBD5F9CDF` |
| `assets/my5170/nxvm_at_0_5_0555_x64.exe` | `C65B200E952F73991B1C242D5FC7624FEAC38B29C45A724DA871C06B2A832CB5` |
| `assets/my5170/nxvm_at_0_5_0555_x86.exe` | `F147FCE0A6B56DB72574B4DD60E0BE5655CD1F5409C94966AA4A106BDACCA3C1` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x64.exe` | `826E3389D7FA981EF828D2B23940693CD928709FF1E91ED531C4758940F25B32` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x86.exe` | `6E54D4143CD2D55B085E9877010DA17BFF4871802D95832A6085EB774C62EFF7` |

Manual desktop and physical electrical timing qualification were not run and
are not inferred from these automated results.
