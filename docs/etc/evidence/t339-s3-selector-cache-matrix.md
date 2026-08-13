# M5 T339 S3: 80286 Selector Query And Cache Materialization Matrix

## Mechanism Inventory

S3 audits one selector/table lookup and cache-publication mechanism, rather
than treating the opcode spellings as unrelated work:

| Form group | Decode owner | Validation/materialization owner | Current-gate evidence |
| --- | --- | --- | --- |
| SLDT, STR, LLDT, LTR (`0F 00 /0`--`/3`) | `INS_0F_00`, `_d_modrm` | `_s_load_ldtr`, `_s_load_tr` | `current.core-machine-dttr-s61-smoke`; `current.core-machine-segment-selector-smoke` |
| VERR, VERW (`0F 00 /4,/5`) | `INS_0F_00` | `_s_check_selector`, `_s_read_xdt` | `current.core-machine-verr-verw-s58-smoke`; selector owner |
| LAR, LSL (`0F 02/03`) | `LAR_R32_RM32`, `LSL_R32_RM32` | `_s_check_selector`, `_s_read_xdt` | `current.core-machine-lar-lsl-s57-smoke`; selector owner |
| MOV Sreg and legacy segment stack | `MOV_RM16_SREG`, `MOV_SREG_RM16`, `_e_pop_sreg` | `_d_modrm_sreg`, `_ksa_load_sreg`, `_s_load_{es,ss,ds}` | `current.core-machine-sreg-mov-smoke`, `current.core-machine-legacy-sreg-stack-smoke`; selector owner |
| LES/LDS (`C4/C5`) | `LES_R32_M16_32`, `LDS_R32_M16_32` | `_e_load_far`, selector preparation/load | `current.core-machine-les-lds-s41-smoke`; selector owner |

The caller/write sweep found no parallel selector-cache publication path in the
allocated forms.  `_ksa_prepare_*_sreg` is deliberately not unified with a
simple data-selector load: code/stack privilege and outer-stack layouts are
owned by later gate/return work.

## 80286 Evidence

The S3 extension to `core-machine-segment-selector-smoke` uses a local 80286
bootstrap with 16-bit IDT gates and no FS/GS setup.  It proves all of the
following using actual 80286 execution:

| Boundary | Evidence |
| --- | --- |
| Valid materialization | LES/LDS, LAR/LSL, VERR/VERW, MOV DS and MOV r16,DS, POP DS, and PUSH ES all execute with the specified selector/cache, register/stack, EIP, and ZF effects. |
| `0F 00` register and memory forms | SLDT/STR/LLDT/LTR execute with their register or DS-memory source/destination.  LLDT/LTR publish their table-register caches only after source acquisition; SLDT/STR write the expected low selector word. |
| GDT/LDT selection | LLDT loads a present LDT descriptor; LAR/LSL/VERR/VERW then query TI=1 selector `0x000c` from an LDT entry at its real offset, proving selector-table materialization rather than a GDT fallback. |
| Null and invalid cache load | Null DS succeeds with an invalid DS cache.  Non-present DS, execute-only DS, and null SS deliver their actual 80286 `#NP`/`#GP` paths through local vector 11/13 16-bit gates, reach the handler boundary, and retain the target cache and non-target visible state. |
| Existing owner breadth | DTTR, LAR/LSL, VERR/VERW, Sreg-MOV, legacy Sreg stack, LES/LDS, and the retained 80286 protected-mode owner provide real-mode rejection, descriptor class, RPL/DPL/present/accessed, memory source, table-image, and successful interrupt-order regressions.  The S3 matrix accepts only their explicit 80286 rows. |

## Exact Transfers

- Accepted T328 remains the sole pre-386 `LOCK` policy owner.
- FS/GS, LSS/LFS/LGS, `66/67`, VM86, paging, debug, VME/PVI, and all 32-bit
  selector/image behavior transfer to T341/T342.
- Call/interrupt/task gates, outer-stack selection, and delivered 16-bit
  frame composition transfer to T339 S4; same/outer return publication to S5;
  TSS type/busy/backlink transition semantics to S6.  S3's local 16-bit gates
  prove only the selector-load failure boundary and do not claim those later
  mechanisms closed.

## Verification

The complete S3 acceptance runs its seven form-owner smokes plus the retained
80286 protected-mode owner, fresh configuration, exact registrations,
artifact identity, documentation governance, whitespace validation, and the
full current gate.  No S3 CPU production defect was reproduced.

This evidence retires into the T339 history record when T339 closes.
