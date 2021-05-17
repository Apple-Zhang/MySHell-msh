#ifndef _MYSHELL_BASIC_H
#define _MYSHELL_BASIC_H

#define MSH_BUFSIZE     512
#define MSH_NAME_LEN    64
#define MSH_CMD_PAR_NUM 32
#define MSH_CMD_PAR_LEN 32

#define MSH_RTN_SUCCESS 0
#define MSH_RTN_FAILURE -1
#define MSH_RTN_NOT_BIC 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// GNU readline
#include <readline/readline.h>
#include <readline/history.h>

/* ============== MSH BASIC DEFINITIONS ===============
 * 
 * 
 * 
 */

typedef enum msh_next_cmd_t {
    NONE = 0,
    PIPE = 1,
    REDIRECT_RD  = 2,
    REDIRECT_WR  = 3,
    REDIRECT_WRA = 4
} msh_next_t;

struct msh_cmd {
    char*           cmd_paras[MSH_CMD_PAR_NUM+1];
    struct msh_cmd* cmd_next;
    struct msh_cmd* cmd_prev;
    int             cmd_n_par;
    msh_next_t      cmd_next_type;
    msh_next_t      cmd_prev_type;
};

// the exit flag for shell loop
extern int   msh_exit_flag;
extern char  msh_username[MSH_NAME_LEN];
extern char  msh_machine_name[MSH_NAME_LEN];
extern char  msh_process_path[MSH_BUFSIZE];
extern pid_t msh_pid;
extern pid_t current_pid;


/* ============== MSH BASIC FUNCTIONS =============== */

// read a line
char *msh_read_line(char *cmd_line);

void msh_free_cmd(struct msh_cmd *cmd);

void msh_free_cmd_from_tail(struct msh_cmd *cmd_tail);

void msh_get_process_path();

// get current folder name
char *msh_current_folder();

// split the line
int msh_parse_cmd(char *cmd_line, struct msh_cmd **cmd_ptr, struct msh_cmd **cmd_tail);

#endif /* _MYSHELL_BSAIC_H */