#include "msh_lib.h"

int msh_set_username(const char *usr_name) {
    // int name_len = strlen(usr_name);
    // int cp_len = (name_len) < MSH_NAME_LEN ? name_len : MSH_NAME_LEN;
    // strncpy(msh_username, usr_name, cp_len);
    strcpy(msh_username, usr_name);
    return MSH_RTN_SUCCESS;
}

int msh_set_machine_name(const char *computer_name) {
    // int name_len = strlen(computer_name);
    // int cp_len = (name_len) < MSH_NAME_LEN ? name_len : MSH_NAME_LEN;
    // strncpy(msh_machine_name, computer_name, cp_len);
    strcpy(msh_username, computer_name);
    return MSH_RTN_SUCCESS;  
}

int msh_bic_call(const struct msh_cmd *cmd) {
    const char *cmd_name = cmd->cmd_paras[0];

    if (!strcmp("exit", cmd_name)) {
        return msh_bic_exit();
    }
    else
    if (!strcmp("cd", cmd_name)) {
        return msh_bic_cd(cmd->cmd_paras[1]);
    }
    else
    if (!strcmp("pwd", cmd_name)) {
        return msh_bic_pwd();
    }
    else
    if (!strcmp("help", cmd_name) || 
        !strcmp("?", cmd_name)) {
        return msh_bic_help();
    }
    else
    if (!strcmp("echo", cmd_name)) {
        return msh_bic_echo(cmd);
    }
    else {
        return MSH_BIC_NOTBIC;
    }
}

int msh_externel_call(struct msh_cmd *cmd) {
    struct msh_cmd *p = cmd;
    char *cmd_name = p->cmd_paras[0];

    if (!p) {
        return MSH_RTN_FAILURE;
    }

    if (msh_pid != getppid()) {
        // check bic
        switch (msh_bic_call(cmd)) {
            case MSH_BIC_SUCCESS: exit(EXIT_SUCCESS);
            case MSH_BIC_NOTBIC: break;
            case MSH_BIC_ERROR:
            default: {
                puts("msh: builtin command execution failure."); // TODO
                exit(EXIT_FAILURE);
            }
        }
    }

    if (p->cmd_prev) {
        if (p->cmd_prev_type == PIPE) {
            // file descriptor for pipe
            int fd[2];
            pipe(fd);

            pid_t next_pid = fork();
            if (next_pid < 0) {
                fprintf(stderr, "msh: error in execuring command. Process [%d] is closed.\n", getpid());
                exit(EXIT_FAILURE);
            }
            // child process play as "writing process".
            else if (next_pid == 0) {
                dup2(fd[1], STDOUT_FILENO); // redirect stdout to write end of pipe and send data
                close(fd[0]);
                close(fd[1]);

                msh_externel_call(p->cmd_prev); // recurese
            }
            else {
                dup2(fd[0], STDIN_FILENO); // redirect stdin to read end of pipe and receive the data
                close(fd[0]);   // unable read end of pipe and read from stdin.
                close(fd[1]);   // unable write end

                wait(NULL);
                execvp(cmd_name, p->cmd_paras);
                perror("msh");
                msh_free_cmd_from_tail(cmd);
                exit(EXIT_FAILURE);
            }   
        }
        else if (p->cmd_prev_type == REDIRECT_WR) {
            pid_t next_pid = fork();
            if (next_pid < 0) {
                fprintf(stderr, "msh: error in execuring command. Process [%d] is closed.\n", getpid());
                exit(EXIT_FAILURE);
            }
            else if (next_pid == 0) {
                freopen(p->cmd_paras[0], "w", stdout);
                msh_externel_call(p->cmd_prev);
            }
            else {
                wait(NULL);
                exit(EXIT_SUCCESS);
            }
        }
        else if (p->cmd_prev_type == REDIRECT_WRA) {
            pid_t next_pid = fork();
            if (next_pid < 0) {
                fprintf(stderr, "msh: error in execuring command. Process [%d] is closed.\n", getpid());
                exit(EXIT_FAILURE);
            }
            else if (next_pid == 0) {
                FILE *fp;
                fp = fopen(p->cmd_paras[0], "a+");
                freopen(p->cmd_paras[0], "a", stdout);
                msh_externel_call(p->cmd_prev);
            }
            else {
                wait(NULL);
                exit(EXIT_SUCCESS);
            }
        }
        else if (p->cmd_prev_type == REDIRECT_RD) { 
            pid_t next_pid = fork();
            if (next_pid < 0) {
                fprintf(stderr, "msh: error in execuring command. Process [%d] is closed.\n", getpid());
                exit(EXIT_FAILURE);
            }
            else if (next_pid == 0) {
                FILE *fp;
                fp = fopen(p->cmd_paras[0], "r+");
                freopen(p->cmd_paras[0], "r", stdin);
                msh_externel_call(p->cmd_prev);
            }
            else {
                wait(NULL);
                exit(EXIT_SUCCESS);
            }
        }
    }
    else {
        execvp(cmd_name, p->cmd_paras);
        perror("msh");
        msh_free_cmd_from_tail(cmd);
        exit(EXIT_FAILURE);
    }
}

int msh_execute_command(struct msh_cmd *cmd) {
    switch (msh_bic_call(cmd)) {
        case MSH_BIC_SUCCESS: break;
        case MSH_BIC_NOTBIC:  {
            // create a new process to execute the externel commands
            pid_t pid = fork();
            if (pid < 0) {
                return MSH_RTN_FAILURE;
            }
            else if (pid == 0) {
                current_pid = getpid();
                return msh_externel_call(cmd);
            }
            else {
                pid_t cpid = wait(NULL);
                return MSH_RTN_SUCCESS;
            }
            break;
        }
        case MSH_BIC_ERROR:
        default: {
            puts("msh: builtin command execution failure."); // bic error
        }
    }

    return MSH_RTN_SUCCESS;
}

int msh_main_loop() {
    char prompt[MSH_BUFSIZE] = {0};
    while (!msh_exit_flag) {
        char *cwd = msh_current_folder();
        char *buffer;

        // set the prompt
        sprintf(prompt, "(msh) ["MSH_TXT_COLOR(1, 32)"%s@%s "MSH_TXT_COLOR(49, 34)"%s"MSH_TXT_COLOR_END"]$ ", 
                msh_username, 
                msh_machine_name, 
                cwd);
        buffer = msh_read_line(prompt);

        // Add the command to history
        add_history(buffer);

        // get command line from keyboard.
        if (!buffer) {
            // input EOF, exit.
            buffer = (char *)malloc(sizeof("exit"));
            strcpy(buffer, "exit");
        }

        // if you input nothing, then we 
        if (!strcmp(buffer, "")) {
            continue;
        }

        // struct msh_cmd *cmd;
        struct msh_cmd *cmd_tail;
        msh_parse_cmd(buffer, NULL, &cmd_tail);

        // free the temporary buffers
        free(buffer);
        free(cwd);

        msh_execute_command(cmd_tail);
        msh_free_cmd_from_tail(cmd_tail);
    }

    return EXIT_SUCCESS;
}