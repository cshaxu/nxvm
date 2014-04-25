/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VFDD_H
#define NXVM_VFDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_FDD "3.5\""

typedef struct {
	t_nubit16 cyl;    /* vfdc.C; cylinder id (0 to 79) */
	t_nubit16 head;   /* vfdc.H; head id (0 or 1) */
	t_nubit16 sector; /* vfdc.R; sector id (1 to 18) */
	t_nubit16 ncyl;
	t_nubit16 nhead;
	t_nubit16 nsector; /* vfdc.EOT; end sector id (default is 18) */
	t_nubit16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
	t_nubit8  gaplen;  /* vfdc.GPL; gap length of sector (default is 3) */

	t_bool flagro;    /* write protect status */
	t_bool flagexist; /* flag of floppy disk existance */

	t_vaddrcc base;  /* pointer to disk in ram */
	t_vaddrcc curr;  /* pointer to current byte */
	t_nubit16 count; /* number of transfer bytes */
} t_fdd;

extern t_fdd vfdd;

#define vfddSetPointer (vfdd.curr = vfdd.base +                               \
                        ((vfdd.cyl * vfdd.nhead + vfdd.head) * vfdd.nsector + \
                        (vfdd.sector - 0x01)) * vfdd.nbyte)
#define vfddGetImageSize (vfdd.nbyte * vfdd.nsector * vfdd.nhead * vfdd.ncyl)

void vfddTransRead();
void vfddTransWrite();
void vfddFormatTrack(t_nubit8 fillbyte);

void vfddRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
