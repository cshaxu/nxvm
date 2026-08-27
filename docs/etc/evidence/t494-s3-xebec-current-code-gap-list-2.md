# T494 S3 IBM 5160 Xebec Current-Code Gap List 2

`M5:T494:S3:XEBEC-LIST-2:OK`

The complete X1--X18 List-1 universe maps to one production chain:

```text
immutable XT profile -> copied Core HDC topology -> Core HDC Xebec substate
    -> Core DMA3/PIC IRQ5 bindings -> sole media registry -> response bytes
```

`hdc.c` remains the sole controller/CHS/sector-buffer owner; the VM profile
only supplies immutable ports, IRQ, DMA channel and Type-2 geometry. No
additional Xebec object, ATA task-file bridge, VM register state or media cache
exists.

| List 1 rows | Current owner/path and observed coverage | Full disposition |
| --- | --- | --- |
| X1--X4 | `controller_interface.h` has one tagged Xebec bus shape; `hdc.c` owns six-byte DCB, state and Type-2 geometry validation; `xt_5160_268.c` supplies the frozen Type-2 plan. Xebec and task-file paths branch before port handling. | Retain. The one admitted Type-2 profile is source-qualified and no task-file alias exists. |
| X5--X6 | `core_machine_xebec_response()` owns one completion/sense stack; `03h` reads and clears it. `core_machine_xebec_wiring_smoke` covers completion, invalid-command sense, Request Sense and reset. | Retain for sourced response bytes. Completion IRQ is not claimed because the manual's enable-mask bit meanings are not specified. |
| X7--X10 | `machine_board.c` registers the exact four-port directions, validates IRQ5/DMA3 and binds the existing Core DMA provider. `hdc.c` owns reset/select/data/mask writes; BYOB mapping remains a VM manifest boundary. | Retain exact registration and reset. `321h` hardware-status bits, `322h` readback encoding and `323h` mask-bit semantics remain L1: manual names the registers but does not define their values/bits, and PCjs labels its alternatives guesses. |
| X11--X12, X14--X16 | The finite opcode gate accepts sourced opcode families. Request Sense, reset and Initialize's eight-byte collection have focused coverage. Operations requiring ready/index/seek/ECC/format/diagnostic/long-drive behavior return the existing sourced error route rather than ATA behavior or a fabricated mechanical model. | Retain as explicit physical/unsupported boundary. No local source supports a richer logical result for these drive-dependent commands. |
| X13 | `08h` Read and `0Ah` Write use `hdc.c`'s sole sector buffer, CHS validation, frozen media registry and DMA3 binding; the smoke proves both DMA directions and terminal cleanup. | **Repair batch R1:** Write decrements `dcb[4]` across sectors, but Read loads only one sector and completes. The manual defines byte 4 as Read Data block count, so Read must use the same owner-local multi-sector progression rather than silently drop requested blocks. |
| X17--X18 | No HDC hardware-status bit state, rotation/seek delay, ECC/MFM model, service deadline, controller ROM bytes or physical input state is published. Existing comments and tests retain this absence. | Retain explicit L1/unsupported boundary; do not convert 86Box timers or PCjs guessed bits into L2/L3 behavior. |

## Similar-Issue Sweep

`rg -n "XEBEC|xebec|320|321|322|323|dma.*3|irq.*5" src tests CMakeLists.txt --glob "*.c" --glob "*.h" --glob "CMakeLists.txt"` finds only the tagged Core HDC implementation, Core board binding, immutable XT profile/session construction and the two focused Xebec/XT profile smokes. The only production-path asymmetry is R1 in `hdc.c`; no second controller, CHS state, media cache, ATA alias or profile-side register state exists.

## One Repair Boundary

R1 is one local correction inside the already sole HDC owner: make `08h` Read
consume its source-defined positive block count through the existing sector
buffer/DMA3 progression, with the same end-of-geometry and early-terminal
failure cleanup as Write. It must not add a second DMA path, counter, CHS
owner, media cache or service delay. The focused smoke must prove two-sector
read, one-sector behavior, terminal/error cleanup and retained ATA isolation.

Every remaining row is either already source-qualified at its sole owner or
explicitly impossible to implement faithfully from the accepted logical-media
and manual basis. R1 is therefore the whole immediate implementation batch.
