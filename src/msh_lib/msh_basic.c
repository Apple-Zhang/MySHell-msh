/* Myshell basic */

#include "msh_basic.h"

int   msh_exit_flag = 0;
char  msh_username[MSH_NAME_LEN] = "none";
char  msh_machine_name[MSH_NAME_LEN] = "localhost";
char  msh_process_path[MSH_BUFSIZE] = ".";
pid_t msh_pid = 0;
pid_t current_pid = -1;

void msh_get_process_path() {
    char *p;
    int n = readlink("/proc/self/exe", msh_process_path, MSH_BUFSIZE);

    if ((p = strrchr(msh_process_path, '/')) != NULL) {
        *p = '\0';
    }
    else {
        perror("msh");
        exit(EXIT_FAILURE);
    }
}

char *msh_read_line(char *reminder) {
    return readline(reminder);
}

void msh_free_cmd(struct msh_cmd *ptr) {
    struct msh_cmd *p = ptr;
    struct msh_cmd *temp;
    while (p) {
        for (int i = 0; i < p->cmd_n_par; i++) {
            free(p->cmd_paras[i]);
        }
        temp = p;
        p = p->cmd_next;
        free(temp);
    }
}

void msh_free_cmd_from_tail(struct msh_cmd *tail) {
    struct msh_cmd *p = tail;
    struct msh_cmd *temp;
    while (p) {
        for (int i = 0; i < p->cmd_n_par; i++) {
            free(p->cmd_paras[i]);
        }
        temp = p;
        p = p->cmd_prev;
        free(temp);
    } 
}

char *msh_current_folder() {
    char cwd[MSH_BUFSIZE] = {0};
    int  len = 0;
    int  idx;

    // obtain current work directory and length
    getcwd(cwd, MSH_BUFSIZE);
    idx = strlen(cwd);

    // When cwd is your home directory, then return "~".
    char myhome[MSH_BUFSIZE] = {0};
    sprintf(myhome, "/home/%s", msh_username);
    if (!strcmp(cwd, myhome)) {
        char *folder = (char *)malloc(sizeof("~"));
        strcpy(folder, "~");
        return folder;
    }

    // When cwd is root directory, then return "/".
    if (!strcmp(cwd, "/")) {
        char *folder = (char *)malloc(sizeof("/"));
        strcpy(folder, "/");
        return folder;
    }

    // find the character '/'.
    char *p;
    if ((p = strrchr(cwd, '/')) == NULL) {
        perror("msh");
    }

    char *folder = (char *)malloc(sizeof(char) * (strlen(p)));
    strcpy(folder, p + 1);

    return folder;
}

int msh_parse_cmd(char *cmd_subset, struct msh_cmd **cmd_ptr, struct msh_cmd **cmd_tail) {
    int idx = 0;
    char buffer[MSH_BUFSIZE];
    char *cptr = NULL;

    struct msh_cmd *prev = NULL;
    msh_next_t prev_type = NONE;

    strcpy(buffer, cmd_subset);
    cptr = strtok(buffer, " ");

    // allocating new command structure
    struct msh_cmd *cmd_parsed = (struct msh_cmd *)malloc(sizeof(struct msh_cmd));
    struct msh_cmd *cmd_p = cmd_parsed;

    while (cptr) {
        if (idx >= MSH_CMD_PAR_NUM) {
            msh_free_cmd(cmd_parsed);
            if (cmd_ptr) {
                *cmd_ptr = NULL;
            }
            if (cmd_tail) {
                *cmd_tail = NULL;
            }
            return MSH_RTN_FAILURE;
        }

        // check special sign: |, <, >, >>.
        switch (*cptr) {
            case '>': // redirect: write
                if (*(cptr+1) == '>') { // >>...
                    cptr++;
                    cmd_p->cmd_next_type = REDIRECT_WRA;
                }
                else { // >...
                    cmd_p->cmd_next_type = REDIRECT_WR;
                }
            goto DO_LIST_NODE;

            case '<': cmd_p->cmd_next_type = REDIRECT_RD; goto DO_LIST_NODE; // redirect: read
            case '|': cmd_p->cmd_next_type = PIPE;        goto DO_LIST_NODE; // pipe
            default: {
                // check if the string includes |, <, >:
                char *special = strpbrk(cptr, "|<>");
                if (special) {
                    int len = (int)(special - cptr);
                    cmd_p->cmd_paras[idx] = (char *)malloc(sizeof(char) * (len + 1));
                    strncpy(cmd_p->cmd_paras[idx], cptr, len);

                    // end string.
                    cmd_p->cmd_paras[idx][len] = '\0';

                    // update index, and cptr jump to next special sign.
                    idx++;
                    cptr = special;
                }
                else {
                    // malloc for command parameter
                    // "mls, mcp, mcat..." are our diy external commands, so redirect to their path.
                    int len = strlen(msh_process_path);
                    if (idx == 0 && !strcmp(cptr, "mls")) {
                        char buf[MSH_BUFSIZE+32] = {0};
                        sprintf(buf, "%s/../src_ext/mls", msh_process_path);
                        cmd_p->cmd_paras[idx] = (char *)malloc(sizeof(char) * (sizeof(buf)));
                        strcpy(cmd_p->cmd_paras[idx], buf);
                    }
                    else if (idx == 0 && !strcmp(cptr, "mcp")) {
                        char buf[MSH_BUFSIZE+32] = {0};
                        sprintf(buf, "%s/../src_ext/mcp", msh_process_path);
                        cmd_p->cmd_paras[idx] = (char *)malloc(sizeof(char) * (sizeof(buf)));
                        strcpy(cmd_p->cmd_paras[idx], buf);
                    }
                    else if (idx == 0 && !strcmp(cptr, "mcat")) {
                        char buf[MSH_BUFSIZE+32] = {0};
                        sprintf(buf, "%s/../src_ext/mcat", msh_process_path);
                        cmd_p->cmd_paras[idx] = (char *)malloc(sizeof(char) * (sizeof(buf)));
                        strcpy(cmd_p->cmd_paras[idx], buf);
                    }
                    else {
                        cmd_p->cmd_paras[idx] = (char *)malloc(sizeof(char) * (strlen(cptr) + 1));
                        strcpy(cmd_p->cmd_paras[idx], cptr);
                    }

                    // update index, and then find next space delimeter
                    idx++;
                    cptr = strtok(NULL, " ");
                }
                continue;
            } 
        }

        // executed only if *cptr is special sign.
        // |, <, > ... being met, denotes that current command is finished.
        // therefore, complete list node here, and create next command list node.
        DO_LIST_NODE: {
            // empty command or filename, error syntax.
            if (idx == 0) {
                fprintf(stderr, "msh: syntax error near unexpected token `newline`.");
                msh_free_cmd_from_tail(cmd_p);
                if (cmd_ptr) {
                    *cmd_ptr = NULL;
                }
                if (cmd_tail) {
                    *cmd_tail = NULL;
                }
                return MSH_RTN_FAILURE;
            }
            struct msh_cmd *next_cmd = (struct msh_cmd *)malloc(sizeof(struct msh_cmd));

            // complete current command
            cmd_p->cmd_n_par = idx;
            cmd_p->cmd_paras[idx] = NULL;
            cmd_p->cmd_next = next_cmd;
            cmd_p->cmd_prev = prev;
            cmd_p->cmd_prev_type = prev_type;

            // update prev pointer
            prev = cmd_p;
            prev_type = cmd_p->cmd_next_type;

            // move to next node
            cmd_p = cmd_p->cmd_next;
            idx = 0; // reset index to 0

            // the next character is '\0'
            // that is, next character is space in input string.
            if (*(cptr + 1) == '\0') {
                cptr = strtok(NULL, " "); // find next delimeter
            }
            else {
                // otherwise, go next character.
                cptr++;
            }
        }
    }

    // empty command or filename, error syntax
    if (idx == 0) {
        fprintf(stderr, "msh: syntax error near unexpected token `newline`.");
        msh_free_cmd_from_tail(cmd_p);
        if (cmd_ptr) {
            *cmd_ptr = NULL;
        }
        if (cmd_tail) {
            *cmd_tail = NULL;
        }
        return MSH_RTN_FAILURE;
    }

    // complete the structure
    cmd_p->cmd_n_par = idx;
    cmd_p->cmd_paras[idx] = NULL;
    cmd_p->cmd_next_type = NONE;
    cmd_p->cmd_next = NULL;
    cmd_p->cmd_prev = prev;
    cmd_p->cmd_prev_type = prev_type;

    // set the pointer of the structure
    if (cmd_ptr) {
        *cmd_ptr = cmd_parsed;
    }
    if (cmd_tail) {
        *cmd_tail = cmd_p;
    }

    /* DEBUG */
    // struct msh_cmd *p = cmd_p;
    // while (p) {
    //     puts("==========");
    //     for (int i = 0; i < p->cmd_n_par; i++) {
    //         printf("(%s)\n", p->cmd_paras[i]);
    //     }
    //     p = p->cmd_prev;
    // }

    return MSH_RTN_SUCCESS;
}
