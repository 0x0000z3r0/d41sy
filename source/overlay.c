#include "overlay.h"

#include "config.h"
#include "graphics.h"
#include "report.h"
#include "resource.h"

static GRAPHICS_IMAGE* g_logo = NULL;
static BOOL g_pinned = TRUE;
static LONG g_skipNewest = 0;
static LONG g_seenWrite = 0;

BOOL OverlayStartup(void)
{
    g_logo = NULL;

    HRSRC resource = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_LOGO), RT_RCDATA);
    if (resource == NULL)
    {
        ReportFail(L"logo.png not found");
        return TRUE;
    }

    HGLOBAL loaded = LoadResource(NULL, resource);
    DWORD size = SizeofResource(NULL, resource);
    void* bytes = LockResource(loaded);
    if (loaded == NULL || bytes == NULL || size == 0 || !GraphicsLoadImage(bytes, size, &g_logo))
    {
        ReportFail(L"logo.png not found");
        g_logo = NULL;
        return TRUE;
    }

    ReportSuccess(L"logo loaded");

    return TRUE;
}

void OverlayShutdown(void)
{
    GraphicsReleaseImage(g_logo);
    g_logo = NULL;
}

static void OverlayColorForLevel(LONG level, FLOAT* red, FLOAT* green, FLOAT* blue)
{
    if (level == REPORT_LEVEL_SUCCESS)
    {
        *red = CONFIG_COLOR_SUCCESS_R;
        *green = CONFIG_COLOR_SUCCESS_G;
        *blue = CONFIG_COLOR_SUCCESS_B;
        return;
    }

    if (level == REPORT_LEVEL_FAIL)
    {
        *red = CONFIG_COLOR_FAIL_R;
        *green = CONFIG_COLOR_FAIL_G;
        *blue = CONFIG_COLOR_FAIL_B;
        return;
    }

    *red = CONFIG_COLOR_INFO_R;
    *green = CONFIG_COLOR_INFO_G;
    *blue = CONFIG_COLOR_INFO_B;
}

static LONG OverlayMaxVisibleLines(void)
{
    FLOAT lineHeight = CONFIG_LOG_FONT_SIZE + CONFIG_LOG_LINE_GAP;
    LONG maxLines = (LONG)((GraphicsHeight() - (CONFIG_LOG_MARGIN * 2.0f)) / lineHeight);
    if (maxLines < 1)
    {
        maxLines = 1;
    }
    if (maxLines > CONFIG_REPORT_CAPACITY)
    {
        maxLines = CONFIG_REPORT_CAPACITY;
    }

    return maxLines;
}

static void OverlayCatchUp(void)
{
    LONG total = ReportGetWriteCount();
    LONG arrived = total - g_seenWrite;
    g_seenWrite = total;
    if (arrived > 0 && !g_pinned)
    {
        g_skipNewest += arrived;
    }
}

static void OverlayClampSkip(void)
{
    LONG available = g_seenWrite;
    if (available > CONFIG_REPORT_CAPACITY)
    {
        available = CONFIG_REPORT_CAPACITY;
    }

    LONG visible = OverlayMaxVisibleLines();
    if (visible > available)
    {
        visible = available;
    }

    LONG maxSkip = available - visible;
    if (g_pinned)
    {
        g_skipNewest = 0;
    }
    else if (g_skipNewest > maxSkip)
    {
        g_skipNewest = maxSkip;
    }
    if (g_skipNewest <= 0)
    {
        g_skipNewest = 0;
        g_pinned = TRUE;
    }
}

void OverlayScrollLines(LONG deltaLines)
{
    OverlayCatchUp();
    if (g_pinned)
    {
        g_skipNewest = 0;
    }

    g_skipNewest -= deltaLines;
    if (g_skipNewest <= 0)
    {
        g_skipNewest = 0;
        g_pinned = TRUE;
    }
    else
    {
        g_pinned = FALSE;
    }

    OverlayClampSkip();
}

static void OverlayDrawLogs(void)
{
    OverlayCatchUp();

    REPORT_LINE lines[CONFIG_REPORT_CAPACITY];
    LONG count = 0;
    if (!ReportSnapshot(lines, CONFIG_REPORT_CAPACITY, &count))
    {
        return;
    }

    LONG visible = OverlayMaxVisibleLines();
    if (count < visible)
    {
        visible = count;
    }

    LONG maxSkip = count - visible;
    if (g_pinned)
    {
        g_skipNewest = 0;
    }
    else if (g_skipNewest > maxSkip)
    {
        g_skipNewest = maxSkip;
    }
    if (g_skipNewest <= 0)
    {
        g_skipNewest = 0;
        g_pinned = TRUE;
    }

    LONG start = count - visible - g_skipNewest;
    FLOAT y = CONFIG_LOG_MARGIN;
    FLOAT lineHeight = CONFIG_LOG_FONT_SIZE + CONFIG_LOG_LINE_GAP;
    FLOAT layoutWidth = GraphicsWidth() - (CONFIG_LOG_MARGIN * 2.0f);
    if (layoutWidth < 1.0f)
    {
        layoutWidth = 1.0f;
    }

    for (LONG i = 0; i < visible; i++)
    {
        FLOAT red;
        FLOAT green;
        FLOAT blue;
        OverlayColorForLevel(lines[start + i].Level, &red, &green, &blue);
        GraphicsDrawText(CONFIG_LOG_MARGIN, y, layoutWidth, lines[start + i].Text, red, green, blue, 1.0f);
        y += lineHeight;
    }
}

static void OverlayDrawLogo(void)
{
    if (g_logo == NULL)
    {
        return;
    }

    FLOAT sourceWidth = GraphicsImageWidth(g_logo);
    FLOAT sourceHeight = GraphicsImageHeight(g_logo);
    if (sourceWidth <= 0.0f || sourceHeight <= 0.0f)
    {
        return;
    }

    FLOAT width = CONFIG_LOGO_WIDTH;
    FLOAT height = width * (sourceHeight / sourceWidth);
    FLOAT x = GraphicsWidth() - width - CONFIG_LOGO_MARGIN;
    FLOAT y = GraphicsHeight() - height - CONFIG_LOGO_MARGIN;
    GraphicsDrawImage(g_logo, x, y, width, height, TRUE);
}

void OverlayRender(void)
{
    if (!GraphicsBegin())
    {
        return;
    }

    GraphicsClear();
    OverlayDrawLogs();
    OverlayDrawLogo();
    GraphicsEnd();
}
