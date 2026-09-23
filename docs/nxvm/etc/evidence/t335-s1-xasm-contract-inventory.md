# T335 S1 xasm Contract Inventory

## Owner And API Decision

`core_product_utils` is the only public xasm owner. S2 replaces its three raw
pointer/length-return wrappers with these bounded, status-returning operations:

```c
type_status core_product_utils_assemble(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32);
type_status core_product_utils_assemble_paragraph(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32);
type_status core_product_utils_disassemble(const type_unsigned_8 *code,
    STD_SIZE_T code_bytes, C_CHAR *statement, STD_SIZE_T statement_capacity,
    STD_SIZE_T *out_statement_bytes, C_INT flag32);
```

Inputs are byte spans, not assumed NUL-terminated strings. S2 rejects a single
assembly statement or paragraph line larger than 255 bytes, because that is the
existing engine's fixed internal representation. It rejects a code output below
the complete result, text output below the complete result plus terminator, a
zero capacity, malformed input, invalid mode, allocation failure, and any
engine error. It returns `TYPE_STATUS_OK` only after producing the full result;
all failure paths leave output bytes and `*out_*_bytes` unchanged. The old
`core_product_utils_aasm32`, `_aasm32x`, and `_dasm32` public symbols are
removed in S2 rather than retained as an unbounded compatibility bypass.

`aasm32*.h` and `dasm32.h` become implementation-private declarations. S2 may
alter their internal signatures as needed, but no product caller may include or
call them directly.

## Entry And Caller Matrix

| Existing entry / caller | Current buffers | Risk / failure publication | S2 migration |
| --- | --- | --- | --- |
| `core_product_utils_aasm32` -> `aasm32` | caller supplies unbounded statement/code; engine `astmt[256]`, instruction code max 15 | `MEMCPY(..., 256)` reads past short inputs and cannot reject long input; emitted prefixes/code have no code capacity | bounded `assemble`; debugger supplies exact statement span and `acode[15]`; status failure prevents guest write. |
| `core_product_utils_aasm32x` -> `aasm32x` | unbounded statement/code; dynamic `instr[]`; every line `stmt[256]`; each output up to 15 | unchecked allocation, count-size overflow, line overwrite, and partial final code copy | bounded `assemble_paragraph`; profile firmware retains allocated capacity but passes it explicitly; facade stages output and commits only complete code. |
| `core_product_utils_dasm32` -> `dasm32` | unbounded code/text; context text fields are 256 bytes | input decoder can read past code span; final `STRCAT` writes past caller text | bounded `disassemble`; debugger uses `ucode[15]` and its local statement capacity; lifecycle adapter migrates its callback contract. |
| debugger assembly prompts | `cmdAsmBuff[256]` / `astmt[256]`, `acode[15]` | old return zero ambiguously represents syntax/failure; no explicit output span | pass exact bounded span/15 output; no guest write unless `TYPE_STATUS_OK`. |
| debugger disassembly listings | `ucode[15]`, caller-provided statement | no code/text capacity supplied | pass 15-byte code span and each exact text capacity; failure prints retained error path. |
| `vm_session_profile_firmware_assemble` | statement literal, allocated `line_count * 15` | caller capacity calculation exists but engine cannot consume it; no paragraph input bound | preserve allocation ownership, pass input bytes/capacity; output remains unpublished/free on failure. |
| `vm_session_debug_disassemble` | callback `C_CHAR *statement`, `type_unsigned_8 *code` | callback ABI omits both capacities | migrate the private VM/core debug provider callback and all consumers to bounded status form; no adapter preserving raw bypass. |
| `core_product_xasm_smoke` | `code[16]`, `statement[256]` | success-only coverage | replace with boundary regression covering exact capacity, one-byte-short output, overlong line/input, malformed input, and allocation failure sentinels. |

## Internal Strategy And Required Tests

- `aasm32_execute` accepts a statement span only after bounded copy into its
  256-byte local buffer; it receives a 15-byte staging code array.
- Paragraph parsing validates each line and allocation multiplication before
  allocation, checks allocation, stages all instruction output, and copies to
  caller output only after total length fits.
- Disassembly receives an explicit code span and refuses every internal decode
  read beyond it; it formats into owned context storage, measures the completed
  text, then copies once if the caller capacity fits.
- S3 uses source-specific allocator redirection only for the xasm smoke's
  copied engine source. It proves sentinel output and result length remain
  unchanged on every failure class, plus representative `nop` single,
  paragraph, and disassembly success.

## Similar-Issue Result

Repository-wide symbol scan finds exactly the five production caller classes
listed above and one success-only product smoke. The only direct xasm engine
calls are the three `core_product_utils` wrappers. No external/generated source,
firmware media, or third-party import is required. Opcode breadth and xasm's
legacy internal implementation structure remain outside this capacity/failure
contract.
