#ifndef _MYSHELL_BIC_H
#define _MYSHELL_BIC_H

#include "msh_lib.h"

#define MSH_BIC_SUCCESS  0
#define MSH_BIC_ERROR   -1
#define MSH_BIC_NOTBIC   1

// exit the shell
/* ============== MSH_BIC ==============
 * the builtin command for myshell 
 */

// exit the shell
int msh_bic_exit();

// change directory
int msh_bic_cd(const char *dir);

// help
int msh_bic_help();

// pwd
int msh_bic_pwd();

// echo
int msh_bic_echo(const struct msh_cmd *cmd);

#endif