#include "gl_render.h"

#include <iostream>

#include <windows.h>
#include <wingdi.h>

#include "deps/glew-2.0.0/include/GL/glew.h"

#include "data.h"

HGLRC g_gl_render_context = nullptr;


const GLchar *vertex_shader =                                                 "\
#version 330 core                                                            \n\
layout(location = 1) in float color_alpha;                                   \n\
out float o_color;                                                           \n\
void main()                                                                  \n\
{                                                                            \n\
    int x = gl_InstanceID % 100;                                             \n\
    int y = gl_InstanceID / 100;                                             \n\
    float xx = float(x) / 100.0;                                             \n\
    float yy = float(y) / 100.0;                                             \n\
    gl_Position = vec4(xx, yy, 0.0, 1.0);                                    \n\
    mat4 pos_mat = mat4(                                                     \n\
            2.0,  0.0,  0.0, 0.0,                                            \n\
            0.0,  2.0,  0.0, 0.0,                                            \n\
            0.0,  0.0,  1.0, 0.0,                                            \n\
            -1.0, -1.0, 0.0, 1.0                                             \n\
            );                                                               \n\
    gl_Position = pos_mat * gl_Position;                                     \n\
    o_color = color_alpha;                                                   \n\
}                                                                              \
";

const GLchar *fragment_shader =                                               "\
#version 330 core                                                            \n\
in float o_color;                                                            \n\
out vec4 fColor;                                                             \n\
void main()                                                                  \n\
{                                                                            \n\
    fColor = vec4(1.0, 0.0, 0.0, o_color);                                   \n\
}                                                                              \
";


static int init_gl_render_context(HWND wnd)
{
#ifdef RETURN
#error macro 'RETURN' has already existed;
#endif
#define RETURN(code, msg) {                                                    \
        err_code = -code;                                                      \
        err_msg = msg;                                                         \
        goto ERROR_##code;                                                     \
    }
    int err_code = 0;
    char *err_msg;
    char err_buf[256];
    
    HDC dc = GetDC(wnd);
    GLenum init_code;

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
        RETURN(1, "ChoosePixelFormat error");

    if (FALSE == SetPixelFormat(dc, formet_num, &pfd))
        RETURN(2, "SetPixelFormat error");

    g_gl_render_context = wglCreateContext(dc);
    if (nullptr == g_gl_render_context)
        RETURN(3, "wglCreateContext error");


    if (FALSE == wglMakeCurrent(dc, g_gl_render_context)) {
        DWORD err = GetLastError();
        char *err_buf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&err_buf,
            0, NULL);
        RETURN(4, "wglMakeCurrent error");
    }

    init_code = glewInit();
    if (GLEW_OK != init_code) {
        memcpy_s(err_buf, 255, "glewInit error ", 15);
        _itoa_s(init_code, err_buf + 15, 5, 10);
        err_buf[20] = '\0';
        RETURN(5, err_buf);
    }

    return 0;

ERROR_5 :
    wglMakeCurrent(NULL, NULL);
ERROR_4:
    wglDeleteContext(g_gl_render_context);
    g_gl_render_context = nullptr;
ERROR_3:
ERROR_2 :
ERROR_1 :
    std::cout << err_msg << std::endl;
    return err_code;
}


static int LoadShader(GLenum type, const GLchar *source, GLuint *ret_shader)
{
    GLuint shader = glCreateShader(type);

    if (GL_INVALID_ENUM == shader) {
        return -1;
    }

    if (0 == shader) {
        return -2;
    }

    glShaderSource(shader, 1, &source, (const GLint*)0);
    glCompileShader(shader);

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (GL_TRUE != compile_status) {

        glDeleteShader(shader);
        return -3;
    }

    *ret_shader = shader;
    return 0;
}

static int SetupProgram(const GLuint *shaders, int size, GLuint *ret_prog)
{
    GLuint program = glCreateProgram();
    if (0 == program) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
        glAttachShader(program, shaders[i]);
    }

    glLinkProgram(program);
    
    GLint link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);

    if (GL_TRUE != link_status) {

        char log[256];
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        glGetProgramInfoLog(program, 255, &len, log);
        glDeleteShader(program);
        return -2;
    }

    *ret_prog = program;
    return 0;
}

static GLuint v_a[5];
static GLuint b_o[2];

bool init_gl_render_data(HWND wnd)
{
    if (0 != init_gl_render_context(wnd))
        return false;

    int vertex_array_index = 0;

    glGenVertexArrays(2, v_a);
    glGenBuffers(2, b_o);
    GLfloat verteices[2][3][2] = {
        {
            { -0.90f, -0.90f },
            { 0.85f, -0.90f },
            { -0.90f, 0.85f }
        },
        {
            { 0.90f, -0.85f },
            { 0.90f, 0.90f },
            { -0.85f, 0.90f }
        }
    };
    verteices;


    //array 0
    vertex_array_index = 0;
    glBindVertexArray(v_a[vertex_array_index]);

    glBindBuffer(GL_ARRAY_BUFFER, b_o[vertex_array_index]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(verteices), 
    //    reinterpret_cast<const void*>((int)verteices), GL_STATIC_DRAW);

    //glBufferData(GL_ARRAY_BUFFER, sizeof(font_data), reinterpret_cast<const GLvoid*>(font_data), GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(font_data), nullptr, GL_STATIC_DRAW);

    for (int i = 0; i < 100; i++) {
        glBufferSubData(GL_ARRAY_BUFFER, (99 - i) * 100, 100, reinterpret_cast<const GLvoid*>(font_data + i * 100));
    }


    GLuint shaders[2];

    if (0 != LoadShader(GL_VERTEX_SHADER, vertex_shader, shaders)) {
        std::cout << "load vertex shader error" << std::endl;

        return false;
    }

    if (0 != LoadShader(GL_FRAGMENT_SHADER, fragment_shader, shaders + 1)) {
        std::cout << "load fragment shader error" << std::endl;

        return false;
    }

    GLuint program;
    if (0 != SetupProgram(shaders, 2, &program)) {
        std::cout << "pipe line error" << std::endl;

        return false;
    }

    glUseProgram(program);

    
    //glVertexAttribPointer(
    //    1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    

    glClearColor(1.0, 1.0, 1.0, 1.0f);
    glPointSize(4.0);

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    return true;

}

void clean_data(void)
{
    if (nullptr == g_gl_render_context) {
        return;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_gl_render_context);
    g_gl_render_context = nullptr;
}

void display()
{
    static int tf = 0;
    static int count = 0;

    //glClearColor(1.0, 1.0, 1.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    glBindVertexArray(v_a[0]);
    glDrawArraysInstanced(GL_POINTS, 0, 1, 10000);


    glFlush();

}