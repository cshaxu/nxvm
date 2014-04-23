#ifndef NXVM_QDVGA_H
#define NXVM_QDVGA_H

void qdvgaSetDisplayMode();
void qdvgaSetCursorType();
void qdvgaSetCursorPos();
void qdvgaGetCursorPos();
void qdvgaSetDisplayPage();
void qdvgaScrollUp();
void qdvgaScrollDown();
void qdvgaGetCharProp();
void qdvgaDisplayCharProp(t_nubit16 count);
void qdvgaDisplayChar();
void qdvgaSetPalette();
void qdvgaDisplayPixel();
void qdvgaGetPixel();
void qdvgaGetAdapterStatus();
void qdvgaGenerateChar();
void qdvgaGetAdapterInfo();
void qdvgaDisplayStr();

#endif