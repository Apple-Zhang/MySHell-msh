#ifndef _MYSHELL_SIG_H
#define _MYSHELL_SIG_H

#include <signal.h>
#include "msh_basic.h"
#include "msh_bic.h"

// Ctrl+C to send sigint
void msh_sigint_handler(int sig);

#endif