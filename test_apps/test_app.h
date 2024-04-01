#pragma once

#include <Windows.h>
#include <cmath>
#include <glad/glad.h>
#include <glad/glad_wgl.h>

using RenderFunction = void (*)(float, int, int);

ULONGLONG startTime;
RenderFunction render;

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        startTime = GetTickCount64();
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT: {
        // Prepare time
        ULONGLONG elapsedMiliseconds = GetTickCount64() - startTime;
        float t = (elapsedMiliseconds % 3000) / 3000.f; // between 0 and 1
        t = 2 * fabs(t - 0.5);                          // make it go back and forth between 0 and 1

        // Prepare client size
        RECT clientRect = {};
        GetClientRect(hwnd, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;

        // Render
        render(t, clientWidth, clientHeight);

        // Swap
        HDC hdc = GetDC(hwnd);
        wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
        ReleaseDC(hwnd, hdc);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

struct TestApp {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    const char *windowClassName;
};

TestApp initializeTestApp(RenderFunction renderFunction) {
    ::render = renderFunction;

    const int windowWidth = 800;
    const int windowHeight = 600;

    // Create window
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC,
        windowProc,
        0L,
        0L,
        GetModuleHandle(NULL),
        NULL,
        NULL,
        NULL,
        NULL,
        "OpenGL Window",
        NULL};
    RegisterClassExA(&wc);
    HWND hwnd = CreateWindowA(
        wc.lpszClassName,
        "OpenGL Window",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        wc.hInstance,
        NULL);

    // Initialize OpenGL
    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0};
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);
    HGLRC hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);
    gladLoadWGL(hdc);
    gladLoadGL();

    return TestApp{
        hwnd,
        hdc,
        hrc,
        wc.lpszClassName,
    };
}

void runTestApp(TestApp &app) {
    ShowWindow(app.hwnd, SW_SHOW);
    UpdateWindow(app.hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void cleanupTestApp(TestApp &app) {
    wglDeleteContext(app.hglrc);
    ReleaseDC(app.hwnd, app.hdc);
    DestroyWindow(app.hwnd);
    UnregisterClassA(app.windowClassName, GetModuleHandle(NULL));
}
