
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int directWrite(const char* path, char* buffer)
{

    struct timeval begin;
    struct timeval end;

    int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);

    if (fd < 0)
    {
        perror("Open Failed\r\n:");
        return -1;
    }


    // Read
    gettimeofday(&(begin), NULL);
    write(fd, buffer, 4096);
    gettimeofday(&(end), NULL);
    close(fd);

    return end - begin;
}