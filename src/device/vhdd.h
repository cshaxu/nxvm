/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VHDD_H
#define NXVM_VHDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_HDD "Unknown Hard Disk Drive"

typedef struct {
    t_nubit16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    t_nubit16 head;    /* vfdc.H; head id (0 or 1) */
    t_nubit16 sector;  /* vfdc.R; sector id (1 to 18) */

    t_nubit16 ncyl;    /* number of cylinders, should be 20 here */
    t_nubit16 nhead;   /* number of heads, should be 16 here */
    t_nubit16 nsector; /* vfdc.EOT; should be 63 here */
    t_nubit16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */

    t_nubit8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */

    t_bool flagReadOnly;  /* write protect status */
    t_bool flagDiskExist; /* flag of floppy disk existance */

    t_vaddrcc pImgBase;   /* pointer to disk in ram */
    t_vaddrcc pCurrByte;  /* pointer to current byte */
    t_nubit16 transCount; /* number of transfer bytes */
} t_hdd;

extern t_hdd vhdd;

#define VHDD_BYTE_PER_MB (1 << 20)

#define vhddSetPointer (vhdd.pCurrByte = vhdd.pImgBase + \
    ((vhdd.cyl * vhdd.nhead + vhdd.head) * vhdd.nsector +  \
    (vhdd.sector - 1)) * vhdd.nbyte)
#define vhddGetImageSize (vhdd.nbyte * vhdd.nsector * vhdd.nhead * vhdd.ncyl)

void vhddTransRead();
void vhddTransWrite();
void vhddFormatTrack(t_nubit8 fillByte);

void vhddRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
