/* Copyright 2012-2014 Neko. */

#ifndef NXVM_AASM32_H
#define NXVM_AASM32_H

#ifdef __cplusplus
extern "C" {
#endif

/* assemble single instruction */
unsigned char aasm32(const char *stmt, unsigned char *rcode, unsigned char flag32);
/* assemble a paragraph of instructions */
unsigned int aasm32x(const char *stmt, unsigned char *rcode, unsigned char flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
