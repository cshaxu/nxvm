/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_W32ADISP_H
#define VM_PLATFORM_W32ADISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/platform/presentation_mailbox_interface.h"

#include "vm/platform/win32/win32_types.h"


typedef struct w32adisp_context w32adisp_context;

w32adisp_context *w32adisp_context_create(C_VOID);
C_VOID w32adisp_context_destroy(w32adisp_context *context);
type_unsigned_64 w32adisp_context_generation(const w32adisp_context *context);

C_VOID w32adispSetScreen(w32adisp_context *context, WIN32_HWND window,
                        const core_platform_presentation_mailbox *mailbox);
C_VOID w32adispPaint(w32adisp_context *context, WIN32_HWND window,
                   const core_platform_presentation_mailbox *mailbox,
                   WIN32_BOOL flagForce);

C_VOID w32adispInit(w32adisp_context *context, WIN32_HWND window,
                  const core_platform_presentation_mailbox *mailbox,
                  const C_CHAR *font_path);
C_VOID w32adispFinal(w32adisp_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
