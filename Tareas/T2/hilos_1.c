#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 10000
#define ARCHIVO "salida_hilos.txt"

int fd;

void *imprimir_numeros(void *arg) {
    char *nombre_hilo = (char *)arg;
    long i;

    for (i = 1; i <= MAX; i++) {
        dprintf(fd, "%s: %ld\n", nombre_hilo, i);
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t hilo1, hilo2;
    int error;


    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error cuando intentamos abrir el archivo");
        exit(-1);
    }

    error = pthread_create(&hilo1, NULL, imprimir_numeros, (void *)"HILO 1");
    if (error) {
        fprintf(stderr, "Error el creado del HILO 1 %d\n", error);
        exit(-1);
    }

    error = pthread_create(&hilo2, NULL, imprimir_numeros, (void *)"HILO 2");
    if (error) {
        fprintf(stderr, "Error el creado del HILO 2 %d\n", error);
        exit(-1);
    }

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);

    close(fd);

    printf("Recorrido del numeros terminado");
    return 0;
}
