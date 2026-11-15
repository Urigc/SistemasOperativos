# Tarea 6: La Cena de los Filósofos con Hilos POSIX

> **Institución:** INSTITUTO POLITÉCNICO NACIONAL

> **Unidad:** Escuela Superior de Cómputo  
> **Materia:** Sistemas Operativos
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

Este proyecto implementa una solución concurrente al clásico problema de **La Cena de los Filósofos**, planteado originalmente por Edsger Dijkstra en 1965. El escenario consta de 5 filósofos sentados en una mesa circular, con 5 tenedores intercalados entre ellos. Para poder comer, un filósofo necesita **dos tenedores** (el de su izquierda y el de su derecha). El reto computacional radica en coordinar el acceso a estos recursos compartidos sin caer en **interbloqueos (deadlocks)** ni en **inanición (starvation)**.

A diferencia de las soluciones básicas (como la asimétrica), este programa implementa la **Solución de Estados (Máximo Paralelismo)**, que es matemáticamente la más óptima. En lugar de bloquear físicamente los tenedores, el sistema rastrea el estado (`THINKING`, `HUNGRY`, `EATING`) de cada filósofo y les permite comer solo si sus vecinos inmediatos no están comiendo, garantizando que hasta dos filósofos puedan comer simultáneamente sin conflictos.

## Librerías Utilizadas

*   `<pthread.h>`: Para la creación de los 5 hilos que representan a los filósofos y el uso de primitivas de exclusión mutua (`pthread_join`).
*   `<semaphore.h>`: Para la implementación de los semáforos POSIX (el semáforo global `mutex` y los semáforos individuales `s[i]`).
*   `<fcntl.h>` y `<unistd.h>`: Utilizadas para abrir el archivo de evidencia y permitir pausas en el procesador (`usleep`) que simulan el tiempo de pensar y comer.
*   `<stdio.h>` y `<stdlib.h>`: Para la impresión de mensajes y control de flujo.

---

## Conceptos Aplicados

La riqueza de este problema radica en que pone a prueba los tres pilares de la programación concurrente:

1.  **Exclusión Mutua:** El arreglo `state[N]` es un recurso compartido por todos los hilos. Para evitar que dos filósofos lean y modifiquen el estado al mismo tiempo generando datos corruptos, se utiliza un semáforo binario llamado `mutex`.
2.  **Sincronización por Condiciones (Semáforos individuales):** Cada filósofo tiene su propio semáforo (`s[i]`). Si un filósofo tiene hambre pero sus vecinos están comiendo, se queda dormido en su semáforo hasta que un vecino termine y lo despierte.
3.  **Prevención de Interbloqueo (Deadlock):** El interbloqueo ocurre si los 5 filósofos toman su tenedor izquierdo al mismo tiempo. Todos se quedan esperando el tenedor derecho que ya nadie soltará. Al usar el arreglo de estados, se rompe la espera circular: el filósofo solo "toma" los tenedores lógicamente si verifica que puede comer.
4.  **Prevención de Inanición (Starvation):** Garantiza que ningún filósofo se quede esperando eternamente. En el momento en que sus vecinos terminan de comer, la función `test()` se ejecuta y le cede el turno automáticamente.

---

## Análisis de la Solución Implementada

El material teórico propone dos soluciones:
1.  **Solución Asimétrica:** Hacer que el último filósofo tome primero el tenedor derecho. Es fácil de programar, pero ineficiente e injusta (sigue habiendo colisiones).
2.  **Solución de Estados (Implementada):** Es la solución óptima. Los tenedores físicos desaparecen como variables y se convierten en "estados". Al verificar si un filósofo puede comer revisando si `state[izquierda] != EATING` y `state[derecha] != EATING`, se maximiza el paralelismo (mientras el filósofo 1 come, el filósofo 3 puede comer sin problemas).

---

## Explicación del Código (Externa)

El código se divide en tres bloques lógicos que respetan el pseudocódigo proporcionado en las diapositivas:

### 1. Funciones de Control y Estado
```c
int left(int i) { return (i + N - 1) % N; }
int right(int i) { return (i + 1) % N; }
```
*   Estas funciones auxiliares calculan los índices de los vecinos de manera circular (usando el operador módulo). Para el filósofo `0`, su izquierda es el `4` y su derecha es el `1`.

```c
void test(int i) {
    if (state[i] == HUNGRY && state[left(i)] != EATING && state[right(i)] != EATING) {
        state[i] = EATING;
        sem_post(&s[i]);
    }
}
```
*   **`test(i)` es el corazón del algoritmo.** No bloquea al hilo. Simplemente revisa si el filósofo `i` tiene hambre y si **ninguno de sus dos vecinos está comiendo**. Si la condición se cumple, cambia el estado del filósofo a `EATING` y ejecuta un `sem_post` en su semáforo individual (lo despierta si estaba dormido esperando).

### 2. Adquisición y Liberación de Tenedores
```c
void take_forks(int i) {
    sem_wait(&mutex);
    state[i] = HUNGRY;
    test(i);
    sem_post(&mutex);
    sem_wait(&s[i]);
}
```
*   Cuando un filósofo tiene hambre, bloquea el `mutex` para modificar el arreglo `state` de forma segura. Se marca como `HUNGRY` y llama a `test(i)`. Si el `test` lo deja pasar (le hizo `sem_post`), el `sem_wait(&s[i])` final no lo bloqueará. Si sus vecinos están comiendo, el hilo se quedará dormido justo en esa última línea hasta que un vecino lo despierte.

```c
void put_forks(int i) {
    sem_wait(&mutex);
    state[i] = THINKING;
    test(left(i));
    test(right(i));
    sem_post(&mutex);
}
```
*   Al terminar de comer, el filósofo bloquea el `mutex`, se marca como `THINKING` y **llama a `test()` para sus vecinos**. Esto es vital: al soltar los tenedores, verifica si el vecino izquierdo o derecho estaban esperando (HUNGRY). Si es así, el `test` los cambiará a `EATING` y los despertará.

### 3. El Hilo del Filósofo y el `main`
*   Cada hilo ejecuta un bucle de 3 comidas (`MEALS = 3`). Piensa (`usleep`), llama a `take_forks`, come (`usleep`) y llama a `put_forks`. Al terminar sus 3 comidas, el hilo hace `pthread_exit`.
*   El `main` inicializa los semáforos (`mutex` en 1 y los `s[i]` en 0), crea los hilos y hace `pthread_join` para esperar a que todos terminen de cenar antes de cerrar el archivo y destruir los semáforos.

---

## Evidencia y Análisis de la Salida (`salida_filosofos.txt`)

El archivo de texto registrará cada cambio de estado. Al analizarlo, se pueden comprobar las propiedades del algoritmo:

1.  **Exclusión Vecinal:** Si ves el mensaje `Filosofo 0 comienza a comer (Tenedores 4 y 0)`, los siguientes mensajes de los filósofos `1` y `4` no podrán ser "comienza a comer", estarán obligados a ser "tiene hambre" (esperando).
2.  **Máximo Paralelismo:** Mientras el filósofo 0 come, verás que el filósofo 2 o el 3 pueden empezar a comer al mismo tiempo, ya que no comparten tenedores directos con el 0.
3.  **Flujo Continuo (Sin Deadlock):** El archivo mostrará que todos los filósofos progresan. Cada vez que uno termina de comer (`termina de comer y vuelve a pensar`), inmediatamente después se verá cómo un vecino que tenía hambre pasa a comer. El sistema nunca se congela.
