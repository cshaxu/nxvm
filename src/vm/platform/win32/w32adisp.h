/* Copyright 2012-2014 Neko. */

#ifndef NXVM_W32ADISP_H
#define NXVM_W32ADISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_presentation_mailbox vm_platform_presentation_mailbox;

VOID w32adispSetScreen(HWND window,
                        const vm_platform_presentation_mailbox *mailbox);
VOID w32adispPaint(HWND window,
                    const vm_platform_presentation_mailbox *mailbox,
                   BOOL flagForce);

VOID w32adispInit(HWND window,
                  const vm_platform_presentation_mailbox *mailbox);
VOID w32adispFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
