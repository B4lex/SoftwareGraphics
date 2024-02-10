#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <Windows.h>

#define wWidth 1280
#define wHeight 720

static bool isWindowOpen = true;

LRESULT WindowCallback(
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


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Instantiating window class
    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = WindowCallback;
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

    const uint32_t framebufferHeight = clientRect.bottom - clientRect.top;
    const uint32_t framebufferWidth = clientRect.right - clientRect.left;

    // Bitmap
    size_t bitMapSize = framebufferHeight * framebufferWidth;
    uint32_t* bitMap = (uint32_t*)malloc(sizeof(uint32_t) * bitMapSize);
    assert(bitMap);

    for (size_t y = 0; y < framebufferHeight; ++y)
    {
        for (size_t x = 0; x < framebufferWidth; ++x)
        {
            size_t pixelIdx = framebufferWidth * y + x;
            bitMap[pixelIdx] = 0xFFEBA228;
        }
    }

    // GDI
    HDC deviceContext = GetDC(window);
    
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

        BITMAPINFO bitMapInfo = {};

        bitMapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
        bitMapInfo.bmiHeader.biWidth = framebufferWidth;
        bitMapInfo.bmiHeader.biHeight = framebufferHeight;
        bitMapInfo.bmiHeader.biPlanes = 1;
        bitMapInfo.bmiHeader.biBitCount = sizeof(uint32_t) * 8;
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
    }

    free(bitMap);

    return 0;
}
