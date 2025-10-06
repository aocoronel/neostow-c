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

#include "printh.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void print_aligned(const char *str, int width) {
        printf("%-*s", width, str);
}

void printh_p(const char *msg, const char *style) {
        FILE *out = stdout;
        fprintf(out, "%s%s%s", style, msg, PH_RESET);
}

int print_d(const char *desc) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int width = w.ws_col;
        int line = PH_DESC_INDENT;
        printf("%*s", PH_DESC_INDENT, "");

        int i = 0;
        int start = 0;

        while (desc[i] != '\0') {
                if (isspace(desc[i]) || desc[i + 1] == '\0') {
                        int word_len =
                                i - start + (desc[i + 1] == '\0' ? 1 : 0);
                        if (line + word_len >= width) {
                                printf("\n%*s", PH_DESC_INDENT, "");
                                line = PH_DESC_INDENT;
                        }
                        for (int j = start; j < start + word_len; j++) {
                                putchar(desc[j]);
                        }
                        if (desc[i + 1] != '\0') {
                                putchar(' ');
                                line += word_len + 1;
                        } else {
                                line += word_len;
                        }
                        start = i + 1;
                }
                i++;
        }
        printf("\n");
        return 0;
}

static void get_cmd_full(const struct ProgramCommands *cmd, char *buffer,
                         size_t size) {
        if (cmd->argument) {
                snprintf(buffer, size, "%s %s", cmd->cmd, cmd->argument);
        } else {
                snprintf(buffer, size, "%s", cmd->cmd);
        }
}

int compare_commands(const void *a, const void *b) {
        const struct ProgramCommands *cmdA = (const struct ProgramCommands *)a;
        const struct ProgramCommands *cmdB = (const struct ProgramCommands *)b;

        char fullA[128], fullB[128];
        get_cmd_full(cmdA, fullA, sizeof(fullA));
        get_cmd_full(cmdB, fullB, sizeof(fullB));

        return strcmp(fullA, fullB);
}

static const char *get_flag_sort_key(const struct ProgramFlag *flag) {
        if (flag->short_flag) return flag->short_flag;
        if (flag->long_flag) return flag->long_flag;
        return "";
}

int compare_flags(const void *a, const void *b) {
        const struct ProgramFlag *flagA = (const struct ProgramFlag *)a;
        const struct ProgramFlag *flagB = (const struct ProgramFlag *)b;

        return strcmp(get_flag_sort_key(flagA), get_flag_sort_key(flagB));
}

int has_commands(ProgramInfo info) {
        int exist = 0;
        for (int i = 0; i < info.cmdc; i++) {
                if (info.commands->cmd != NULL) exist++;
        }
        return exist;
}

int has_options(ProgramInfo info) {
        int exist = 0;
        for (int i = 0; i < info.flagc; i++) {
                if (info.flags->long_flag != NULL ||
                    info.flags->short_flag != NULL)
                        exist++;
        }
        return exist;
}

void printh(ProgramInfo program_info) {
        FILE *out = stdout;

        qsort(program_info.commands, program_info.cmdc,
              sizeof(struct ProgramCommands), compare_commands);

        qsort(program_info.flags, program_info.flagc,
              sizeof(struct ProgramFlag), compare_flags);

        fprintf(out, "%s | %s\n\n", program_info.name, program_info.desc);

        printh_p("Usage:", PH_BOLD_UNDERLINE);
        fprintf(out, "  %s%s%s", PH_BOLD, program_info.name, PH_RESET);
        fprintf(out, " %s\n\n", program_info.usage);

        if (has_commands(program_info) != 0) {
                printh_p("Commands:", PH_BOLD_UNDERLINE);
                fprintf(out, "\n");

                for (int i = 0; i < program_info.cmdc; i++) {
                        const char *cmd = program_info.commands[i].cmd;
                        const char *arg = program_info.commands[i].argument;
                        const char *desc = program_info.commands[i].cmd_desc;

                        char cmd_full[128] = { 0 };

                        if (arg) {
                                snprintf(cmd_full, sizeof(cmd_full),
                                         "%s%s%s %s", PH_BOLD, cmd, PH_RESET,
                                         arg);
                        } else {
                                snprintf(cmd_full, sizeof(cmd_full), "%s%s%s",
                                         PH_BOLD, cmd, PH_RESET);
                        }

                        fprintf(out, "  %s\n", cmd_full);
                        if (desc && strlen(desc) > 0) {
                                print_d(desc);
                        }
                }
                fprintf(out, "\n");
        }

        if (has_options(program_info) != 0) {
                printh_p("Options:", PH_BOLD_UNDERLINE);
                fprintf(out, "\n");

                for (int i = 0; i < program_info.flagc; i++) {
                        const char *short_flag =
                                program_info.flags[i].short_flag;
                        const char *long_flag = program_info.flags[i].long_flag;
                        const char *arg = program_info.flags[i].argument;
                        const char *desc = program_info.flags[i].desc;

                        char flag_buffer[128] = { 0 };

                        if (short_flag && long_flag) {
                                snprintf(flag_buffer, sizeof(flag_buffer),
                                         "%s%s%s, %s%s%s", PH_BOLD, short_flag,
                                         PH_RESET, PH_BOLD, long_flag,
                                         PH_RESET);
                        } else if (long_flag) {
                                snprintf(flag_buffer, sizeof(flag_buffer),
                                         "%s%s%s", PH_BOLD, long_flag,
                                         PH_RESET);
                        } else if (short_flag) {
                                snprintf(flag_buffer, sizeof(flag_buffer),
                                         "%s%s%s", PH_BOLD, short_flag,
                                         PH_RESET);
                        }

                        if (arg) {
                                char upper_arg[32] = { 0 };
                                for (int j = 0; arg[j] && j < 30; j++) {
                                        upper_arg[j] = (char)toupper(arg[j]);
                                }

                                strncat(flag_buffer, " <",
                                        sizeof(flag_buffer) -
                                                strlen(flag_buffer) - 1);
                                strncat(flag_buffer, upper_arg,
                                        sizeof(flag_buffer) -
                                                strlen(flag_buffer) - 1);
                                strncat(flag_buffer, ">",
                                        sizeof(flag_buffer) -
                                                strlen(flag_buffer) - 1);
                        }

                        fprintf(out, "  %s\n", flag_buffer);
                        if (desc && strlen(desc) > 0) {
                                print_d(desc);
                        }
                }
        }
}
