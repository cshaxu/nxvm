/* Copyright 2012-2014 Neko. */

#ifndef NXVM_W32ADISP_H
#define NXVM_W32ADISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdint.h>

typedef struct vm_platform_presentation_mailbox vm_platform_presentation_mailbox;
typedef struct w32adisp_context w32adisp_context;

w32adisp_context *w32adisp_context_create(void);
VOID w32adisp_context_destroy(w32adisp_context *context);
uint64_t w32adisp_context_generation(const w32adisp_context *context);

VOID w32adispSetScreen(w32adisp_context *context, HWND window,
                        const vm_platform_presentation_mailbox *mailbox);
VOID w32adispPaint(w32adisp_context *context, HWND window,
                   const vm_platform_presentation_mailbox *mailbox,
                   BOOL flagForce);

VOID w32adispInit(w32adisp_context *context, HWND window,
                  const vm_platform_presentation_mailbox *mailbox);
VOID w32adispFinal(w32adisp_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
