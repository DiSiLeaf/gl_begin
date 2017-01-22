#include <iostream>
#include <vector>

#include <windows.h>

#include <atlbase.h>
#include <atlwin.h>

#include "app/gl_render.h"

#include "gl_windows/windows_gl_utilities.h"

#include "utilities/init_debug_console.h"

#include "utilities/funtion_annotation_macro.h"

//HWND g_main_window = nullptr;
WCHAR *g_main_wndclass_name = L"main class";

class GlWindow;
GlWindow *g_window = nullptr;


class MsgIdleObserver {
public:
    static void AddObserver(VAR_IN MsgIdleObserver *ob)
    {
        if (ob)
            idle_observers.push_back(ob);
    }
    static void RemoveObserver(VAR_IN MsgIdleObserver *ob)
    {
        if (ob) {
            for (std::vector<MsgIdleObserver*>::const_iterator it
                    = idle_observers.begin()
                    ; idle_observers.end() != it; ++it) 
                if (ob == *it) {
                    idle_observers.erase(it);
                    return;
                }
        }
    }

    static void Dispatch(void)
    {
        for (auto it : idle_observers) {
            if (it)
                it->OnIdle();
        }
    }
private:
    static std::vector<MsgIdleObserver*> idle_observers;

protected:
    MsgIdleObserver(void) : valid_(true) {
    }
    void InvalidateObserver(void)
    {
        valid_ = false;

        for (std::vector<MsgIdleObserver*>::const_iterator it 
                = idle_observers.begin()
                ; idle_observers.end() != it; ++it)
            if (this == *it) {
                idle_observers.erase(it);
                return;
            }
    }
private:
    bool valid_;

protected:
    virtual void OnIdle(void) = 0;
};

std::vector<MsgIdleObserver*> MsgIdleObserver::idle_observers;

class GlWindow 
        : public CWindowImpl< GlWindow, CWindow, 
            CWinTraits<WS_OVERLAPPEDWINDOW, 0> >,
        public MsgIdleObserver {
public:
    DECLARE_WND_CLASS_EX(L"GlWindow", 
            CS_HREDRAW | CS_VREDRAW | CS_OWNDC, COLOR_WINDOW)

    static LPCWSTR GetWndCaption(void)
    {
        return L"main window";
    }
    BEGIN_MSG_MAP(GlWindow)
    MESSAGE_HANDLER(WM_SIZE, ResizeHandle)
    MESSAGE_HANDLER(WM_DESTROY, DestoryHandle)
    END_MSG_MAP()

protected:
    void OnIdle(void)
    {
        display();

        SwapBuffers(GetDC());
    }


private:
    LRESULT ResizeHandle(VAR_IN UINT uMsg, VAR_IN WPARAM wParam, VAR_IN LPARAM lParam, VAR_IN BOOL &handled)
    {
        uMsg;

        if (SIZE_RESTORED == wParam) {
            WORD w, h;
            w = LOWORD(lParam);
            h = HIWORD(lParam);

            glViewport(0, 0, (GLsizei)w, (GLsizei)h);
        }

        handled = TRUE;
        return 0;
    }

    LRESULT DestoryHandle(VAR_IN UINT, VAR_IN WPARAM, LPARAM, VAR_OUT BOOL &handled)
    {
        InvalidateObserver();

        PostQuitMessage(0);

        handled = TRUE;
        return 0;
    }

};



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

    //init_gl_render_data(g_main_window);

    init_gl_render_data(*g_window);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            //display();

            MsgIdleObserver::Dispatch();

            //SwapBuffers(GetDC(g_main_window));
        }
    }

    clean_data();

    //delete g_window;
    //g_window = nullptr;

    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(VAR_IN HWND hWnd, VAR_IN UINT message, VAR_IN WPARAM wParam, VAR_IN LPARAM lParam)
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

bool create_my_main_window(VAR_IN HINSTANCE)
{

    //WNDCLASSEX wcex;

    //wcex.cbSize = sizeof(WNDCLASSEX);

    //wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    //wcex.lpfnWndProc = WndProc;
    //wcex.cbClsExtra = 0;
    //wcex.cbWndExtra = 0;
    //wcex.hInstance = instance;
    //wcex.hIcon = nullptr;
    //wcex.hCursor = nullptr;
    //wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    ////wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_G);
    //wcex.lpszMenuName = nullptr;
    //wcex.lpszClassName = g_main_wndclass_name;
    //wcex.hIconSm = nullptr;

    //RegisterClassEx(&wcex);

    g_window = new GlWindow();
    g_window->Create(nullptr);
    g_window->ShowWindow(SW_SHOW);
    MsgIdleObserver::AddObserver(g_window);


    //HWND wnd;

    //wnd = CreateWindow(g_main_wndclass_name, L"main window", WS_OVERLAPPEDWINDOW,
    //    CW_USEDEFAULT, 0, 400, 400, NULL, NULL, instance, NULL);

    //if (!wnd)
    //{
    //    return false;
    //}

    //g_main_window = wnd;
    //ShowWindow(wnd, SW_SHOW);
    //UpdateWindow(wnd);

    return true;

}
