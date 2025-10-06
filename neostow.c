#include "vendor/printfc.h"
#include "vendor/bashgen.h"
#include "vendor/zshgen.h"
#include "vendor/printh.h"
#include "include/autocomplete.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_LEN 4096
#define MAX_LINE_LEN 1024
#define MAX_ENTRIES 1000

bool VERBOSE = false;
bool FAIL_SAFE = false;
bool REMOVE = false;
bool DELETE = false;
bool DEBUG_MODE = false;

struct config_data {
        char *file;
        char *src;
        char *dst;
};

struct config_data entries[MAX_ENTRIES];
int count = 0;

static struct option long_options[] = { { "verbose", no_argument, 0, 'V' },
                                        { "help", no_argument, 0, 'h' },
                                        { "config", required_argument, 0, 'c' },
                                        { "debug", no_argument, 0, 'D' },
                                        { "delete", no_argument, 0, 'd' },
                                        { "remove", no_argument, 0, 'r' },
                                        { "version", no_argument, 0, 'v' },
                                        { 0, 0, 0, 0 } };

char *expand_value(const char *value);
char *realpath(const char *path, char *resolved_path);
char *safe_strdup(const char *s);
const char *basename_from_path(const char *path);
int edit_config(char *config_file);
int ensure_directory(const char *path);
int neostow(int i);
int read_config(char *file);
int symlink(const char *target, const char *linkpath);
void trim_newline(char *str);
void version();

int main(int argc, char *argv[]) {
        int opt = 0;
        char *NEOSTOW_FILE = ".neostow";
        char cwd[MAX_PATH_LEN];
        char config_file[MAX_PATH_LEN + 256];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
                // printfc(DEBUG, "Current working directory: %s\n", cwd);
        } else {
                printfc(FATAL, "failed to get current working directory.\n");
                return EXIT_FAILURE;
        }

        while ((opt = getopt_long(argc, argv, ":rDdvVhc:", long_options,
                                  NULL)) != -1) {
                switch (opt) {
                case 'c':
                        NEOSTOW_FILE = optarg;
                        break;
                case 'r':
                        REMOVE = true;
                        break;
                case 'D':
                        DEBUG_MODE = true;
                        break;
                case 'V':
                        VERBOSE = true;
                        break;
                case 'v':
                        version();
                        return EXIT_SUCCESS;
                case 'h':
                        printh(program_info);
                        return EXIT_SUCCESS;
                case ':':
                        printf("option '%c' needs a value\n", opt);
                        return EXIT_FAILURE;
                case '?':
                        printf("unknown option: %c\n", optopt);
                        return EXIT_FAILURE;
                }
        }

        snprintf(config_file, sizeof(config_file), "%s/%s", cwd, NEOSTOW_FILE);
        struct stat st;
        if (stat(config_file, &st) != 0) {
                printfc(FATAL, "%s\n", strerror(errno));
                return EXIT_FAILURE;
        };
        if (S_ISREG(st.st_mode)) {
                if (DEBUG_MODE) printfc(DEBUG, "found config file\n");
        } else {
                if (DEBUG_MODE) printfc(DEBUG, "neostow file not found\n");
                return EXIT_FAILURE;
        }

        if (optind < argc && strcmp(argv[optind], "autocomplete") == 0) {
                optind++;
                if (optind >= argc) {
                        fprintf(stderr, "Usage: %s autocomplete bash | zsh\n",
                                argv[0]);
                        return EXIT_FAILURE;
                }

                const char *shell = argv[optind];
                if (strcmp(shell, "bash") == 0) {
                        generate_bash_completion(&completion_info);
                        return EXIT_SUCCESS;
                } else if (strcmp(shell, "zsh") == 0) {
                        generate_zsh_completion(&completion_info);
                        return EXIT_SUCCESS;
                } else {
                        fprintf(stderr, "Usage: %s autocomplete bash | zsh\n",
                                argv[0]);
                        return EXIT_FAILURE;
                }
        } else if (optind < argc && strcmp(argv[optind], "edit") == 0) {
                optind++;
                if (edit_config(config_file) != 0)
                        return EXIT_FAILURE;
                else
                        return EXIT_SUCCESS;
        } else if (optind < argc && strcmp(argv[optind], "delete") == 0) {
                optind++;
                DELETE = true;
                REMOVE = true;
        }

        if (read_config(config_file) != 0) return EXIT_FAILURE;
        int i;
        for (i = 0; i < count; i++) {
                if (neostow(i) != 0) {
                        if (DEBUG_MODE) printfc(DEBUG, "%s failed");
                };
                free(entries[i].src);
                free(entries[i].dst);
                free(entries[i].file);
        }

        return EXIT_SUCCESS;
}

int edit_config(char *config_file) {
        char *editor = getenv("EDITOR");
        if (!editor) {
                editor = "vim";
        }
        char cmd[MAX_PATH_LEN + 50]; // + editor length
        snprintf(cmd, sizeof(cmd), "%s %s", editor, config_file);
        if (system(cmd) != 0)
                return EXIT_FAILURE;
        else
                return EXIT_SUCCESS;
}

char *safe_strdup(const char *s) {
        if (!s) return NULL;
        char *p = malloc(strlen(s) + 1);
        if (p) strcpy(p, s);
        return p;
}

char *expand_value(const char *value) {
        if (!value || value[0] == '\0') return safe_strdup("");

        char temp[MAX_PATH_LEN];

        if (value[0] == '~') {
                const char *home = getenv("HOME");
                if (!home) home = "";
                snprintf(temp, sizeof(temp), "%s%s", home, value + 1);
        } else if (value[0] == '$') {
                const char *env = getenv(value + 1);
                snprintf(temp, sizeof(temp), "%s", env ? env : "");
        } else {
                snprintf(temp, sizeof(temp), "%s", value);
        }

        char *resolved = realpath(temp, NULL);
        if (resolved) return resolved;

        if (temp[0] != '/') {
                char cwd[MAX_PATH_LEN];
                if (getcwd(cwd, sizeof(cwd))) {
                        char *full_path =
                                malloc(strlen(cwd) + 1 + strlen(temp) + 1);
                        if (!full_path) return safe_strdup(temp); // fallback
                        sprintf(full_path, "%s/%s", cwd, temp);
                        return full_path;
                }
        }

        return safe_strdup(temp);
}

void trim_newline(char *str) {
        size_t len = strlen(str);
        if (len && str[len - 1] == '\n') str[len - 1] = '\0';
}

void version() {
        printf("%s", "0.1.0");
        return;
}

const char *basename_from_path(const char *path) {
        const char *slash = strrchr(path, '/');
        return slash ? slash + 1 : path;
}

int read_config(char *file) {
        FILE *fp = fopen(file, "r");
        if (!fp) {
                printfc(FATAL, "%s\n", strerror(errno));
                return EXIT_FAILURE;
        }
        int linenum = 0;
        char line[MAX_LINE_LEN];
        char *eq_pos;
        char *key;
        char *value;
        char *expanded_value;
        char *expanded_key;

        while (fgets(line, MAX_LINE_LEN, fp)) {
                linenum++;
                if (count >= MAX_ENTRIES) {
                        printfc(WARN, "too many entries\n");
                        printfc(WARN, "truncated entries\n");
                        break;
                }

                line[strcspn(line, "\r\n")] = '\0';

                eq_pos = strchr(line, '=');
                if (!eq_pos) {
                        printfc(ERROR, "Line %d: %s\n", linenum, line);
                        continue;
                }

                *eq_pos = '\0';
                key = line;
                value = eq_pos + 1;

                expanded_value = expand_value(value);
                expanded_key = expand_value(key);
                if (!expanded_value) {
                        printfc(ERROR, "failed to expand value for key '%s'\n",
                                key);
                        continue;
                }

                const char *filename = basename_from_path(key);
                entries[count].file = safe_strdup(filename);
                entries[count].src = safe_strdup(expanded_key);
                entries[count].dst = expanded_value;
                if (!entries[count].src) {
                        free(expanded_value);
                        printfc(ERROR, "failed to allocate memory for key\n");
                        continue;
                }

                count++;
        }

        fclose(fp);

        return EXIT_SUCCESS;
}

int ensure_directory(const char *path) {
        char path_copy[MAX_PATH_LEN];
        size_t len = strlen(path);
        if (len >= sizeof(path_copy)) {
                return 1;
        }
        strcpy(path_copy, path);

        char *p = path_copy;
        while (*p) {
                if (*p == '/') {
                        *p = '\0';
                        struct stat st;
                        if (stat(path_copy, &st) != 0) {
                                if (mkdir(path_copy, 0700) != 0) return 2;
                        }
                        *p = '/';
                }
                p++;
        }
        struct stat st;
        if (stat(path, &st) != 0) {
                if (mkdir(path, 0700) != 0) return 2;
        }
        return 0;
}

int neostow(int i) {
        struct stat st_src;
        if (stat(entries[i].src, &st_src) != 0) {
                printfc(ERROR, "%s\n", strerror(errno));
                return EXIT_FAILURE;
        };
        if (S_ISREG(st_src.st_mode)) {
                if (DEBUG_MODE) printfc(DEBUG, "found %s\n", entries[i].src);
        } else {
                if (DEBUG_MODE)
                        printfc(DEBUG, "not found %s\n", entries[i].src);
                return EXIT_FAILURE;
        }

        struct stat st_dst;
        if (stat(entries[i].dst, &st_dst) != 0) {
                printfc(ERROR, "%s\n", strerror(errno));
                return EXIT_FAILURE;
        };
        if (S_ISDIR(st_dst.st_mode)) {
                if (DEBUG_MODE) printfc(DEBUG, "found %s\n", entries[i].dst);
        } else {
                if (DEBUG_MODE)
                        printfc(DEBUG, "not found %s\n", entries[i].dst);
                if (FAIL_SAFE) return EXIT_FAILURE;
        }

        char filepath[MAX_PATH_LEN];
        snprintf(filepath, MAX_PATH_LEN, "%s/%s", entries[i].dst,
                 entries[i].file);

        if (FAIL_SAFE == false) ensure_directory(entries[i].dst);

        if (REMOVE) {
                struct stat st_file;
                if (stat(filepath, &st_file) == 0) {
                        if (remove(filepath) != 0) {
                                printfc(ERROR, "Failed to remove %s: %s\n",
                                        filepath, strerror(errno));
                                return EXIT_FAILURE;
                        }
                } else if (errno != ENOENT) {
                        printfc(ERROR, "%s\n", strerror(errno));
                        return EXIT_FAILURE;
                }

                if (DELETE) return EXIT_SUCCESS;
        }

        if (symlink(entries[i].src, filepath) == 0) {
                if (VERBOSE)
                        printf("%s ==> %s\n", entries[i].file, entries[i].dst);
        } else {
                printfc(ERROR, "%s ==> %s\n", strerror(errno), entries[i].file);
        }

        return EXIT_SUCCESS;
}
