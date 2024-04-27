#include "shell.h"
#include <linux/limits.h>

const char *control_operator_list[] = {
    "|",
    "||",
    "&&",
    ";"
};

// functions similarly to strtok in that it returns the next token in s based on delim
// but ignores delims in blocks. each block is detected by its first character
// in the open_block string and its last character by the corresponding char in close_block.
char *block_strtok(char *s, char *delim, char *open_block, char *close_block) {
    static char *token = NULL;
    char *lead = NULL, *block = NULL;
    int in_block = 0;
    int block_char_index = 0;
    int escaped = 0;

    if (s != NULL) {
        token = s;
        lead = s;
    } else {
        lead = token;
        if (*token == '\0')
            lead = NULL;
    }

    while (*token != '\0') {
        if (escaped) {
            escaped = 0;
            token++;
            continue;
        }
        if (*token == '\\') {
            escaped = 1;
            token++;
            continue;
        }
        if (in_block) {
            if (close_block[block_char_index] == *token)
                in_block = 0;

            token++;
            continue;
        }
        if ((block = strchr(open_block, *token)) != NULL) {
            in_block = 1;
            block_char_index = block - open_block;

            token++;
            continue;
        }

        if (strchr(delim, *token) != NULL) {
            *token = '\0';
            token++;
            break;
        }
        token++;
    }

    return lead;
}

// reads a line from stdin. returns line on success otherwise terminates the program.
char *read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        free(line);
        // check for ctrl + d
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("read_line:getline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

// tokenize a line based on TOKEN_DELIM (see shell.h).
// WARNING: this functions changes the original line by replacing the delimiters
// and string indicators with \0.
char **tokenize_line(char *line) {
    int bufsize = 16;
    char *token;
    char **tokens = (char**) malloc(bufsize * sizeof(char *));
    if (!tokens) {
        perror("tokenize_line:malloc");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    token = block_strtok(line, TOK_DELIM, STRING_DELIM, STRING_DELIM);
    while (token != NULL) {
        // handle comments
        if (token[0] == '#') {
            break;
        }
        // handle strings
        if (strlen(token) >= 2 &&
            strchr(STRING_DELIM, token[0]) &&
            strchr(STRING_DELIM, token[strlen(token) -1])) {
            token++;
            token[strlen(token) -1] = '\0';
        }
        tokens[i] = token;
        i++;
    
        // check if there are more tokens than we have buffer for
        if (i >= bufsize) {
            bufsize += bufsize;
            tokens = (char**) realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                perror("tokenize_line:realloc");
                exit(EXIT_FAILURE);
            }
        }
        token = block_strtok(NULL, TOK_DELIM, STRING_DELIM, STRING_DELIM);
    }
    tokens[i] = NULL;
    return tokens;
}

// split tokens into commands. returns command size.
// WARNING: this function changes the original token stream
// by replacing control operator tokens with NULL.
int split_commands(struct command **buf, char **tokens) {
    int bufsize = 8;
    *buf = (struct command*) malloc(bufsize * sizeof(struct command));
    if (!*buf) {
        perror("split_commands:malloc");
        exit(EXIT_FAILURE);
    }

    int split_start = 0;
    int token_count = 0;
    int cmd_count = 0;
    while (tokens[token_count] != NULL) {
        long unsigned int i = 0;
        for (; i < sizeof(control_operator_list) / sizeof(char *); i++) {
            if (strcmp(tokens[token_count], control_operator_list[i]) != 0)
                continue;

            // create a new split
            tokens[token_count] = NULL;
            (*buf)[cmd_count].argc = token_count - split_start;
            (*buf)[cmd_count].argv = &tokens[split_start];
            (*buf)[cmd_count].op= (enum control_ops) i;

            split_start = token_count + 1;
            cmd_count++;
            // check if there are more commands than we have buffer for
            if (cmd_count > bufsize) {
                bufsize += bufsize;
                *buf = (struct command*) realloc(*buf, bufsize * sizeof(struct command));
                if (!*buf) {
                    perror("split_commands:realloc");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }

        token_count++;
    }

    (*buf)[cmd_count].argc = token_count - split_start;
    (*buf)[cmd_count].argv = &tokens[split_start];
    (*buf)[cmd_count].op= CTRL_NOP;
    cmd_count++;

    return cmd_count;
}

// prints the promp.
void print_prompt() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        printf("$ ");
    } else
        printf("%s$ ", cwd);
}
