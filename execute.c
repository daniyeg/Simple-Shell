#include "shell.h"

volatile sig_atomic_t interrupted = 0;

void sigint_handler(int sig) {
    interrupted = 1;
}

// forks a new process and replaces its input and output file descriptors.
// returns the new child's pid if successful, otherwise returns -1.
int spawn(int in, int out, char* argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        // replace stdin
        if (in != STDIN_FILENO) {
            dup2(in, STDIN_FILENO);
            close(in);
        }
        // replace stdout
        if (out != STDOUT_FILENO) {
            dup2(out, STDOUT_FILENO);
            close(out);
        }

        execvp(argv[0], argv);
        perror("spawn:execvp");
        exit(errno);
    }

    return pid;
}

// executes a command. returns exit status if successful otherwise retuns errno.
int execute(struct command cmd, int in, int out) {
    int status, exit_status;

    signal(SIGINT, sigint_handler); // set sigint to stop child 

    pid_t p = spawn(in, out, cmd.argv);
    if (p < 0) {
        perror("spawn:fork");
        return errno;
    }

    while(waitpid(p, &status, WNOHANG) == 0 && interrupted == 0);
    if (interrupted) {
        kill(p, SIGINT);
        waitpid(p, &status, 0);
    }
    if (WIFEXITED(status)) {
        fprintf(stderr, "%s exited with status %d\n",
            cmd.argv[0], WEXITSTATUS(status));
        exit_status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "%s terminated by signal %d\n",
            cmd.argv[0], WTERMSIG(status));
        exit_status = EXIT_FAILURE;
    }

    interrupted = 0;
    signal(SIGINT, SIG_IGN); // reset sigint to ignore

    return exit_status;
}

// executes a builtin function. returns exit status of the function.
int execute_builtin(struct command cmd, int builtin_index, int in, int out) {
    int status = (*builtin_func[builtin_index])(cmd.argv, in, out);
    fprintf(stderr, "%s exited with status %d\n", cmd.argv[0], status);
    return status;
}

// checks whether or not we should continue execution based on exit status
// of the previous command. returns 0 if we should continue, -1 otherwise.
int check_cmd(struct command *cmd, int status) {
    switch (cmd->op) {
    case CTRL_OR:
        // only continue if the last command failed
        if (status != EXIT_SUCCESS)
            return 0;
        break;
    case CTRL_PIPE:
    case CTRL_AND:
    case CTRL_NOP:
        // only continue if the last command didn't fail
        if (status == EXIT_SUCCESS)
            return 0;
        break;
    case CTRL_SEQ:
        // continue regardless of exit status
        return 0;
    default:
        fprintf(stderr, "Error: unrecognized control operator value!\n");
        break;
    }
    return -1;
}

// executes the commands in the given command array.
void execute_cmds(struct command *cmds, int cmd_count){
    int status;
    int in = STDIN_FILENO, out = STDOUT_FILENO, fd[2];

    for (int i = 0; i < cmd_count; i++) {
        // check to see if we need to open a pipe
        if (cmds[i].op == CTRL_PIPE) {
            if (pipe(fd) != 0) {
                perror("execute_cmds:pipe");
                return;
            }
            out = fd[1];
        }

        // check to see if there's a match with a builtin
        int j = 0;
        for (; j < builtin_func_size; j++) {
            if (strcmp(cmds[i].argv[0], builtin_func_list[j]) == 0)
                break;
        }
        if (j != builtin_func_size)
            status = execute_builtin(cmds[i], j, in, out);
        else
            status = execute(cmds[i], in, out);

        // handle pipes
        if (in != STDIN_FILENO) close(in);
        in = STDIN_FILENO;
        if (cmds[i].op == CTRL_PIPE) {
            close(fd[1]);
            in = fd[0];
            out = STDOUT_FILENO;
        }
        
        // decide whether or not we should continue execution
        if (check_cmd(&cmds[i], status) != 0)
            break;
    }

    return;
}
