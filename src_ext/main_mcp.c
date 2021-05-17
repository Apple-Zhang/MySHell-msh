#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MCP_PATH_MAX  512
#define MCP_BUFSIZE   2048

int isdir(const char *filename);
int copy_f2f(const char *src, const char *dst);

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        copy_f2f(argv[1], argv[2]);
    }
    else if (argc == 2) {
        char buffer[MCP_PATH_MAX+1];
        sprintf(buffer, "%s.bk", argv[1]);
        copy_f2f(argv[1], buffer);
    }

    return 0;
}

int isdir(const char *filename) {
    struct stat fileInfo;
    if (stat(filename, &fileInfo) >= 0) {
        return S_ISDIR(fileInfo.st_mode);
    }
    return 0;
}

int copy_f2f(const char *src, const char *dst) {
    FILE *fin_p, *fout_p;
    char dst_name[MCP_BUFSIZE+1] = {0};
    const char *dst_filename_p = dst;

    if (isdir(dst)) {
        int index = strlen(src)-1;
        while (index >= 0 && src[index] != '/') {
            index--;
        }

        // dstname = dst + "/" (if is not root dir) + src_file_name
        strcpy(dst_name, dst);
        if (strcmp(dst_name, "/")) {
            strcat(dst_name, "/");
        }
        strcat(dst_name, src + index + 1);
        
        dst_filename_p = dst_name;
    }

    if (!strcmp(src, dst_filename_p)) {
        return 0;
    }

    if ((fin_p = fopen(src, "rb")) == NULL) {
        perror("src file error");
        return -1;
    }

    if ((fout_p = fopen(dst_filename_p, "wb")) == NULL) {
        perror("dst file error");
        fclose(fin_p);
        return -1;
    }

    char buffer[MCP_BUFSIZE+1] = {0};
    int  counter;
    if (counter = fread(buffer, sizeof(char), MCP_BUFSIZE, fin_p)) {
        if (ferror(fin_p)) {
            perror("read src file error");
            fclose(fin_p);
            fclose(fout_p);
            return -1;
        }
        else if (feof(fin_p)) {
            // empty file
            fclose(fin_p);
            fclose(fout_p);
            return 0;
        }
    }

    // copy to dst
    while (!feof(fin_p)) {
        fwrite(buffer, sizeof(char), counter, fout_p);
        if (counter = fread(buffer, sizeof(char), MCP_BUFSIZE, fin_p)) {
            if (ferror(fin_p)) {
                perror("read src file error");
                fclose(fin_p);
                fclose(fout_p);
                return -1;
            }
        }

        if (counter < MCP_BUFSIZE) {
            // reach end of file
            fwrite(buffer, sizeof(char), counter, fout_p);
            break;
        }
    }

    // remember close the files
    fclose(fin_p);
    fclose(fout_p);

    return 0;
}