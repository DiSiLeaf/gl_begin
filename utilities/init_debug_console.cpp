#include "utilities/init_debug_console.h"

#include <stdio.h>

#include <iostream>

#include <windows.h>

void init_debug_env()
{
    if (TRUE == AllocConsole()) {
        FILE *unknow_fp = nullptr;

        freopen_s(&unknow_fp, "CONOUT$", "w", stdout);
    }
}