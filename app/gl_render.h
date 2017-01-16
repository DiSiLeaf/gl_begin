#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <windows.h>

#include "deps/glew-2.0.0/include/GL/glew.h"

bool init_gl_render_data(HWND wnd);
void clean_data(void);

void display();


#endif