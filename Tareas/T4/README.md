# Múltiples Productores y Consumidores con Hilos POSIX

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

En esta tarea se implementa la generalización del clásico problema de **Productores-Consumidores** sobre un **Buffer Acotado (Bounded Buffer)**, migrando de un esquema básico 1:1 a un modelo M:N. En esta práctica, el programa instancia **dos hilos productores y dos hilos consumidores** que interactúan concurrentemente sobre un buffer circular de tamaño limitado (`N = 5`).

Para garantizar la integridad de los datos y evitar condiciones de carrera, se utilizan **semáforos POSIX contadores** (para el control de capacidad) y **semáforos binarios/mutex** (para la exclusión mutua). El programa tiene una condición de término programada: cada productor genera 100 productos (200 en total), por lo que el programa finaliza limpiamente cuando los consumidores han extraído los 200 elementos, dejando una evidencia detallada en un archivo de texto.

## Librerías Utilizadas

*   `<pthread.h>`: Para la creación y el manejo de los hilos (productores y consumidores), así como el uso del mutex de conteo (`pthread_mutex_t`).
*   `<semaphore.h>`: Para la implementación de los cuatro semáforos POSIX que coordinan la sincronización y exclusión mutua.
*   `<fcntl.h>` y `<unistd.h>`: Utilizadas para la manipulación del archivo de texto (`open`, `close`, `dprintf`), asegurando que las escrituras se realicen de manera atómica y directa al disco.
*   `<stdio.h>` y `<stdlib.h>`: Para la impresión de errores en consola y la gestión del flujo del programa.

---

## Conceptos Aplicados

1.  **Buffer Circular:** Un arreglo de tamaño fijo (`buffer[5]`) en el que los índices `front` (para extraer) y `rear` (para depositar) se desplazan usando la operación módulo (`% BUFFER_SIZE`). Al llegar al final del arreglo, los índices vuelven al inicio, reutilizando el espacio.
2.  **Semáforos Contadores (`empty` y `full`):** 
    *   `empty` (inicializado en 5): Cuenta los espacios vacíos disponibles. Los productores lo decrementan (`P`) antes de escribir y los consumidores lo incrementan (`V`) al leer.
    *   `full` (inicializado en 0): Cuenta los elementos disponibles para consumir. Los productores lo incrementan (`V`) al escribir y los consumidores lo decrementan (`P`) antes de leer.
3.  **Exclusión Mutua (`mutexP` y `mutexC`):** En un modelo con múltiples hilos, dos productores podrían intentar escribir en `buffer[rear]` al mismo tiempo, corrompiendo los datos. Los semáforos `mutexP` y `mutexC` (binarios, inicializados en 1) actúan como candados: aseguran que solo un productor (o consumidor) modifique el arreglo y los índices a la vez.
4.  **Terminación Controlada:** Dado que los hilos suelen correr en bucles infinitos (`while(1)`), se implementó un contador global protegido por un `pthread_mutex_t` (`count_mutex`) y una "píldora de veneno" para despertar a los consumidores y hacerles saber que la producción ha terminado, permitiendo que el programa cierre con éxito.

---

## Explicación del Código 

El código se divide en tres bloques lógicos principales:

### 1. Variables Globales y Sincronización
Se declaran los hilos, el buffer, los índices y los 4 semáforos. Además, se usa un `pthread_mutex_t count_mutex` exclusivamente para llevar la cuenta segura de cuántos productos se han consumido en total y saber cuándo detener el programa.

### 2. El Hilo Productor
```c
// (Lógica resumida)
for (i = 0; i < ITEMS_PER_PRODUCER; i++) {
    item = (id * 100) + i;          // Genera el dato
    sem_wait(&empty);               // P(empty): ¿Hay espacio? Si no, duerme.
    sem_wait(&mutexP);              // P(mutexP): Toma el candado de escritura.
    
    buffer[rear] = item;            // Sección Crítica: escribe en el buffer.
    dprintf(...);                   // Imprime la evidencia en el txt.
    rear = (rear + 1) % BUFFER_SIZE;// Avanza el índice circular.
    
    sem_post(&mutexP);              // V(mutexP): Suelta el candado de escritura.
    sem_post(&full);                // V(full): Avisa que hay un dato nuevo disponible.
}
```
*El orden de los semáforos es vital.* Primero se verifica que haya espacio (`empty`) y luego se bloquea el recurso (`mutexP`). Si se hiciera al revés, un productor podría quedarse dormido esperando espacio con el candado tomado, bloqueando a todos los demás.

### 3. El Hilo Consumidor
```c
// (Lógica resumida)
while (1) {
    sem_wait(&full);                // P(full): ¿Hay datos? Si no, duerme.
    sem_wait(&mutexC);              // P(mutexC): Toma el candado de lectura.
    
    // [Verificación de terminación con count_mutex]
    
    item = buffer[front];           // Sección Crítica: extrae del buffer.
    dprintf(...);                   // Imprime la evidencia en el txt.
    front = (front + 1) % BUFFER_SIZE;// Avanza el índice circular.
    
    sem_post(&mutexC);              // V(mutexC): Suelta el candado de lectura.
    sem_post(&empty);               // V(empty): Avisa que hay un espacio libre.
}
```
*El consumidor sigue la misma lógica de seguridad.* Bloquea el recurso (`mutexC`) solo cuando ya sabe que hay algo que leer (`full`), evitando así el bloqueo de otros consumidores mientras espera un dato que no existe.

### 4. Control de Terminación y `main`
La función `main` inicializa los semáforos (con los valores `N`, `0`, `1`, `1` respectivamente), crea los 2 productores y los 2 consumidores con `pthread_create`, y se queda esperando a que todos terminen con `pthread_join`. Al final, destruye los semáforos y cierra el archivo.

---

## Evidencia y Análisis de la Salida

Con la configuración establecida (`ITEMS_PER_PRODUCER = 100`), el archivo `salida_productor_consumidor.txt` contendrá **400 líneas** (200 depósitos y 200 extracciones). 

Al abrir el archivo, se pueden observar los siguientes comportamientos:

1.  **Concurrencia y Alternancia:** Verás que los mensajes de `Productor 1`, `Productor 2`, `Consumidor 1` y `Consumidor 2` se intercalan, demostrando que los 4 hilos están ejecutándose simultáneamente.
2.  **Naturaleza Circular:** Los índices (posición 0, 1, 2, 3, 4) se repiten constantemente. Cuando un productor escribe en la posición `4`, el siguiente en escribir lo hará en la posición `0`, demostrando que el buffer recicla su capacidad.
3.  **Bloqueos por Llenado (Buffer Full):** Si observas con detenimiento, en algún momento los productores dejan de imprimir temporalmente. Esto se debe a que llenaron los 5 espacios (`empty == 0`) y están esperando a que un consumidor extraiga algo para continuar.
4.  **Bloqueos por Vacío (Buffer Empty):** De manera inversa, los consumidores se pausan si el buffer se vacía (`full == 0`), esperando que los productores coloquen más datos.
5.  **Cierre Limpio:** El programa no se congela. El uso del mutex de conteo asegura que cuando el producto número 200 sea extraído, los consumidores salgan de su bucle y el programa principal termine correctamente.
