#include "vendor/printfc.h"
#include "vendor/bashgen.h"
#include "vendor/zshgen.h"
#include "vendor/printh.h"
#include "include/autocomplete.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH_LEN 4096
#define MAX_LINE_LEN 1024
#define MAX_ENTRIES 1000

bool VERBOSE = false;
bool DRY_MODE = false;
bool REMOVE = false;
bool DELETE = false;
bool FORCE = false;
bool DEBUG_MODE = false;

int operations = 0;
int count = 0;

char *NEOSTOW_FILE = ".neostow";

struct config_data {
        char *file;
        char *src;
        char *dst;
};

struct config_data entries[MAX_ENTRIES];

static struct option long_options[] = { { "verbose", no_argument, 0, 'V' },
                                        { "help", no_argument, 0, 'h' },
                                        { "file", required_argument, 0, 'f' },
                                        { "debug", no_argument, 0, 'D' },
                                        { "dry", no_argument, 0, 'd' },
                                        { "force", no_argument, 0, 'F' },
                                        { "delete", no_argument, 0, 'd' },
                                        { "overwrite", no_argument, 0, 'o' },
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
        char cwd[MAX_PATH_LEN];
        char config_file[MAX_PATH_LEN + 256];

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                printfc(FATAL, "failed to get current working directory.\n");
                return EXIT_FAILURE;
        }

        while ((opt = getopt_long(argc, argv, ":oFDdvVhf:", long_options,
                                  NULL)) != -1) {
                switch (opt) {
                case 'f':
                        NEOSTOW_FILE = optarg;
                        break;
                case 'F':
                        FORCE = true;
                        break;
                case 'o':
                        REMOVE = true;
                        break;
                case 'D':
                        DEBUG_MODE = true;
                        break;
                case 'V':
                        VERBOSE = true;
                        break;
                case 'd':
                        DRY_MODE = true;
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
        }

        snprintf(config_file, sizeof(config_file), "%s/%s", cwd, NEOSTOW_FILE);
        struct stat st;
        if (stat(config_file, &st) != 0) {
                printfc(FATAL, "%s\n", strerror(errno));
                return EXIT_FAILURE;
        };
        if (S_ISREG(st.st_mode)) {
                if (DEBUG_MODE) printfc(DEBUG, "found neostow file\n");
        } else {
                if (DEBUG_MODE) printfc(DEBUG, "neostow file not found\n");
                return EXIT_FAILURE;
        }

        if (optind < argc && strcmp(argv[optind], "edit") == 0) {
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
                        if (DEBUG_MODE)
                                printfc(ERROR,
                                        "an error occurred while processing %s\n",
                                        entries[i].file);
                };
                free(entries[i].src);
                free(entries[i].dst);
                free(entries[i].file);
        }

        printf("%d operations were applied.\n", operations);

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

static void trim(char *str) {
        if (!str) return;

        char *start = str;
        while (*start && isspace((unsigned char)*start)) {
                start++;
        }

        if (start != str) {
                memmove(str, start, strlen(start) + 1);
        }

        char *end = str + strlen(str) - 1;
        while (end >= str && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
        }
}

char *expand_value(const char *value) {
        if (!value || value[0] == '\0') return safe_strdup("");

        char temp[MAX_PATH_LEN];

        if (value[0] == '~' && value[1] == '/') {
                const char *home = getenv("HOME");
                if (!home) home = "";
                snprintf(temp, sizeof(temp), "%s%s", home, value + 2);
                trim(temp);
        } else if (value[0] == '$') {
                const char *slash = strchr(value, '/');
                char varname[256];

                if (slash) {
                        size_t len = slash - value - 1;
                        strcpy(varname, "");
                        strncpy(varname, value + 1, len);
                        varname[len] = '\0';
                } else {
                        strcpy(varname, value + 1);
                }

                const char *env = getenv(varname);
                if (!env) env = "";

                snprintf(temp, sizeof(temp), "%s%s", env, slash ? slash : "");
                trim(temp);
        } else if (value[0] == '.') {
                snprintf(temp, sizeof(temp), "%s", value + 1);
                trim(temp);
        } else {
                snprintf(temp, sizeof(temp), "%s", value);
                trim(temp);
        }

        char *resolved = realpath(temp, NULL);
        if (resolved) return resolved;

        if (temp[0] != '/') {
                char cwd[MAX_PATH_LEN];
                if (getcwd(cwd, sizeof(cwd))) {
                        char *full_path =
                                malloc(strlen(cwd) + 1 + strlen(temp) + 1);
                        if (!full_path) return safe_strdup(temp);
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
        printf("%s", "1.0.0");
        return;
}

const char *basename_from_path(const char *path) {
        const char *slash = strrchr(path, '/');
        return slash ? slash + 1 : path;
}

const char *memrchr(const char *s, int c, size_t n) {
        const char *end = s + n;
        while (end > s) {
                if (*--end == (char)c) {
                        return end;
                }
        }
        return NULL;
}

char *dirname_from_path(char *path) {
        const char *slash = strrchr(path, '/');
        if (!slash || slash == path) {
                return NULL;
        }
        const char *prev_slash = memrchr(path, '/', slash - path);
        if (!prev_slash) {
                path[slash - path] = '\0';
                return path;
        }
        size_t len = prev_slash - path + 1;
        path[len] = '\0';
        return path;
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
        const char *data_src;
        const char *data_dst;
        char *expanded_dst;
        char *expanded_src;

        while (fgets(line, MAX_LINE_LEN, fp)) {
                linenum++;
                if (count >= MAX_ENTRIES) {
                        printfc(WARN, "too many entries\n");
                        printfc(WARN, "truncated entries\n");
                        break;
                }

                if (line[0] == '#' || strlen(line) < 3) continue;
                line[strcspn(line, "\r\n")] = '\0';

                char *hash = strchr(line, '#');
                if (hash) *hash = '\0';

                eq_pos = strchr(line, '=');
                if (!eq_pos) {
                        data_src = line;
                        expanded_src = expand_value(data_src);
                        data_dst = dirname_from_path(expand_value(line));
                        if (data_dst == NULL) {
                                printfc(ERROR,
                                        "failed to get destination for %s",
                                        data_src);
                                continue;
                        }
                        expanded_dst = expand_value(data_dst);
                } else {
                        *eq_pos = '\0';
                        data_src = line;
                        data_dst = eq_pos + 1;
                        expanded_dst = expand_value(data_dst);
                        expanded_src = expand_value(data_src);
                }

                if (!expanded_dst) {
                        printfc(ERROR,
                                "failed to expand value for destination '%s'\n",
                                data_dst);
                        continue;
                }
                if (!expanded_src) {
                        printfc(ERROR,
                                "failed to expand value for source '%s'\n",
                                data_src);
                        continue;
                }

                if (DEBUG_MODE) {
                        printfc(DEBUG, "Destination: %s\n", data_dst);
                        printfc(DEBUG, "Expanded Destination: %s\n",
                                expanded_dst);
                        printfc(DEBUG, "Source: %s\n", data_src);
                        printfc(DEBUG, "Expanded Source: %s\n\n", expanded_src);
                }

                const char *filename = basename_from_path(data_src);
                entries[count].file = safe_strdup(filename);
                entries[count].src = safe_strdup(expanded_src);
                entries[count].dst = expanded_dst;
                if (!entries[count].src) {
                        free(expanded_src);
                        printfc(ERROR,
                                "failed to allocate memory for source file: %s\n",
                                entries[count].src);
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

int diff(const char *src, const char *dst) {
        pid_t pid = fork();
        if (pid == 0) {
                execlp("diff", "diff", "-u", src, dst, (char *)NULL);
                _exit(127);
        } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                return status;
        } else {
                printfc(ERROR, "%s ==> %s\n", strerror(errno), src);
                return -1;
        }
}

int neostow(int i) {
        bool use_basename = false;
        char filepath[MAX_PATH_LEN + MAX_PATH_LEN];
        struct stat st_src;
        const char *src_basename;
        {
                bool found_src = false;
                bool found_dst = false;
                struct stat st_dst;

                if (stat(entries[i].src, &st_src) != 0) {
                        printfc(ERROR, "%s ==> %s\n", strerror(errno),
                                entries[i].file);
                        return EXIT_FAILURE;
                }
                if (S_ISREG(st_src.st_mode) || S_ISDIR(st_src.st_mode)) {
                        found_src = true;
                }
                if (S_ISDIR(st_src.st_mode)) {
                        use_basename = true;
                        src_basename = basename_from_path(entries[i].src);
                        snprintf(filepath, MAX_PATH_LEN, "%s/%s",
                                 entries[i].dst, src_basename);
                } else {
                        snprintf(filepath, MAX_PATH_LEN, "%s/%s",
                                 entries[i].dst, entries[i].file);
                }
                if (stat(entries[i].dst, &st_dst) != 0) {
                        printfc(ERROR, "%s ==> %s\n", strerror(errno),
                                entries[i].file);
                        return EXIT_FAILURE;
                }
                if (S_ISDIR(st_dst.st_mode) || S_ISREG(st_dst.st_mode)) {
                        found_dst = true;
                }
                if (!found_src) {
                        if (VERBOSE) {
                                printfc(ERROR, "source not found: %s\n",
                                        entries[i].src);
                        }
                        return EXIT_FAILURE;
                }
                if (!found_dst) {
                        if (VERBOSE) {
                                printfc(ERROR, "destination not found: %s\n",
                                        entries[i].dst);
                        }
                        return EXIT_FAILURE;
                }
        }

        if (DRY_MODE == false) ensure_directory(entries[i].dst);

        if (DEBUG_MODE) {
                printfc(DEBUG, "Source: %s\n", entries[i].src);
                printfc(DEBUG, "Destination: %s\n", entries[i].dst);
                printfc(DEBUG, "Source File: %s\n", entries[i].file);
                printfc(DEBUG, "Destination File: %s\n", filepath);
        }

        if (REMOVE) {
                struct stat st_file;
                if (lstat(filepath, &st_file) == 0) {
                        if (!S_ISDIR(st_file.st_mode)) {
                                if (FORCE == false &&
                                    !S_ISLNK(st_file.st_mode)) {
                                        printf("Diff: %s x %s\n",
                                               entries[i].file, filepath);
                                        diff(entries[i].src, filepath);
                                        char choice;
                                        printf("Do you want to continue? (y/N): ");
                                        do {
                                                choice = fgetc(stdin);

                                                int c;
                                                while ((c = getchar()) !=
                                                               '\n' &&
                                                       c != EOF)
                                                        ;

                                        } while (choice != 'Y' &&
                                                 choice != 'y' &&
                                                 choice != 'N' &&
                                                 choice != 'n');

                                        if (choice == 'N' || choice == 'n')
                                                return EXIT_SUCCESS;
                                }

                                if (DRY_MODE) {
                                        printfc(INFO, "would remove %s\n",
                                                filepath);
                                } else {
                                        if (remove(filepath) != 0) {
                                                printfc(ERROR,
                                                        "failed to remove %s: %s\n",
                                                        filepath,
                                                        strerror(errno));
                                                return EXIT_FAILURE;
                                        } else
                                                operations++;
                                }
                        }
                        if (DELETE) return EXIT_SUCCESS;
                }
        }

        if (DRY_MODE) {
                printf("%s ==> %s\n", entries[i].file, entries[i].dst);
                return EXIT_SUCCESS;
        }

        if (symlink(entries[i].src, filepath) == 0) {
                if (VERBOSE) {
                        if (use_basename) {
                                printf("%s ==> %s\n", src_basename,
                                       entries[i].dst);
                        } else {
                                printf("%s ==> %s\n", entries[i].file,
                                       entries[i].dst);
                        }
                        operations++;
                }
        } else {
                printfc(ERROR, "%s ==> %s\n", strerror(errno), entries[i].file);
        }

        return EXIT_SUCCESS;
}
