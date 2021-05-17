#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "msh_lib/msh_lib.h"

int main(int argc, char *argv[]) {
    if (argc > 1) {
        msh_set_username(argv[1]);
    }
    else {
        struct passwd *pwd;
        pwd = getpwuid(getuid());
        msh_set_username(pwd->pw_name);
    }

    // set the pid of msh
    msh_pid = getpid();
    current_pid = msh_pid;

    // set the path of msh process
    msh_get_process_path();
    printf("msh: start %s/msh\n", msh_process_path);

    // SIGINT handler
    signal(SIGINT, msh_sigint_handler);

    return msh_main_loop();
}