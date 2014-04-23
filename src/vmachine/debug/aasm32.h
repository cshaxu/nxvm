/* This file is a part of NXVM project. */

#ifndef NXVM_AASM32_H
#define NXVM_AASM32_H

#include "../vglobal.h"

/* assemble single instruction */
t_nubit8 aasm32(const t_strptr stmt, t_vaddrcc rcode);
/* assemble a paragraph of instructions */
t_nubit32 aasm32x(const t_strptr stmt, t_vaddrcc rcode);

#endif
