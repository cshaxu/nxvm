/* This file is a part of NXVM project. */

// Floppy Disk Drive: 3.5", 1.44MB Floppy

#ifndef NXVM_VFDD_H
#define NXVM_VFDD_H

#include "vglobal.h"

#define VFDD_GetByte(id)   (0x0001 << ((id) + 7))

#define VFDD_NHEAD          0x0002
#define VFDD_NCYL           0x0050

typedef struct {
	t_nubit8  cyl;                          /* vfdc.C; cylinder id (0 to 79) */
	t_nubit8  head;                              /* vfdc.H; head id (0 or 1) */
	t_nubit8  sector;                         /* vfdc.R; sector id (1 to 18) */
	t_nubit8  nsector;            /* vfdc.EOT; end sector id (default is 18) */
	t_nubit8  nbyte;            /* vfdc.N; bytes per sector (default is 512) */
	t_nubit8  gaplen;       /* vfdc.GPL; gap length of sector (default is 3) */

	t_bool    flagro;                                /* write protect status */
	t_bool    flagexist;                    /* flag of floppy disk existance */

	t_vaddrcc base;                                /* pointer to disk in ram */
	t_vaddrcc curr;                               /* pointer to current byte */
	t_nubitcc count;                             /* count of remaining bytes */
} t_fdd;

extern t_fdd vfdd;

t_bool vfddRead(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count);	// From FDD To RAM, count sectors
t_bool vfddWrite(t_nubit8 *cyl,t_nubit8 *head,t_nubit8 *sector,t_vaddrcc memloc,t_nubit8 count);	// From RAM To FDD, count sectors



void vfddFormat(t_nubit8 fillbyte);
void vfddInit();
void vfddFinal();

#endif