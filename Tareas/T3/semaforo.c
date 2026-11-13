#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX 10000
#define LIMITE_INF 4000
#define LIMITE_SUP 6000   
#define ARCHIVO "salida_semaforos.txt"

int fd;
sem_t sem_critico;

void* imprimir_numeros(void* arg) {
    char *nombre = (char*)arg;
    int i;
    int contador = 0;

    for (i = 1; i <= MAX; i++) {
        
        if (i >= LIMITE_INF && i <= LIMITE_SUP) {
            if (i == LIMITE_INF) {
                sem_wait(&sem_critico); 
            }
            
            dprintf(fd, "%s: %d | ", nombre, i);
            contador++;
            
            if (i == LIMITE_SUP) {
                sem_post(&sem_critico); 
            }
        } 
        
        else {
            if (sem_trywait(&sem_critico) == 0) {
                dprintf(fd, "%s: %d | ", nombre, i);
                sem_post(&sem_critico); 
                contador++;
            } else {

                sem_wait(&sem_critico); 
                dprintf(fd, "%s: %d | ", nombre, i);
                sem_post(&sem_critico);
                contador++;
            }
        }

        if (contador % 5 == 0) {
            dprintf(fd, "\n");
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t h1, h2;

    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error al abrir archivo");
        return 1;
    }

    sem_init(&sem_critico, 0, 1);

    if (pthread_create(&h1, NULL, imprimir_numeros, (void*)"hilo 1") != 0) return 1;
    if (pthread_create(&h2, NULL, imprimir_numeros, (void*)"hilo 2") != 0) return 1;

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);

    sem_destroy(&sem_critico);
    close(fd);

    return 0;
}