/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_W32CDISP_H
#define VM_PLATFORM_W32CDISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/platform/presentation_mailbox_interface.h"

#include "vm/platform/win32/win32_types.h"


typedef struct w32cdisp_context w32cdisp_context;

w32cdisp_context *w32cdisp_context_create(C_VOID);
C_VOID w32cdisp_context_destroy(w32cdisp_context *context);
type_unsigned_64 w32cdisp_context_generation(const w32cdisp_context *context);

C_VOID w32cdispSetScreen(w32cdisp_context *context, WIN32_HANDLE output,
                       const core_platform_presentation_mailbox *mailbox);
C_VOID w32cdispPaint(w32cdisp_context *context, WIN32_HANDLE output,
                   const core_platform_presentation_mailbox *mailbox,
                   WIN32_BOOL flagForce);

C_VOID w32cdispInit(w32cdisp_context *context, WIN32_HANDLE output,
                  const core_platform_presentation_mailbox *mailbox);
C_VOID w32cdispFinal(w32cdisp_context *context, WIN32_HANDLE output);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
