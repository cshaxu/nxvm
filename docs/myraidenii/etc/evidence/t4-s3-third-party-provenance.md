# T4 S3: Third-Party Video Reference Provenance

## Candidate

The public `spacestate1/Arcade-Raiden2_MiSTer` repository was inspected only
through its README and license declaration.  No checkout, RTL, binary,
generated file, ROM, table or implementation detail was imported or retained.

Its README identifies the project as a GPL-3.0 FPGA recreation.  More
importantly, it explicitly states that its hardware behavior was derived from
MAME Seibu-driver source files, including the RAIDEN II video/sprite-related
drivers.  It is therefore not independent original-board documentation or an
independent behavioral oracle for this product.

## Policy Comparison

MyRaidenII's source policy permits MAME and other emulators as research
references only.  It forbids copying, transliterating, linking or
redistributing their implementations, decryption algorithms or tables.  A
third-party implementation whose publisher states that it derives behavior
from those same sources cannot upgrade MAME-derived claims to independent
hardware evidence merely by reproducing them in another language or hardware
description.

Its GPL-3.0 license is an additional reason it is not an import candidate for
this MIT repository, but the rejection does not depend on license alone: the
missing receiver facts remain absent even if licensing were compatible.

## Result

The candidate contributes no admissible fact for the `0x6ce--0x6cf` flags
receiver, adjacent sprite-control family, sprite decryption, palette, layer
priority or frame composition.  It must not be used as a source of algorithms,
tables, register behavior, expected pixels or test vectors.

The existing source-evidence blocker remains unchanged.  A later Video
implementation requires original or independently reproducible hardware facts
that cover the complete receiver prerequisites recorded by T4 S1 and S2.  This
disposition does not prohibit citing the project historically; it only prevents
its implementation from becoming a hidden specification.
