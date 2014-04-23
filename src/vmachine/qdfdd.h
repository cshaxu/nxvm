#ifndef NXVM_QDFDD_H
#define NXVM_QDFDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

typedef struct {
	t_bool    flagro;                                /* write protect status */
	t_bool    flagexist;                    /* flag of floppy disk existance */

	t_vaddrcc base;                                /* pointer to disk in ram */
} t_fdd;

extern t_fdd qdfdd;

t_vaddrcc qdfddGetAddress(t_nubit8 cyl,
                          t_nubit8 head,
                          t_nubit8 sector);
void qdfddResetDrive();
void qdfddGetDiskStatus();
void qdfddReadSector();
void qdfddWriteSector();
void qdfddGetParameter();
void qdfddGetDriveType();

void qdfddInit();
void qdfddFinal();

#ifdef __cplusplus
}/*cends*/
#endif

#endif