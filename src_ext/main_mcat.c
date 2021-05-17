#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define MCAT_N_MODE   1

typedef unsigned int mcat_opt_t;
int fd[2];

int mcat_print_file(const char *file, mcat_opt_t options);
int mcat_echo(mcat_opt_t options);

int main(int argc, char **argv) {
    if (argc == 1) {
        mcat_echo(0);
        // puts("mcat: not enough parameters.");
        exit(EXIT_SUCCESS);
    }

    char opt;
    mcat_opt_t option = 0;

    // opterr = 0;
    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
        case 'n':
            option |= MCAT_N_MODE;
            break;
        }
    }

    // check the optind
    if (optind >= argc)
    {
        mcat_echo(option);
        exit(EXIT_SUCCESS);
    }

    char *file_path = argv[optind];
    mcat_print_file(file_path, option);

    return 0;
}

int mcat_echo(mcat_opt_t options) {
    int  row_counter = 0;
    char buffer[BUFSIZ+1] = {0};
    while (!feof(stdin)) {
        int len_read = fread(buffer, 1, BUFSIZ, stdin);
        if (len_read < BUFSIZ) {
            buffer[len_read] = 0;
        }

        if (options & MCAT_N_MODE) {
            printf("%-6d  %s", row_counter, buffer);
            row_counter++;
        }
        else {
            printf("%s", buffer);
        }
    }

    return 0;
}


int mcat_print_file(const char *file, mcat_opt_t options) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        perror("mcat");
        return 1;
    }
    
    char buffer[BUFSIZ+1];
    int  row_counter = 0;
    while (!feof(fp)) {
        int len_read = fread(buffer, 1, BUFSIZ, fp);
        if (len_read < BUFSIZ) {
            buffer[len_read] = 0;
        }
        if (options & MCAT_N_MODE) {
            printf("%-4d %s", row_counter, buffer);
            row_counter++;
        }
        else {
            printf("%s", buffer);
        }
    }
    putchar('\n');
    return 0;
}