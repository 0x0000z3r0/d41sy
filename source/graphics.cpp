#include "graphics.h"
#include "config.h"

#include <d2d1_1.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <wincodec.h>

static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_d3dContext = nullptr;
static IDXGIDevice* g_dxgiDevice = nullptr;
static IDXGIAdapter* g_adapter = nullptr;
static IDXGIFactory2* g_factory = nullptr;
static IDXGISwapChain1* g_swapChain = nullptr;
static IDCompositionDevice* g_dcomp = nullptr;
static IDCompositionTarget* g_dcompTarget = nullptr;
static IDCompositionVisual* g_visual = nullptr;
static ID2D1Factory1* g_d2dFactory = nullptr;
static ID2D1Device* g_d2dDevice = nullptr;
static ID2D1DeviceContext* g_dc = nullptr;
static IDXGISurface* g_surface = nullptr;
static ID2D1Bitmap1* g_target = nullptr;
static IDWriteFactory* g_writeFactory = nullptr;
static IDWriteTextFormat* g_textFormat = nullptr;
static ID2D1SolidColorBrush* g_brush = nullptr;
static IWICImagingFactory* g_wic = nullptr;
static FLOAT g_width = 0.0f;
static FLOAT g_height = 0.0f;

template <typename T>
static void ReleaseInterface(T** iface)
{
    if (iface != nullptr && *iface != nullptr)
    {
        (*iface)->Release();
        *iface = nullptr;
    }
}

static BOOL GraphicsBindTarget(void)
{
    HRESULT hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&g_surface));
    if (FAILED(hr))
    {
        return FALSE;
    }

    D2D1_BITMAP_PROPERTIES1 properties;
    ZeroMemory(&properties, sizeof(properties));
    properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    properties.dpiX = 96.0f;
    properties.dpiY = 96.0f;

    hr = g_dc->CreateBitmapFromDxgiSurface(g_surface, &properties, &g_target);
    if (FAILED(hr))
    {
        return FALSE;
    }

    g_dc->SetTarget(g_target);
    return TRUE;
}

static void GraphicsReleaseTarget(void)
{
    if (g_dc != nullptr)
    {
        g_dc->SetTarget(nullptr);
    }

    ReleaseInterface(&g_target);
    ReleaseInterface(&g_surface);
}

static BOOL GraphicsCreateDevice(void)
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &g_device,
        nullptr,
        &g_d3dContext);
    if (FAILED(hr))
    {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &g_device,
            nullptr,
            &g_d3dContext);
    }

    return SUCCEEDED(hr);
}

BOOL GraphicsStartup(HWND window, UINT width, UINT height)
{
    g_width = (FLOAT)width;
    g_height = (FLOAT)height;

    if (!GraphicsCreateDevice())
    {
        GraphicsShutdown();
        return FALSE;
    }

    HRESULT hr = g_device->QueryInterface(IID_PPV_ARGS(&g_dxgiDevice));
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_dxgiDevice->GetAdapter(&g_adapter);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_adapter->GetParent(IID_PPV_ARGS(&g_factory));
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    DXGI_SWAP_CHAIN_DESC1 description;
    ZeroMemory(&description, sizeof(description));
    description.Width = width;
    description.Height = height;
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount = 2;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    description.Scaling = DXGI_SCALING_STRETCH;

    hr = g_factory->CreateSwapChainForComposition(g_device, &description, nullptr, &g_swapChain);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = DCompositionCreateDevice(g_dxgiDevice, IID_PPV_ARGS(&g_dcomp));
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_dcomp->CreateTargetForHwnd(window, TRUE, &g_dcompTarget);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_dcomp->CreateVisual(&g_visual);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_visual->SetContent(g_swapChain);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_dcompTarget->SetRoot(g_visual);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_dcomp->Commit();
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    D2D1_FACTORY_OPTIONS options;
    ZeroMemory(&options, sizeof(options));
    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(ID2D1Factory1),
        &options,
        reinterpret_cast<void**>(&g_d2dFactory));
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_d2dFactory->CreateDevice(g_dxgiDevice, &g_d2dDevice);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &g_dc);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    if (!GraphicsBindTarget())
    {
        GraphicsShutdown();
        return FALSE;
    }

    g_dc->SetDpi(96.0f, 96.0f);
    g_dc->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&g_writeFactory));
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = g_writeFactory->CreateTextFormat(
        L"Consolas",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        CONFIG_LOG_FONT_SIZE,
        L"en-us",
        &g_textFormat);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    D2D1_COLOR_F color;
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;
    hr = g_dc->CreateSolidColorBrush(&color, nullptr, &g_brush);
    if (FAILED(hr))
    {
        GraphicsShutdown();
        return FALSE;
    }

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_wic));
    if (FAILED(hr))
    {
        g_wic = nullptr;
    }

    return TRUE;
}

void GraphicsShutdown(void)
{
    ReleaseInterface(&g_brush);
    ReleaseInterface(&g_textFormat);
    ReleaseInterface(&g_writeFactory);
    ReleaseInterface(&g_wic);
    GraphicsReleaseTarget();
    ReleaseInterface(&g_dc);
    ReleaseInterface(&g_d2dDevice);
    ReleaseInterface(&g_d2dFactory);
    ReleaseInterface(&g_visual);
    ReleaseInterface(&g_dcompTarget);
    ReleaseInterface(&g_dcomp);
    ReleaseInterface(&g_swapChain);
    ReleaseInterface(&g_factory);
    ReleaseInterface(&g_adapter);
    ReleaseInterface(&g_dxgiDevice);
    ReleaseInterface(&g_d3dContext);
    ReleaseInterface(&g_device);
    g_width = 0.0f;
    g_height = 0.0f;
}

BOOL GraphicsResize(UINT width, UINT height)
{
    if (g_dc == nullptr || g_swapChain == nullptr || width == 0 || height == 0)
    {
        return FALSE;
    }

    GraphicsReleaseTarget();
    HRESULT hr = g_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (!GraphicsBindTarget())
    {
        return FALSE;
    }

    g_width = (FLOAT)width;
    g_height = (FLOAT)height;
    return TRUE;
}

BOOL GraphicsBegin(void)
{
    if (g_dc == nullptr)
    {
        return FALSE;
    }

    g_dc->BeginDraw();
    return TRUE;
}

void GraphicsClear(void)
{
    if (g_dc == nullptr)
    {
        return;
    }

    D2D1_COLOR_F color;
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = CONFIG_BACK_ALPHA;
    g_dc->Clear(color);
}

void GraphicsDrawText(FLOAT x, FLOAT y, FLOAT layoutWidth, PCWSTR text, FLOAT red, FLOAT green, FLOAT blue, FLOAT alpha)
{
    if (g_dc == nullptr || g_brush == nullptr || g_textFormat == nullptr || text == nullptr)
    {
        return;
    }

    D2D1_COLOR_F color;
    color.r = red;
    color.g = green;
    color.b = blue;
    color.a = alpha;
    g_brush->SetColor(color);

    D2D1_RECT_F layout;
    layout.left = x;
    layout.top = y;
    layout.right = x + layoutWidth;
    layout.bottom = y + CONFIG_LOG_FONT_SIZE + CONFIG_LOG_LINE_GAP;

    g_dc->DrawText(
        text,
        (UINT32)lstrlenW(text),
        g_textFormat,
        &layout,
        g_brush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL);
}

void GraphicsDrawImage(GRAPHICS_IMAGE* image, FLOAT x, FLOAT y, FLOAT width, FLOAT height, BOOL flipHorizontal)
{
    if (g_dc == nullptr || image == nullptr || image->Bitmap == nullptr)
    {
        return;
    }

    D2D1_MATRIX_3X2_F previous;
    g_dc->GetTransform(&previous);
    if (flipHorizontal)
    {
        D2D1_MATRIX_3X2_F mirror;
        mirror.m11 = -1.0f;
        mirror.m12 = 0.0f;
        mirror.m21 = 0.0f;
        mirror.m22 = 1.0f;
        mirror.dx = 2.0f * (x + (width * 0.5f));
        mirror.dy = 0.0f;
        g_dc->SetTransform(&mirror);
    }

    D2D1_RECT_F destination;
    destination.left = x;
    destination.top = y;
    destination.right = x + width;
    destination.bottom = y + height;
    g_dc->DrawBitmap(image->Bitmap, &destination, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nullptr);
    g_dc->SetTransform(&previous);
}

BOOL GraphicsEnd(void)
{
    if (g_dc == nullptr || g_swapChain == nullptr)
    {
        return FALSE;
    }

    HRESULT hr = g_dc->EndDraw();
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = g_swapChain->Present(1, 0);
    return SUCCEEDED(hr);
}

FLOAT GraphicsWidth(void)
{
    return g_width;
}

FLOAT GraphicsHeight(void)
{
    return g_height;
}

BOOL GraphicsLoadImage(const void* bytes, DWORD size, GRAPHICS_IMAGE** outImage)
{
    if (outImage != nullptr)
    {
        *outImage = nullptr;
    }

    if (g_wic == nullptr || g_dc == nullptr || bytes == nullptr || size == 0 || outImage == nullptr)
    {
        return FALSE;
    }

    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID2D1Bitmap* bitmap = nullptr;
    GRAPHICS_IMAGE* image = nullptr;

    HRESULT hr = g_wic->CreateStream(&stream);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(static_cast<const BYTE*>(bytes)), size);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = g_wic->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = g_wic->CreateFormatConverter(&converter);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
    {
        goto Done;
    }

    hr = g_dc->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap);
    if (FAILED(hr))
    {
        goto Done;
    }

    D2D1_SIZE_U pixelSize = bitmap->GetPixelSize();
    UINT width = pixelSize.width;
    UINT height = pixelSize.height;
    if (width == 0 || height == 0)
    {
        hr = E_FAIL;
        goto Done;
    }

    image = static_cast<GRAPHICS_IMAGE*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GRAPHICS_IMAGE)));
    if (image == nullptr)
    {
        hr = E_OUTOFMEMORY;
        goto Done;
    }

    image->Bitmap = bitmap;
    image->Width = (FLOAT)width;
    image->Height = (FLOAT)height;
    bitmap = nullptr;
    *outImage = image;
    image = nullptr;
    hr = S_OK;

Done:
    ReleaseInterface(&bitmap);
    ReleaseInterface(&converter);
    ReleaseInterface(&frame);
    ReleaseInterface(&decoder);
    ReleaseInterface(&stream);
    if (image != nullptr)
    {
        HeapFree(GetProcessHeap(), 0, image);
    }

    return SUCCEEDED(hr);
}

void GraphicsReleaseImage(GRAPHICS_IMAGE* image)
{
    if (image == nullptr)
    {
        return;
    }

    ReleaseInterface(&image->Bitmap);
    HeapFree(GetProcessHeap(), 0, image);
}

FLOAT GraphicsImageWidth(GRAPHICS_IMAGE* image)
{
    if (image == nullptr)
    {
        return 0.0f;
    }

    return image->Width;
}

FLOAT GraphicsImageHeight(GRAPHICS_IMAGE* image)
{
    if (image == nullptr)
    {
        return 0.0f;
    }

    return image->Height;
}
