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

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <string.h>
#include "bashgen.h"

const char *find_comparg_bash(const char *arg_name,
                              struct ProgramArguments *args, int args_count) {
        for (int i = 0; i < args_count; i++) {
                if (args[i].name != NULL &&
                    strcmp(args[i].name, arg_name) == 0) {
                        return args[i].completions;
                }
        }
        return NULL;
}

void generate_bash_completion(const CompletionInfo *info) {
        // Header
        printf("#!/usr/bin/env bash\n");
        // Environment defaults
        for (int i = 0; i < info->envc; i++) {
                printf("%s=%s\n", info->envs[i].name, info->envs[i].value);
        }
        // Define helper functions to autocomplete arguments
        for (int i = 0; i < info->argc; i++) {
                if (info->args[i].completions == NULL) continue;
                printf("_%s() {\n", info->args[i].name);
                printf("  %s\n", info->args[i].completions);
                printf("}\n");
        }
        // Main function
        printf("_%s() {\n", info->info->name);
        printf("  COMPREPLY=()\n");
        printf("  cur=\"${COMP_WORDS[COMP_CWORD]}\"\n");
        printf("  prev=\"${COMP_WORDS[COMP_CWORD - 1]}\"\n");

        // Autocomplete flags
        printf("  case \"${cur}\" in\n");
        printf("  -*)\n");
        printf("    mapfile -t COMPREPLY < <(compgen -W \"");
        for (int i = 0; i < info->info->flagc; i++) {
                const char *short_flag = info->info->flags[i].short_flag;
                const char *long_flag = info->info->flags[i].long_flag;

                if (long_flag != NULL) {
                        printf(" %s", long_flag);
                }
                if (short_flag != NULL) {
                        printf(" %s", short_flag);
                }
        }
        printf("\" -- \"${cur}\"%c\n    return 0\n    ;;\n  esac\n", ')');

        // Autocomplete arguments from flags and commands
        printf("  case \"${prev}\" in\n");
        int j = 0;
        for (int i = 0; i < info->info->flagc; i++) {
                if (info->info->flags[i].argument != NULL &&
                    find_comparg_bash(info->info->flags[i].argument, info->args,
                                      info->argc) != NULL) {
                        const char *short_flag =
                                info->info->flags[i].short_flag;
                        const char *long_flag = info->info->flags[i].long_flag;
                        if (long_flag != NULL && short_flag != NULL) {
                                printf("  %s | %s%c\n", short_flag, long_flag,
                                       ')');
                        } else if (long_flag != NULL) {
                                printf("  %s%c\n", long_flag, ')');
                        } else if (short_flag != NULL) {
                                printf("  %s%c\n", short_flag, ')');
                        }
                        printf("    mapfile -t COMPREPLY < <(compgen -W \"$(_%s%c\" -- \"${cur}\"%c\n",
                               info->info->flags[i].argument, ')', ')');
                        printf("    return 0\n");
                        printf("    ;;\n");
                } else {
                        j++;
                        // If flags does not have argument with completions, fallback
                        if (j == info->info->flagc) {
                                printf("  \"\"%c\n", ')');
                                printf("    break\n    ;;\n");
                                break;
                        }
                }
        }
        printf("  esac\n");

        // Autocomplete commands
        printf("  mapfile -t COMPREPLY < <(compgen -W \"");
        for (int i = 0; i < info->info->cmdc; i++) {
                printf(" %s", info->info->commands[i].cmd);
        }
        printf("\" -- \"${cur}\"%c\n  return 0\n}\n", ')');

        // Assign function to program
        printf("complete -F _%s %s", info->info->name, info->info->name);
}
