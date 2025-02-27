#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    //Verify arg count
    if (argc != 3) {
        return -1;
    }
    //Convert mode string to octal
    char *end;
    long perm = strtol(argv[2], &end, 8);
   
    //Validate permission value
    if (*end != '\0' || perm < 0 || perm > 0777) {
       return -1;
    }
    //Store original mask
    mode_t old_umask = umask(0);
    //Try creating file
    int file_desc = open(argv[1], O_CREAT | O_EXCL | O_WRONLY, (mode_t)perm);
    //Reset mask
    umask(old_umask);
    //Handle creation failure
    if (file_desc == -1) {
        return -1;
    }
    
    close(file_desc);
    return 0;

}


