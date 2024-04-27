#include "shell.h"
#include <linux/limits.h>

char *builtin_func_list[] = {
    "exit",
    "cd",
    "pwd",
    "echo",
    "export",
    "exec"
};
int (*builtin_func[])(char **, int, int) = {
    &builtin_exit,
    &builtin_cd,
    &builtin_pwd,
    &builtin_echo,
    &builtin_export,
    &builtin_exec
};
const int builtin_func_size = sizeof(builtin_func_list) / sizeof(char *);

/**
 * builtin_cd - changes the working dir of the current shell executon env
 * @args: target directory
 *
 * Return: 0 on success, 1 otherwise.
 */
int builtin_cd(char **args, int in, int out) {
    if (args[1] == NULL) {
        fprintf(stderr, "Error: No directory provided!\n");
        return EXIT_FAILURE;
    }
    if (chdir(args[1]) != 0) {
        perror("builtin_cd:chdir");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * builtin_exit - causes normal process termination
 * @args: empty
 *
 * Return: does not return anything, terminates shell.
 */
int builtin_exit(char **args, int in, int out) {
    // exit with status
    if (args[1])
        exit(atoi(args[1]));
    // exit success
    else
        exit(EXIT_SUCCESS);
}

/**
 * builtin_pwd - outputs the current working directory to stdout
 * @args: empty
 *
 * Return: 0 on success, 1 otherwise.
 */
int builtin_pwd(char **args, int in, int out) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("builtin_pwd:getcwd");
        return EXIT_FAILURE;
    }
    dprintf(out, "%s\n", cwd);
    return EXIT_SUCCESS;
}

/**
 * builtin_echo - outputs its arguments to stdout
 * @args: input strings
 *
 * Return: 0 on success, 1 otherwise.
 */
int builtin_echo(char **args, int in, int out) {
    // check if args is empty
    if (args[1] == NULL) {
        dprintf(out, "\n");
        return EXIT_SUCCESS;
    }

    int i;
    for (i = 1; args[i + 1] != NULL; i++) {
        dprintf(out, "%s ", args[i]);
    }
    dprintf(out, "%s\n", args[i]);
    return EXIT_SUCCESS;
}

/**
 * builtin_export - sets env variables of the shell
 * @args: variables and their values in "key=value" format
 *
 * Return: 0 on success, 1 otherwise.
 */
int builtin_export(char **args, int in, int out) {
    for (int i = 1; args[i] != NULL; i++) {
        char* key = strtok(args[i], "=");
        char* value = strtok(NULL, "=");

        if (key == NULL || value == NULL) {
            fprintf(stderr,
                "Error: invalid argument format. Expected key=value but got %s!\n",
                args[i]);
            continue;
        }

        if (setenv(key, value, 1) != 0) {
            perror("builtin_export:setenv");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * builtin_exec - replaces the shell into another command
 * @args: target command
 *
 * Return: nothing (replaces shell), 1 if no command provided, execvp errno otherwise.
 */
int builtin_exec(char **args, int in, int out) {
    // check if args is empty
    if (args[1] == NULL) {
        fprintf(stderr, "Error: No command provided!\n");
        return EXIT_FAILURE;
    }
    execvp(args[1], &args[1]);
    perror("builtin_exec:execvp");
    return errno;
}
