#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 100
#define TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)
#define ARCHIVO "salida_productor_consumidor.txt"

int fd;
int buffer[BUFFER_SIZE];
int front = 0;
int rear = 0;

sem_t empty;
sem_t full;
sem_t mutexP;
sem_t mutexC;

pthread_mutex_t count_mutex;
int total_consumed = 0;

void* productor(void* arg) {
    int id = *(int*)arg;
    int i;
    int item;

    for (i = 0; i < ITEMS_PER_PRODUCER; i++) {
        item = (id * 100) + i;

        sem_wait(&empty);
        sem_wait(&mutexP);

        buffer[rear] = item;
        dprintf(fd, "Productor %d depositó: %d en posicion %d\n", id, item, rear);
        rear = (rear + 1) % BUFFER_SIZE;

        sem_post(&mutexP);
        sem_post(&full);
        
        usleep(100000); 
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    int id = *(int*)arg;
    int item;

    while (1) {
        sem_wait(&full);
        sem_wait(&mutexC);

        pthread_mutex_lock(&count_mutex);
        if (total_consumed >= TOTAL_ITEMS) {
            pthread_mutex_unlock(&count_mutex);
            sem_post(&mutexC);
            sem_post(&full);
            break;
        }
        total_consumed++;
        pthread_mutex_unlock(&count_mutex);

        item = buffer[front];
        dprintf(fd, "Consumidor %d extrajo: %d de posicion %d\n", id, item, front);
        front = (front + 1) % BUFFER_SIZE;

        sem_post(&mutexC);
        sem_post(&empty);
        
        usleep(150000);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t h_prod[NUM_PRODUCERS];
    pthread_t h_cons[NUM_CONSUMERS];
    int id_prod[NUM_PRODUCERS] = {1, 2};
    int id_cons[NUM_CONSUMERS] = {1, 2};
    int i;

    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return 1;
    }

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mutexP, 0, 1);
    sem_init(&mutexC, 0, 1);
    pthread_mutex_init(&count_mutex, NULL);

    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&h_prod[i], NULL, productor, &id_prod[i]);
    }
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&h_cons[i], NULL, consumidor, &id_cons[i]);
    }

    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(h_prod[i], NULL);
    }
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(h_cons[i], NULL);
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutexP);
    sem_destroy(&mutexC);
    pthread_mutex_destroy(&count_mutex);
    close(fd);

    return 0;
}