#include "msh_bic.h"

int msh_bic_exit() {
    msh_exit_flag = 1;
    puts("exit msh");
    return MSH_BIC_SUCCESS;
}

int msh_bic_help() {
    puts("\n~~ Myshell (msh) version 0.1 ~~");
    puts("author: Junhong Zhang, SZU.");
    puts("email: zhangjunhong2018@szu.edu.cn");
    puts("type \"exit\" to exit msh.\n");
    puts("======= built-in commands =======");
    puts("exit: exit msh\npwd: peek current folder\ncd: change directory\necho: echo your input\nhelp: where we are now");
    puts("======= external commands =======");
    puts("mls: list of files\nmcat: peek file\nmcp: make a duplication of file\nAnd almost all bash external commands");
    puts("=======   more functions  =======");
    puts("use \"|\", \"<\", \">\" to make a pipe, and redirect your input\n");
    return MSH_BIC_SUCCESS;
}

int msh_bic_pwd() {
    static char pwd[MSH_BUFSIZE];

    // get current work directory
    getcwd(pwd, MSH_BUFSIZE);
    puts(pwd);
    
    return MSH_BIC_SUCCESS;
}

int msh_bic_cd(const char *dir) {
    if (dir == NULL) {
        char buf[MSH_BUFSIZE] = {0};

        sprintf(buf, "/home/%s", msh_username);
        if (chdir(buf) == -1) {
            perror("cd");
            return MSH_BIC_ERROR;
        }
        else {
            return MSH_BIC_SUCCESS;
        }
    }
    else if (chdir(dir) == -1) {
        perror("cd");
        return MSH_BIC_ERROR;
    }
    else {
        return MSH_BIC_SUCCESS;
    }
}

int msh_bic_echo(const struct msh_cmd *cmd) {
    if (cmd->cmd_paras[1] == NULL) {
        puts("");
        return MSH_BIC_SUCCESS;
    }
    else {
        for (int i = 1; i < cmd->cmd_n_par; i++) {
            printf("%s", cmd->cmd_paras[i]);
            if (i < cmd->cmd_n_par-1) {
                putchar(' ');
            }
        }
        putchar('\n');
        return MSH_BIC_SUCCESS;
    }
}