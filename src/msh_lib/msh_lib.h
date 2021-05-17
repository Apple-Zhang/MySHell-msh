#ifndef _MYSHELL_LIB_H
#define _MYSHELL_LIB_H

#define MSH_TXT_COLOR(x, y) "\001\033["#x";"#y"m\002"
#define MSH_TXT_COLOR_END   "\001\033[0m\002"

#include "msh_basic.h"
#include "msh_bic.h"
#include "msh_sig.h"

// Builtin Command part interface
int msh_bic_call(const struct msh_cmd *cmd);

// call externel exe
int msh_externel_call(struct msh_cmd *cmd);

int msh_execute_command(struct msh_cmd *cmd);

// Set the username
int msh_set_username(const char *usr_name);

// Set the computer name
int msh_set_machine_name(const char *computer_name);

// loop of shell
int msh_main_loop();

#endif