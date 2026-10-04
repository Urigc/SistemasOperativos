#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX 10000
#define ARCHIVO "salida.txt"

int main(void)
{
    pid_t pid;
    long i;
    int fd;

    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }

    pid = fork();

    if (pid == -1) {
        perror("Error al crear el proceso");
        exit(-1);
    }
    else if (pid == 0) {
        for (i = 1; i <= MAX; i++)
            dprintf(fd, "%ld\n", i);
    }
    else {
        for (i = 1; i <= MAX; i++)
            dprintf(fd, "%ld\n", i);
    }

    close(fd);
    exit(0);
}