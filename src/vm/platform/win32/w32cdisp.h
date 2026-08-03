/* Copyright 2012-2014 Neko. */

#ifndef NXVM_W32CDISP_H
#define NXVM_W32CDISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include <windows.h>

#include <stdint.h>

typedef struct vm_platform_presentation_mailbox vm_platform_presentation_mailbox;
typedef struct w32cdisp_context w32cdisp_context;

w32cdisp_context *w32cdisp_context_create(C_VOID);
VOID w32cdisp_context_destroy(w32cdisp_context *context);
uint64_t w32cdisp_context_generation(const w32cdisp_context *context);

VOID w32cdispSetScreen(w32cdisp_context *context, HANDLE output,
                       const vm_platform_presentation_mailbox *mailbox);
VOID w32cdispPaint(w32cdisp_context *context, HANDLE output,
                   const vm_platform_presentation_mailbox *mailbox,
                   BOOL flagForce);

VOID w32cdispInit(w32cdisp_context *context, HANDLE output,
                  const vm_platform_presentation_mailbox *mailbox);
VOID w32cdispFinal(w32cdisp_context *context, HANDLE output);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
