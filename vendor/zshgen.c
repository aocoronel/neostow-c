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
#include "zshgen.h"

const char *find_comparg_zsh(const char *arg_name,
                             struct ProgramArguments *args, int args_count) {
        for (int i = 0; i < args_count; i++) {
                if (args[i].name != NULL &&
                    strcmp(args[i].name, arg_name) == 0) {
                        return args[i].completions;
                }
        }
        return NULL;
}

void generate_zsh_completion(const CompletionInfo *info) {
        const ProgramInfo *pinfo = info->info;
        // Header
        printf("#compdef %s\n\n", pinfo->name);
        // Environment defaults
        for (int i = 0; i < info->envc; i++) {
                printf("%s=%s\n", info->envs[i].name, info->envs[i].value);
        }
        // Main function
        printf("_%s() {\n", pinfo->name);
        printf("  local -a subcommands\n\n");
        // Define Subcommands
        printf("  subcommands=(\n");
        for (int i = 0; i < pinfo->cmdc; i++) {
                printf("    \"%s:%s\"\n", pinfo->commands[i].cmd,
                       pinfo->commands[i].cmd_desc);
        }
        printf("  )\n\n");
        // Define arguments
        printf("  _arguments -C \\\n");
        printf("    '1:command:->subcmds' \\\n");
        for (int i = 0; i < pinfo->flagc; i++) {
                const char *short_flag = pinfo->flags[i].short_flag;
                const char *long_flag = pinfo->flags[i].long_flag;
                const char *arg = pinfo->flags[i].argument;
                const char *desc = pinfo->flags[i].desc;
                const char *comp =
                        (arg != NULL) ?
                                find_comparg_zsh(arg, info->args, info->argc) :
                                NULL;
                if (long_flag) {
                        printf("    '%s", long_flag);
                        if (desc) printf("=[%s]", desc);
                        if (arg) {
                                if (comp) {
                                        printf(":%s:_%s_get_%s", arg,
                                               pinfo->name, arg);
                                } else {
                                        printf(":%s", arg);
                                }
                        }
                        printf("' \\\n");
                }
                if (short_flag) {
                        printf("    '%s", short_flag);
                        if (desc) printf("[%s]", desc);
                        if (arg) {
                                if (comp) {
                                        printf(":%s:_%s_get_%s", arg,
                                               pinfo->name, arg);
                                } else {
                                        printf(":%s", arg);
                                }
                        }
                        printf("' \\\n");
                }
        }
        printf("    '*::args:->command_args'\n\n");

        // Autocompletion
        printf("  case $state in\n");
        printf("    subcmds)\n");
        printf("      _describe 'command' subcommands\n");
        printf("      return\n");
        printf("      ;;\n");
        printf("    command_args)\n");
        printf("      case $words[1] in\n");

        // Autocomplete arguments from flags and commands
        for (int i = 0; i < pinfo->cmdc; i++) {
                const char *cmd = pinfo->commands[i].cmd;
                const char *cmdargs = pinfo->commands[i].argument;
                if (cmdargs == NULL) continue;
                const char *completions =
                        find_comparg_zsh(cmdargs, info->args, info->argc);
                if (completions == NULL) continue;

                printf("        %s)\n", cmd);
                printf("          _arguments \\\n");
                printf("            '*:%s:_%s_get_%s' \\\n", cmdargs,
                       info->info->name, cmdargs);
                printf("          ;;\n");
        }
        for (int i = 0; i < pinfo->flagc; i++) {
                const char *short_flag = pinfo->flags[i].short_flag;
                const char *long_flag = pinfo->flags[i].long_flag;
                const char *flagargs = pinfo->flags[i].argument;
                if (flagargs == NULL) continue;
                const char *completions =
                        find_comparg_zsh(flagargs, info->args, info->argc);

                if (completions == NULL) continue;

                if (short_flag != NULL && long_flag != NULL) {
                        printf("        %s | %s)\n", short_flag, long_flag);
                } else if (short_flag != NULL) {
                        printf("        %s)\n", short_flag);
                } else if (long_flag != NULL) {
                        printf("        %s)\n", long_flag);
                }
                printf("          _arguments \\\n");
                printf("            '*:%s:_%s_get_%s' \\\n", flagargs,
                       info->info->name, flagargs);
                printf("          return\n");
                printf("          ;;\n");
        }
        printf("      esac\n");
        printf("      ;;\n");
        printf("  esac\n");
        printf("}\n\n");

        // Define helper functions to autocomplete arguments
        for (int i = 0; i < info->argc; i++) {
                const char *name = info->args[i].name;
                const char *completions = info->args[i].completions;
                if (completions != NULL) {
                        printf("_%s_get_%s() {\n", pinfo->name, name);
                        printf("  local results\n");
                        printf("  results=(${(f)\"$(%s 2>/dev/null)\"})\n",
                               completions);
                        printf("  compadd -Q -a results\n");
                        printf("}\n\n");
                }
        }

        // Assign function to program
        printf("compdef _%s %s\n", pinfo->name, pinfo->name);
}
