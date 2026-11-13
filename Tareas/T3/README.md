# Tarea 4: Sincronización de Hilos con Semáforos POSIX

---
> **Institución:** INSTITUTO POLITÉCNICO NACIONAL

> **Unidad:** Escuela Superior de Cómputo  
> **Materia:** Sistemas Operativos
>
> | **Tema** | Concurrencia y Sincronización — Semáforos POSIX |
> 
> **Alumno:** Uriel González Casiano
> 
> **Profesor:** Rolando Quintero Téllez
> 
> **Grupo:** 4CM1
> 
> **Fecha de entrega:** 18 de Noviembre de 2026

---

## Descripción General

Esta tarea da continuidad a la problemática de concurrencia abordada en tareas anteriores (uso de procesos y luego hilos). En esta ocasión, el objetivo es implementar **Exclusión Mutua** para proteger una **Sección Crítica**.

El programa crea dos hilos POSIX que deben imprimir simultáneamente una secuencia de números del `1` al `10,000` en un archivo de texto. Sin embargo, se impone una regla de sincronización estricta: **los números comprendidos entre el `4000` y el `8000` no deben intercalarse**. Para lograr esto, se utiliza un semáforo binario que actúa como un "candado" que garantiza que solo un hilo a la vez pueda imprimir durante ese rango.

Adicionalmente, para facilitar la lectura de la evidencia, la salida en el archivo `.txt` se agrupa en bloques de 5 números por línea y se añaden "carteles" visuales que indican cuándo un hilo entra y sale de la sección protegida.

## Librerías Utilizadas

*   `<pthread.h>`: Biblioteca estándar POSIX para la creación y manipulación de hilos (`pthread_create`, `pthread_join`, `pthread_exit`).
*   `<semaphore.h>`: Biblioteca POSIX para la gestión de semáforos (`sem_t`, `sem_init`, `sem_wait`, `sem_post`, `sem_destroy`).
*   `<fcntl.h>` y `<unistd.h>`: Utilizadas para la manipulación de archivos a bajo nivel (`open`, `close`, `dprintf`), garantizando escrituras atómicas sin usar el *buffering* de la librería estándar.
*   `<stdio.h>` y `<stdlib.h>`: Para impresión en consola, manejo de errores (`perror`) y finalización del programa.

---

## Conceptos Aplicados

1.  **Hilos (Threads):** Flujos de ejecución ligeros dentro de un mismo proceso. A diferencia de los procesos, los hilos comparten el mismo espacio de memoria, lo que hace que la comunicación sea más rápida, pero requiere mecanismos explícitos para evitar colisiones en los datos (o en este caso, en los recursos como el archivo de texto).
2.  **Sección Crítica:** La porción de código donde se accede a un recurso compartido (la impresión de los números del 4000 al 8000). Si dos hilos entran aquí al mismo tiempo, los datos se corrompen (se intercalan).
3.  **Semáforo Binario:** Un mecanismo de sincronización que toma valores `0` o `1`.
    *   `sem_wait()`: Intenta tomar el candado. Si el valor es `1`, lo decrementa a `0` y continúa. Si es `0`, el hilo se duerme hasta que esté disponible.
    *   `sem_post()`: Libera el candado, incrementándolo a `1` y despertando a cualquier hilo que estuviera esperando.

---

## Explicación del Código

El código se divide lógicamente en tres grandes bloques:

### 1. Variables globales y preparación
```c
#define MAX 10000
#define LIMITE_INF 4000
#define LIMITE_SUP 8000
#define ARCHIVO "salida_semaforos.txt"

int fd;
sem_t sem_bloqueo;
```
*   Se definen las constantes para el rango total (`10000`) y los límites de la sección crítica (`4000` y `8000`).
*   `fd` es el descriptor del archivo compartido por ambos hilos.
*   `sem_bloqueo` es la variable global que representa el semáforo.

### 2. Función de los hilos (`imprimir_numeros`)
```c
void* imprimir_numeros(void* arg) {
    char *nombre = (char*)arg;
    int i;
    for (i = 1; i <= MAX; i++) {
        
        if (i == LIMITE_INF) {
            sem_wait(&sem_bloqueo);
            // [Carteles visuales de entrada...]
        }
        
        dprintf(fd, "%s: %d | ", nombre, i);
        if (i % 5 == 0) dprintf(fd, "\n");
        
        if (i == LIMITE_SUP) {
            // [Carteles visuales de salida...]
            sem_post(&sem_bloqueo);
        }
    }
    pthread_exit(NULL);
}
```
*   El bucle `for` corre del 1 al 10,000.
*   **Bloqueo:** Cuando el contador llega a `4000`, el hilo ejecuta `sem_wait`. Si el semáforo está libre, lo toma y entra a la sección crítica (imprimiendo un cartel visual). Si ya está tomado por el otro hilo, se queda dormido en esa línea.
*   **Impresión (Formato en bloque):** Se usa `dprintf` para escribir directo al archivo. La condición `if (i % 5 == 0)` añade un salto de línea cada 5 números para que el texto no sea una lista vertical infinita.
*   **Liberación:** Al llegar al `8000`, imprime un cartel de salida y ejecuta `sem_post`, liberando el candado para que el otro hilo pueda continuar.

### 3. Función Principal (`main`)
```c
int main() {
    pthread_t h1, h2;
    fd = open(ARCHIVO, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    sem_init(&sem_bloqueo, 0, 1);
    
    pthread_create(&h1, NULL, imprimir_numeros, (void*)"Hilo 1");
    pthread_create(&h2, NULL, imprimir_numeros, (void*)"Hilo 2");

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);

    sem_destroy(&sem_bloqueo);
    close(fd);
    return 0;
}
```
*   Se abre el archivo con `O_APPEND` (para que ambos hilos escriban al final sin sobreescribirse).
*   `sem_init(&sem_bloqueo, 0, 1)`: Inicializa el semáforo en `1` (abierto).
*   Se crean los dos hilos (`h1` y `h2`). Ambos ejecutan la misma función, pero se les pasa un nombre distinto por argumento para distinguir sus salidas.
*   `pthread_join`: El hilo principal se bloquea hasta que ambos hilos terminen, asegurando que el programa no cierre el archivo prematuramente.
*   Finalmente, se destruye el semáforo y se cierra el archivo.

---

## Evidencia Visual de la Salida

Gracias a la modificación del formato, el archivo `salida_semaforos.txt` demuestra claramente la exclusión mutua:

1.  **Del 1 al 3999:** Verás los números de ambos hilos intercalados, compitiendo por la CPU.
2.  **Al llegar al 4000:** Verás un cartel que dice `>>> ENTRANDO A SECCIÓN CRÍTICA: Hilo X`. A partir de ese instante, **todos los números serán de un solo hilo**, sin interrupción, demostrando que el semáforo bloqueó al otro.
3.  **Al llegar al 8000:** Verás el cartel `<<< SALIENDO DE SECCIÓN CRÍTICA: Hilo X`. Inmediatamente después, el otro hilo tomará el control (o continuará si ya estaba esperando) e imprimirá su propio bloque del 4000 al 8000.
4.  **Del 8001 al 10000:** El semáforo se libera y los números vuelven a intercalarse.

## Compilación y Ejecución

Para compilar el programa, es obligatorio enlazar la biblioteca de hilos y semáforos con el flag `-pthread`:

```bash
gcc -o tarea4_posix tarea4_posix.c -pthread
./tarea4_posix
```

Para inspeccionar la zona de conflicto (la sección crítica) en el archivo generado, puedes ejecutar:

```bash
grep -n "SECCIÓN CRÍTICA" salida_semaforos.txt
```
*(Este comando te mostrará exactamente en qué líneas se aplicó el candado y se demostró la exclusión mutua).*
