#include <iostream>

#include <windows.h>

#include "app/gl_render.h"

#include "gl_windows/windows_gl_utilities.h"

#include "utilities/init_debug_console.h"

HWND g_main_window = nullptr;
WCHAR *g_main_wndclass_name = L"main class";

bool create_my_main_window(HINSTANCE instance);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    init_debug_env();

    if (0 != init_windows_gl(hInstance))
        return 0;

    hPrevInstance;
    lpCmdLine;
    nCmdShow;

    // TODO: Place code here.
    MSG msg;

    if (!create_my_main_window(hInstance))
        return 1;

    init_gl_render_data(g_main_window);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            display();

            SwapBuffers(GetDC(g_main_window));
        }
    }

    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool create_my_main_window(HINSTANCE instance)
{

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = nullptr;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_G);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = g_main_wndclass_name;
    wcex.hIconSm = nullptr;

    RegisterClassEx(&wcex);


    HWND wnd;

    wnd = CreateWindow(g_main_wndclass_name, L"main window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 400, NULL, NULL, instance, NULL);

    if (!wnd)
    {
        return false;
    }

    g_main_window = wnd;
    ShowWindow(wnd, SW_SHOW);
    UpdateWindow(wnd);

    return true;

}