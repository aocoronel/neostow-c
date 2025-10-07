#include "bashgen.h"
#include "zshgen.h"
#include "stddef.h"

struct ProgramArguments args[] = {
        { "SHELL", "echo -e 'bash\\nzsh'" },
        { "CONFIG", "ls" },
};

struct ProgramCommands commands[] = {
        { "autocomplete", "SHELL", "Generate autocompletion for bash or zsh" },
        { "edit", NULL, "Edit the configuration file" },
        { "delete", NULL, "Delete symlinks" },
};

struct ProgramFlag flags[] = {
        { "-c", "--config", "CONFIG", "Load an alternative config" },
        { "-D", "--debug", NULL, "Enables debug verbosity" },
        { "-d", "--dry", NULL, "Describe potential operations" },
        { "-o", "--overwrite", NULL, "Overwrite existing symlinks" },
        { "-h", "--help", NULL, "Displays this message and exits" },
        { "-F", "--force", NULL, "Skip prompt dialogs" },
        { "-V", "--verbose", NULL, "Enable verbosity" },
        { "-v", "--version", NULL, "Displays program version" },
};

ProgramInfo program_info = {
        .flagc = sizeof(flags) / sizeof(flags[0]),
        .cmdc = sizeof(commands) / sizeof(commands[0]),
        .name = "neostow",
        .desc = "The Declarative GNU Stow",
        .usage = "[OPTIONS] <COMMAND>",
        .commands = commands,
        .flags = flags,
};

// struct ProgramEnv envs[] = {
// };

CompletionInfo completion_info = {
        .info = &program_info,
        .argc = sizeof(args) / sizeof(args[0]),
        // .envc = sizeof(envs) / sizeof(envs[0]),
        .args = args,
        // .envs = envs,
};
