/* This file is a part of NXVM project. */

#ifndef NXVM_QDDISK_H
#define NXVM_QDDISK_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define HARD_FDD_INT_0E "  \
push ax                  \n\
push dx                  \n\
mov dx, 03f4             \n\
in  al, dx               \n\
and al, c0               \n\
cmp al, 80 ; write ready \n\
jnz $(label_int_0e_ret)  \n\
\
mov dx, 03f5             \n\
mov al, 08               \n\
out dx, al ; sense int   \n\
in  al, dx ; st0 0x20    \n\
in  al, dx ; cyl 0x00    \n\
\
$(label_int_0e_ret):     \n\
mov al, 20               \n\
out 20, al ; eoi cmd     \n\
pop dx                   \n\
pop ax                   \n"

void INT_13();
void qddiskReset();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
