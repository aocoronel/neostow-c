/*
 * MIT License
 *
 * Copyright (c) 2025 Augusto Coronel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "printfc.h"
#include <stdarg.h>
#include <stdio.h>

#define PFATAL(...) printfc(FATAL, __VA_ARGS__)
#define PERROR(...) printfc(ERROR, __VA_ARGS__)
#define PWARN(...) printfc(WARN, __VA_ARGS__)
#define PINFO(...) printfc(INFO, __VA_ARGS__)
#define PDEBUG(...) printfc(DEBUG, __VA_ARGS__)

int printfc(LogLevel level, const char *restrict fmt, ...) {
        const char *color;
        const char *label;
        FILE *out = stdout;

        switch (level) {
        case FATAL:
                color = COLOR_RED;
                label = "FATAL";
                out = stderr;
                break;
        case ERROR:
                color = COLOR_RED;
                label = "ERROR";
                out = stderr;
                break;
        case WARN:
                color = COLOR_YELLOW;
                label = "WARNING";
                break;
        case INFO:
                color = COLOR_GREEN;
                label = "INFO";
                break;
        case DEBUG:
                color = COLOR_BLUE;
                label = "DEBUG";
                break;
        default:
                return 1;
        }

        va_list args;
        va_start(args, fmt);
        fprintf(out, "%s[%s]:%s ", color, label, COLOR_RESET);
        vfprintf(out, fmt, args);
        va_end(args);
        return 0;
}
