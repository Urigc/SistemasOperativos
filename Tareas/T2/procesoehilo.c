#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 10000
#define ARCHIVO "salida_proceso_hilo.txt"

int fd;

void *imprimir_regresivo(void *arg) {
    int i;
    for (i = MAX; i >= 1; i--) {
        dprintf(fd, "Hilo: %d\n", i);
    }
    pthread_exit(NULL);
}

int main(void) {
    pthread_t hilo;
    int error;
    int i;

    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(-1);
    }

    error = pthread_create(&hilo, NULL, imprimir_regresivo, NULL);
    if (error) {
        fprintf(stderr, "Error en el creado de nuestro hilo %d\n", error);
        exit(-1);
    }


    for (i = 1; i <= MAX; i++) {
        dprintf(fd, "Proceso padre: %d\n", i);
    }

    pthread_join(hilo, NULL);

    close(fd);

    printf("Desglose del programa finalizado");
    return 0;
}