#include "shell.h"

int main(int argc, char* argv[]) {
    char *line;
    char **tokens;
    struct command *cmds;
    int cmd_count;

    signal(SIGINT, SIG_IGN);

    while(1) {
        print_prompt();

        // parse input
        line = read_line();
        tokens = tokenize_line(line);
        cmd_count = split_commands(&cmds, tokens);

        // execute commands
        execute_cmds(cmds, cmd_count);

        // avoid memory leaks
        free(cmds);
        free(tokens);
        free(line);
    }

    return EXIT_SUCCESS;
}
