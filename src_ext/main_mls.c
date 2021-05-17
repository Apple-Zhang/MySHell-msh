#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define L_MODE   1
#define A_MODE   2
#define R_MODE   4
#define H_MODE   8
#define R_MODE_  16

#define MLS_PARDIR_MAX  8
#define MLS_PATH_MAX    512

#define MLS_TXT_COLOR(x, y) "\001\033["#x";"#y"m\002"
#define MLS_TXT_COLOR_END   "\001\033[0m\002"

// the mode of output
static int  mls_mode = 0;
// the path when you enter this program.
static char begin_path[MLS_PATH_MAX+1];

// queue to cache the input directories
typedef struct mls_queue {
    char *dnames[MLS_PARDIR_MAX];
    int   qlen;
} mqueue_t;

int  do_mls(const char *dirname, const char *parent_dir);
int  show_file_stat(const char *fname, struct stat *info);

int  read_arg(char **argv, int argc, mqueue_t *mqueue);

void free_mls_queue(mqueue_t *p);
int  deal_mls_queue(mqueue_t *queue_of_dirs, const char *parent_dir);
int  append_mls_queue(mqueue_t *mqueue, const char *s);
char *get_file_mode(int mode_code, char *s);

// ID transform to name
char *uid2name(uid_t uid);
char *gid2name(uid_t gid);

int main(int argc, char *argv[]) {
    getcwd(begin_path, MLS_PATH_MAX);   
    if (argc == 1) {
        puts(".:");
        do_mls(".", begin_path);
        return 0;
    }

    mqueue_t queue_of_dirs;
    read_arg(argv, argc, &queue_of_dirs);
    if (!queue_of_dirs.qlen) {
        puts(".:");
        do_mls(".", begin_path);
    }
    else if (queue_of_dirs.qlen == 1) {
        printf("%s:\n", queue_of_dirs.dnames[0]);
        do_mls(queue_of_dirs.dnames[0], begin_path);
    }
    else {
        deal_mls_queue(&queue_of_dirs, begin_path);
    }

    // free the memory of malloc resources.
    free_mls_queue(&queue_of_dirs);
    return 0;
}

int do_mls(const char *dirname, const char *parent_dir) {
    struct dirent *p;
    struct stat info;
    mqueue_t mqueue;
    DIR *dir_p;

    // initialize the queue
    mqueue.qlen = 0;
    
    // open the directory
    if ((dir_p = opendir(dirname)) == NULL) {
        fprintf(stderr, "mls: cannot open directory %s.\n", dirname);
        return -1;
    }

    // if using -l parameter, then list out the detail stat
    chdir(dirname);
    if (mls_mode & L_MODE) {
        while (p = readdir(dir_p)) {
            // if no mls -a, dont show the file start with ".".
            if (!(mls_mode & A_MODE) && p->d_name[0] == '.') {
                continue;
            }
            
            stat(p->d_name, &info);
            show_file_stat(p->d_name, &info);

            // recurse
            if (S_ISDIR(info.st_mode)) {
                if (strcmp(p->d_name, "..") != 0 && 
                    strcmp(p->d_name, ".")  != 0 && 
                    (mls_mode & R_MODE_)) {
                    char buf[MLS_PATH_MAX] = {0};
                    sprintf(buf, "%s/%s", dirname, p->d_name);
                    append_mls_queue(&mqueue, buf);
                }
            }
        }
        puts("\n");
    }
    else {
        int output_counter = 0;
        while (p = readdir(dir_p)) {
            // if no mls -a, dont show the file start with ".".
            if (!(mls_mode & A_MODE) && p->d_name[0] == '.') {
                continue;
            }

            // get detail stat info of this object.
            stat(p->d_name, &info);
            
            // newline when output 8 elements.
            if (output_counter >= 8) {
                output_counter = 0;
                putchar('\n');
            }
            
            if (S_ISDIR(info.st_mode)) {
                // directory shown with blue.
                printf(MLS_TXT_COLOR(49, 34)"%s"MLS_TXT_COLOR_END"\t", p->d_name);

                // if using recursive option, then append this directory to the queue.
                if (strcmp(p->d_name, "..") != 0 && 
                    strcmp(p->d_name, ".")  != 0 && 
                    (mls_mode & R_MODE_)) {
                    char buf[MLS_PATH_MAX] = {0};
                    sprintf(buf, "%s/%s", dirname, p->d_name);
                    append_mls_queue(&mqueue, buf);
                }
            }
            else {
                printf("%s\t", p->d_name);
            }
        }
        puts("\n");
        // puts("======");
    }

    chdir(parent_dir);
    closedir(dir_p);

    if (mqueue.qlen) {
        deal_mls_queue(&mqueue, parent_dir);
        free_mls_queue(&mqueue);
    }
    
    return 0;
}

int show_file_stat(const char *fname, struct stat *info) {
    char s_mode[11];

    // output the stat information of file
    printf("%s ", get_file_mode(info->st_mode, s_mode)); // file mode
    printf("%4d ",(int)info->st_nlink);                  // number of link
    printf("%-8s ", uid2name(info->st_uid));             // username
    printf("%-8s ", gid2name(info->st_gid));             // group name
    printf("%8ld ", (long)info->st_size);                // size of the file
    printf("%.12s ", 4 + ctime(&info->st_mtime));        // time of modified time

    if (S_ISDIR(info->st_mode)) {
        // directory shown with blue.
        printf(MLS_TXT_COLOR(49, 34)"%s"MLS_TXT_COLOR_END"\n", fname);
    }
    else {
        printf("%s\n", fname);
    }

    return 0;
}

char *uid2name(uid_t uid)
{
    struct passwd *pw_ptr;
    static char numstr[10];

    if ((pw_ptr = getpwuid(uid)) == NULL) {
        sprintf(numstr, "%d", uid);
        return numstr;
    }
    else {
        return pw_ptr->pw_name;
    }
}

char *gid2name(gid_t gid)
{
    struct group *grp_ptr;
    static char numstr[10];

    if ((grp_ptr = getgrgid(gid)) == NULL) {
        sprintf(numstr, "%d", gid);
        return numstr;
    }
    else {
        return grp_ptr->gr_name;
    }
}

int deal_mls_queue(mqueue_t *queue_of_dir, const char *parent_dir) {
    int ret = 0;
    for (int i = 0; i < queue_of_dir->qlen; i++) {
        // output directory.
        printf("%s:\n", queue_of_dir->dnames[i]);
        ret = do_mls(queue_of_dir->dnames[i], parent_dir);
    }
    return ret;
}

int append_mls_queue(mqueue_t *mqueue, const char *s) {
    if (mqueue->qlen >= MLS_PARDIR_MAX) {
        fprintf(stderr, "mls: only show info of first %d dirrectories.\n", MLS_PARDIR_MAX);
        return 1;
    }

    int fname_len = strlen(s);
    int i = mqueue->qlen;

    // add directory.
    mqueue->dnames[i] = (char *)malloc(sizeof(char) * (fname_len+1));
    strcpy(mqueue->dnames[i], s);

    mqueue->qlen++;
    return 0;
}

void free_mls_queue(mqueue_t *p) {
    for (int i = 0; i < p->qlen; i++) {
        free(p->dnames[i]);
    }
}

int read_arg(char **argv, int argc, mqueue_t *queue_of_dirs) {
    int ret = 0;
    // set the length of dir
    queue_of_dirs->qlen = 0;

    char opt;
    while ((opt = getopt(argc, argv, "laR")) != -1) {
        switch (opt) {
            case 'l': mls_mode |= L_MODE;  break;
            case 'a': mls_mode |= A_MODE;  break;
            case 'R': mls_mode |= R_MODE_; break;
            return -1;
        }
    }

    for (int i = optind; i < argc; i++) {
        ret = append_mls_queue(queue_of_dirs, argv[i]);
    }
    
    /* abandoned */
    // // parse each argument
    // while (argc--) {
    //     char *arg_p = *argv;

    //     // mls output mode option 
    //     if (*arg_p == '-') {
    //         char opt = *(arg_p+1);
    //         switch (*(arg_p+1)) {
    //             case 'l': mls_mode |= L_MODE;  break;
    //             case 'a': mls_mode |= A_MODE;  break;
    //             case 'R': mls_mode |= R_MODE_; break;
    //             default:
    //                 fprintf(stderr, "mls: unknown option \'%c\'\n", opt);
    //                 return -1;
    //         }
    //     }
    //     // normal directory, put them into the queue
    //     else {
    //         ret = append_mls_queue(queue_of_dirs, arg_p);
    //     }
    //     argv++;
    // }
    return ret;
}

char *get_file_mode(int code, char *str) {
    strcpy(str, "-----------");

    if (S_ISDIR(code))    str[0] = 'd';
    if (S_ISCHR(code))    str[0] = 'c';
    if (S_ISBLK(code))    str[0] = 'b';

    if (code & S_IRUSR)   str[1] = 'r';
    if (code & S_IWUSR)   str[2] = 'w';
    if (code & S_IXUSR)   str[3] = 'x';

    if (code & S_IRGRP)   str[4] = 'r';
    if (code & S_IWGRP)   str[5] = 'w';
    if (code & S_IXGRP)   str[6] = 'x';

    if (code & S_IROTH)   str[7] = 'r';
    if (code & S_IWOTH)   str[8] = 'w';
    if (code & S_IXOTH)   str[9] = 'x';
    return str;
}