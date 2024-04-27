#ifndef SHELL_H_INCLUDED
#define SHELL_H_INCLUDED

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define TOK_DELIM " \t\r\n\a"
#define STRING_DELIM "\'\""

enum control_ops {
    CTRL_PIPE = 0, // |
    CTRL_OR,       // ||
    CTRL_AND,      // &&
    CTRL_SEQ,      // ;
    CTRL_NOP       // last command
};

struct command {
    int argc;
    char **argv;
    enum control_ops op;
};

// input.c
extern const char *control_operator_list[];

char *read_line(void);
char **tokenize_line(char *line);
int split_commands(struct command **buf, char **tokens);
void print_prompt();


// execute.c
void execute_cmds(struct command *cmds, int cmd_count);


// builtin.c
extern char *builtin_func_list[];
extern int (*builtin_func[])(char **, int, int);
extern const int builtin_func_size;

int builtin_cd(char **args, int in, int out);
int builtin_exit(char **args, int in, int out);
int builtin_pwd(char **args, int in, int out);
int builtin_echo(char **args, int in, int out);
int builtin_export(char **args, int in, int out);
int builtin_exec(char **args, int in, int out);


#endif /* SHELL_H_INCLUDED */
