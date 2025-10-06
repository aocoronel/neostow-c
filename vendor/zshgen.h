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

#ifndef ZSHGEN_H
#define ZSHGEN_H

#ifndef PROGRAM_VAR
#define PROGRAM_VAR
/*
 * Define the program command details
 *
 * command, argument, description
 */
struct ProgramCommands {
  const char *cmd;
  const char *argument;
  const char *cmd_desc;
};

/*
 * Define the program flag details
 *
 * short flag, long flag, argument, description
 */
struct ProgramFlag {
        const char *short_flag;
        const char *long_flag;
        const char *argument;
        const char *desc;
};

/*
 * Define the program info
 *
 * flag count, command count, program name, program description, program usage,
 * ProgramCommands, ProgramFlag
 */
typedef struct {
        int flagc;
        int cmdc;
        const char *name;
        const char *desc;
        const char *usage;
        struct ProgramCommands *commands;
        struct ProgramFlag *flags;
} ProgramInfo;
#endif // !PROGRAM_VAR

#ifndef BZ_COMPLETION
#define BZ_COMPLETION
/*
 * Define the program arguments and completions
 *
 * argument, completion
*/
struct ProgramArguments {
        const char *name;
        const char *completions;
};

/*
 * Define the environment variables
 *
 * name, value
*/
struct ProgramEnv {
        const char *name;
        const char *value;
};

/*
 * Includes the program info and program arguments
 *
 * ProgramInfo, ProgramArguments
*/
typedef struct {
        ProgramInfo *info;
        int argc;
        int envc;
        struct ProgramArguments *args;
        struct ProgramEnv *envs;
} CompletionInfo;
#endif // !BZ_COMPLETION

/*
 * Generates bash completions for commands, flags and arguments
*/
void generate_zsh_completion(const CompletionInfo *info);
#endif // ZSHGEN_H
