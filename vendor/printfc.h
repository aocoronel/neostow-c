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

#ifndef PRINTFC_H
#define PRINTFC_H

#define COLOR_RED "\x1b[91m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_GREEN "\x1b[38;5;47m"
#define COLOR_BLUE "\x1b[38;5;75m"
#define COLOR_RESET "\x1b[0m"

/*
 * Holds printfc options
*/
typedef enum { FATAL, ERROR, WARN, INFO, DEBUG } LogLevel;

/*
 * Write formatted output to stderr or stdout
 *
 * FATAL, ERROR => stderr
 *
 * INFO, DEBUG, WARN => stdout
 *
 * printfc(FATAL, "Something went very wrong: %s", "disk not found");
*/
int printfc(LogLevel level, const char *restrict fmt, ...);

#endif // PRINTFC_H
