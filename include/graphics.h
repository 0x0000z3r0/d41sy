#pragma once

#include <windows.h>
#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GRAPHICS_IMAGE
{
    ID2D1Bitmap* Bitmap;
    FLOAT Width;
    FLOAT Height;
} GRAPHICS_IMAGE;

BOOL GraphicsStartup(HWND window, UINT width, UINT height);
void GraphicsShutdown(void);
BOOL GraphicsResize(UINT width, UINT height);

BOOL GraphicsBegin(void);
void GraphicsClear(void);
void GraphicsDrawText(FLOAT x, FLOAT y, FLOAT layoutWidth, PCWSTR text, FLOAT red, FLOAT green, FLOAT blue, FLOAT alpha);
void GraphicsDrawImage(GRAPHICS_IMAGE* image, FLOAT x, FLOAT y, FLOAT width, FLOAT height, BOOL flipHorizontal);
BOOL GraphicsEnd(void);

FLOAT GraphicsWidth(void);
FLOAT GraphicsHeight(void);

BOOL GraphicsLoadImage(const void* bytes, DWORD size, GRAPHICS_IMAGE** outImage);
void GraphicsReleaseImage(GRAPHICS_IMAGE* image);
FLOAT GraphicsImageWidth(GRAPHICS_IMAGE* image);
FLOAT GraphicsImageHeight(GRAPHICS_IMAGE* image);

#ifdef __cplusplus
}
#endif
