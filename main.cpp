#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <Windows.h>

#define wWidth 1280
#define wHeight 720

typedef uint8_t u8;
typedef uint32_t u32;

static bool isWindowOpen = true;

LRESULT WindowEventCallback(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
) {
    LRESULT result = {};

    switch (message) {
        case WM_CLOSE:
        case WM_QUIT:
        {
            isWindowOpen = false;
        } 
        break;
        default:
        {
            result = DefWindowProcA(hWnd, message, wParam, lParam);
        } 
        break;
    }
    return result;
}

u32 GetRGBA(
    u8 Red,
    u8 Green,
    u8 Blue,
    u8 Alpha
) {
    return ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Instantiating window class
    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = WindowEventCallback;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "Rasterizer";

    assert(RegisterClassA(&windowClass));

    // Creating window object
    HWND window = CreateWindowExA(
        NULL,
        windowClass.lpszClassName,
        windowClass.lpszClassName,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wWidth,
        wHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    assert(window);

    RECT clientRect = {};
    assert(GetClientRect(window, &clientRect));

    const u32 framebufferHeight = clientRect.bottom - clientRect.top;
    const u32 framebufferWidth = clientRect.right - clientRect.left;

    // Bitmap
    size_t bitMapSize = framebufferHeight * framebufferWidth;
    u32* bitMap = (u32*)malloc(sizeof(u32) * bitMapSize);
    assert(bitMap);

    for (size_t y = 0; y < framebufferHeight; ++y)
    {
        for (size_t x = 0; x < framebufferWidth; ++x)
        {
            size_t pixelIdx = framebufferWidth * y + x;
            bitMap[pixelIdx] = GetRGBA(
                (u8)x,
                0,
                (u8)y,
                255
            );
        }
    }

    // GDI
    HDC deviceContext = GetDC(window);

    // Rendering bitmap
    BITMAPINFO bitMapInfo = {};

    bitMapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
    bitMapInfo.bmiHeader.biWidth = framebufferWidth;
    bitMapInfo.bmiHeader.biHeight = framebufferHeight;
    bitMapInfo.bmiHeader.biPlanes = 1;
    bitMapInfo.bmiHeader.biBitCount = sizeof(u32) * 8;
    bitMapInfo.bmiHeader.biCompression = BI_RGB;

    assert(StretchDIBits(
        deviceContext,
        0,
        0,
        framebufferWidth,
        framebufferHeight,
        0,
        0,
        framebufferWidth,
        framebufferHeight,
        bitMap,
        &bitMapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    ));
    
    // Main loop
    while (isWindowOpen) {
        MSG message = {};
        while (PeekMessageA(&message, window, NULL, NULL, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                isWindowOpen = false;
            } else {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        }
    }

    free(bitMap);

    return 0;
}
