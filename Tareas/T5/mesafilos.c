#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define MEALS 3
#define ARCHIVO "salida_filosofos.txt"

int state[N];
sem_t s[N];
sem_t mutex;
int fd;

int left(int i) { return (i + N - 1) % N; }
int right(int i) { return (i + 1) % N; }

void test(int i) {
    if (state[i] == HUNGRY && state[left(i)] != EATING && state[right(i)] != EATING) {
        state[i] = EATING;
        dprintf(fd, "Filosofo %d comienza a comer (Tenedores %d y %d).\n", i, left(i), i);
        sem_post(&s[i]);
    }
}

void take_forks(int i) {
    sem_wait(&mutex);
    state[i] = HUNGRY;
    dprintf(fd, "Filosofo %d tiene hambre.\n", i);
    test(i);
    sem_post(&mutex);
    sem_wait(&s[i]);
}

void put_forks(int i) {
    sem_wait(&mutex);
    state[i] = THINKING;
    dprintf(fd, "Filosofo %d termina de comer y vuelve a pensar.\n", i);
    test(left(i));
    test(right(i));
    sem_post(&mutex);
}

void* philosopher(void* arg) {
    int i = *(int*)arg;
    int j;
    for (j = 0; j < MEALS; j++) {
        dprintf(fd, "Filosofo %d esta pensando.\n", i);
        usleep(500000);
        take_forks(i);
        usleep(500000);
        put_forks(i);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[N];
    int ids[N];
    int i;

    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error al abrir archivo");
        return 1;
    }

    sem_init(&mutex, 0, 1);
    for (i = 0; i < N; i++) {
        sem_init(&s[i], 0, 0);
        state[i] = THINKING;
        ids[i] = i;
    }

    for (i = 0; i < N; i++) {
        pthread_create(&hilos[i], NULL, philosopher, &ids[i]);
    }

    for (i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    for (i = 0; i < N; i++) {
        sem_destroy(&s[i]);
    }
    sem_destroy(&mutex);
    close(fd);

    return 0;
}