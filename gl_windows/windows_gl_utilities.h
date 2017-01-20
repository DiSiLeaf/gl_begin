#ifndef WINDOWS_GL_UTILITIES_H
#define WINDOWS_GL_UTILITIES_H

#include <windows.h>

#include "utilities/funtion_annotation_macro.h"

int init_windows_gl(VAR_IN HINSTANCE instance);
int gl_major_ver(void);
int gl_minor_ver(void);



#endif