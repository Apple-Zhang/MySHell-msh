#include "msh_sig.h"

void msh_sigint_handler(int sig) {
    puts("\nmsh: keyboard interupt.");
    if (current_pid == msh_pid) {
        return;
    }
    else {
        exit(EXIT_FAILURE);
    }
}