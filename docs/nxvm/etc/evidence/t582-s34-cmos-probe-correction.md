# T582 S34 — fixed-work CMOS setup correction

The fixed-work probe launched executables from a directory different from its
copied-media working directory. MyPC110 resolves its CMOS files relative to
the executable, not the process working directory. Inspection found empty
`mypc110-dstn.cmos` files next to the S31 and S32 fixed-work executables;
the copied configured seeds in their run directories were therefore not the
seeds loaded by those programs. An empty file is a supported first-run case.
The equal instruction, tick and PC checkpoints remain measurements of that
empty-CMOS startup path; they must not be represented as configured-CMOS
measurements or as evidence of equal complete machine state.

This affects S31's fixed-work memory comparison and S32's fixed-work deadline,
preview, horizon and prefix-classifier experiments. It does not invalidate
their recorded timing numbers, alter the production code, or imply that the
separately registered unit/integration tests used the same fixture. S33's
corrected product-path game probe placed each executable beside its copied
CMOS and is recorded separately in [S33](t582-s33-prefix.md).

S34 repeated the prefix-classifier fixed-work comparison with each executable
and an identical configured CMOS seed in its own private run directory. Each
run reported the same seed hash `608DF3ABC3CDD9BB3E35015D2D7AA9D8756567E080643B01E593FF99D1616034`,
10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 24236 | 22848 | 22752 | 20473 |
| x86, ms | 24512 | 24736 | 25658 | 26260 |

The first and reverse comparisons change direction on both widths; baseline
variation is substantial. This configured-CMOS measurement does not establish
a stable speedup or regression, complete state equivalence, or game usability.
The S33 code still removes an out-of-translation-unit classifier call without
introducing another classifier; its production EXE-size cost and functional
qualification are recorded in S33. Log SHA-256:
`ECB40D6E7C2C62470ADDDF698B9E7FDFF2762BAEE29313B43D842E2044FE9D2B`.
