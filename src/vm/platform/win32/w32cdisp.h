/* Copyright 2012-2014 Neko. */

#ifndef NXVM_W32CDISP_H
#define NXVM_W32CDISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_presentation_mailbox vm_platform_presentation_mailbox;

VOID w32cdispSetScreen(HANDLE output,
                       const vm_platform_presentation_mailbox *mailbox);
VOID w32cdispPaint(HANDLE output,
                   const vm_platform_presentation_mailbox *mailbox,
                   BOOL flagForce);

VOID w32cdispInit(HANDLE output,
                  const vm_platform_presentation_mailbox *mailbox);
VOID w32cdispFinal(HANDLE output);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
