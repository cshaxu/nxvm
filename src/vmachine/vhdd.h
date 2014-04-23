/* This file is a part of NXVM project. */

// Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63

#ifndef NXVM_VHDD_H
#define NXVM_VHDD_H

#include "vglobal.h"

#define NXVM_DEVICE_HDD "20 cylinders"

typedef struct {
	t_nubit16  cyl;                          /* vfdc.C; cylinder id (0 to 79) */
	t_nubit16  head;                              /* vfdc.H; head id (0 or 1) */
	t_nubit16  sector;                         /* vfdc.R; sector id (1 to 18) */
	t_nubit16  ncyl;                                     /* should be 20 here */
	t_nubit16  nhead;                                    /* should be 16 here */
	t_nubit16  nsector;                        /* vfdc.EOT; should be 63 here */
	t_nubit16 nbyte;            /* vfdc.N; bytes per sector (default is 512) */
	t_nubit8  gaplen;       /* vfdc.GPL; gap length of sector (default is 3) */

	t_bool    flagro;                                /* write protect status */
	t_bool    flagexist;                    /* flag of floppy disk existance */

	t_vaddrcc base;                                /* pointer to disk in ram */
	t_vaddrcc curr;                               /* pointer to current byte */
	t_nubit16 count;                             /* number of transfer bytes */
} t_hdd;

extern t_hdd vhdd;

#define vhddSetPointer (vhdd.curr = vhdd.base +                                \
                       ((vhdd.cyl * vhdd.nhead + vhdd.head) * vhdd.nsector +  \
                        (vhdd.sector - 0x01)) * vhdd.nbyte)
#define vhddGetImageSize (vhdd.nbyte * vhdd.nsector * vhdd.nhead * vhdd.ncyl)

void vhddTransRead();
void vhddTransWrite();
void vhddFormatTrack(t_nubit8 fillbyte);

void vhddAlloc();
void vhddRefresh();
void vhddInit();
void vhddReset();
void vhddFinal();

#endif
