// 1. Stat: Write your own version of the command line program stat, which simply calls
// the stat() system call on a given file or directory. Print out file size, number of blocks
// allocated, reference (link) count, file permissions, and file inode. 

#include <stdio.h>     // fprintf, perror
#include <sys/stat.h>
#include <unistd.h> //syscall
#include <stdlib.h>    // exit, EXIT_FAILURE, EXIT_SUCCESS
int main(int argc, char **argv){
    struct stat dirstat;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(stat(argv[1],&dirstat) == -1){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    printf("%o\n",dirstat.st_mode);
    printf("%o\n",S_IFREG);
    printf("%o\n",S_IFMT);
    printf("%o\n",dirstat.st_mode & S_IFMT);
    printf("File type:                ");

    switch (dirstat.st_mode & S_IFMT) {
        case S_IFBLK:
            printf("block device\n");
            break;
        case S_IFCHR:
            printf("character device\n");
            break;
        case S_IFDIR:
            printf("directory\n");
            break;
        case S_IFIFO:
            printf("FIFO/pipe\n");
            break;
        case S_IFLNK:
            printf("symbolic link\n");
            break;
        case S_IFREG:
            printf("regular file\n");
            break;
        case S_IFSOCK:
            printf("socket\n");
            break;
        default:
            printf("unknown?");
            break;
    }
    printf("I-node number:            %ld\n", (long) dirstat.st_ino);
    printf("Mode:                     %lo (octal)\n", (unsigned long) dirstat.st_mode);
    printf("Link count:               %ld\n", (long) dirstat.st_nlink);
    printf("Ownership:                UID=%ld    GID=%ld\n", (long) dirstat.st_uid, (long) dirstat.st_gid);
    printf("Preferred I/O block size: %ld bytes\n", (long) dirstat.st_blksize);
    printf("File size:                %lld bytes\n", (long long) dirstat.st_size);
    printf("Blocks allocated:         %lld\n", (long long) dirstat.st_blocks);
    printf("Last status change:       %s", ctime(&dirstat.st_ctime));
    printf("Last file access:         %s", ctime(&dirstat.st_atime));
    printf("Last file modification:   %s", ctime(&dirstat.st_mtime));

    exit(EXIT_SUCCESS);
}