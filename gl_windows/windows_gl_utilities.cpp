#include "windows_gl_utilities.h"

#include <iostream>

#include <windows.h>

#include "deps/glew-2.0.0/include/GL/glew.h"

static int g_gl_major_version = 0;
static int g_gl_minor_version = 0;


static LRESULT CALLBACK WndProc(HWND wnd, UINT message, WPARAM w_p, LPARAM l_p)
{
    return DefWindowProc(wnd, message, w_p, l_p);
}

static int create_dummy_window_class(HINSTANCE instance, ATOM *wnd_class)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_G);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = TEXT("gl dummy class");
    wcex.hIconSm = nullptr;

    ATOM ret = RegisterClassEx(&wcex);

    if (0 == ret)
        return -1;
    
    *wnd_class = ret;
    return 0;
}

static int create_dummy_window(HINSTANCE instance, ATOM wnd_class, HWND *wnd)
{
    HWND wd;

    wd = CreateWindow((LPCTSTR)MAKELONG(wnd_class, 0), TEXT("dummy window"),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
            NULL, NULL, instance, NULL);

    if (!wd)
    {
        return -1;
    }

    *wnd = wd;
    return 0;
}


static int create_dummy_gl_context(HWND wnd, HGLRC *ret_gl_context)
{
    HGLRC gl_context = nullptr;
    HDC dc = GetDC(wnd);

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
        32,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                        //Number of bits for the depthbuffer
        8,                        //Number of bits for the stencilbuffer
        0,                        //Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int formet_num = ChoosePixelFormat(dc, &pfd);

    if (0 == formet_num)
        return -1;

    if (FALSE == SetPixelFormat(dc, formet_num, &pfd))
        return -2;

    gl_context = wglCreateContext(dc);
    if (nullptr == gl_context)
        return -3;

    *ret_gl_context = gl_context;
    return 0;
}

static void init_gl_version(const GLubyte *gl_raw_ver)
{
    if (!gl_raw_ver)
        return;

    const char *ver = reinterpret_cast<const char *>(gl_raw_ver);
    const char *pos = ver;
    int ma_v;
    int mi_v;

    for (; ' ' == *pos; ++pos);

    if (!('0' <= *pos && '9' >= *pos))
        return;


    //major version
    ma_v = *pos - '0';
    pos++;
    for (; '\0' == pos[0]; ++pos)
        if (pos[0] >= '0' && pos[0] <= '9') {
            ma_v *= 10;
            ma_v += pos[0] - '0';
        }
        else {
            break;
        }

        if ('.' != pos[0])
            return;

        ++pos;
        if (!('0' <= *pos && '9' >= *pos))
            return;
        //minor version
        mi_v = *pos - '0';
        pos++;
        for (; '\0' == pos[0] && (pos[0] >= '0' && pos[0] <= '9'); ++pos) {
            ma_v *= 10;
            ma_v += pos[0] - '0';
        }

        g_gl_major_version = ma_v;
        g_gl_minor_version = mi_v;
}


int init_windows_gl(HINSTANCE instance)
{
#ifdef RETURN
#error macro 'RETURN' has existed
#endif
#define RETURN(code) {                                                         \
    err_code = -code;                                                          \
    goto ERROR_##code;                                                         \
}
    int err_code = 0;

    ATOM class_atom;
    HWND wnd;
    HGLRC gl_context;
    HDC dc;
    const GLubyte *raw_ver = nullptr;

    if (0 != create_dummy_window_class(instance, &class_atom))
        RETURN(1);

    if (0 != create_dummy_window(instance, class_atom, &wnd))
        RETURN(2);

    if (0 != create_dummy_gl_context(wnd, &gl_context))
        RETURN(3);

    dc = GetDC(wnd);

    if (FALSE == wglMakeCurrent(dc, gl_context))
        RETURN(4);

    if (GLEW_OK != glewInit())
        RETURN(5);

    raw_ver = glGetString(GL_VERSION);
    if (!raw_ver) {
        RETURN(6);
    }

    init_gl_version(raw_ver);

    if (g_gl_major_version < 3 || (3 == g_gl_major_version
            && 3 > g_gl_minor_version))
        RETURN(7);

    std::cout << "current version: "
        << g_gl_major_version << "." << g_gl_minor_version << std::endl;


    RETURN(0);

ERROR_0:
ERROR_7:
ERROR_6:
ERROR_5:
    wglMakeCurrent(NULL, NULL);
ERROR_4:
    wglDeleteContext(gl_context);
ERROR_3:
    DestroyWindow(wnd);
ERROR_2:
    UnregisterClass((LPCTSTR)MAKELONG(class_atom, 0), instance);
ERROR_1:
    return err_code;

#undef RETURN
}

int gl_major_ver()
{
    return g_gl_major_version;
}

int gl_minor_ver()
{
    return g_gl_major_version;
}