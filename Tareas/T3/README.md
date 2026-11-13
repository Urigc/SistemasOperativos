# Tarea 4: Sincronización de Hilos con Semáforos POSIX

---
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

Este proyecto es la evolución directa de las prácticas anteriores de concurrencia. El objetivo es implementar **Exclusión Mutua** para controlar el acceso a un recurso compartido (el archivo de texto) mediante el uso de **semáforos POSIX**.

El programa instancia dos hilos que compiten por imprimir una secuencia del `1` al `10,000`. La regla de negocio exige que, cuando un hilo llegue al número `4000`, este **tome el control absoluto del recurso** y lo retenga hasta llegar al `6000`. Si el otro hilo intenta imprimir en ese lapso, se congela automáticamente en el número exacto en el que se encuentre, sin importar que esté en la zona "libre" (por ejemplo, en el número 3871), garantizando que la sección crítica quede estrictamente protegida.

## Librerías Utilizadas

*   `<pthread.h>`: Para la creación y manipulación de hilos (`pthread_create`, `pthread_join`).
*   `<semaphore.h>`: Para la gestión del semáforo (`sem_t`, `sem_init`, `sem_wait`, `sem_trywait`, `sem_post`).
*   `<fcntl.h>` y `<unistd.h>`: Para la manipulación del archivo a bajo nivel (`open`, `close`, `dprintf`), evitando el *buffering* para que el orden de las líneas sea real.
*   `<stdio.h>` y `<stdlib.h>`: Para control de errores y flujo estándar.

---

## Conceptos Aplicados

1.  **Sección Crítica:** El bloque de código (y en este caso, el rango de números del `4000` al `6000`) donde el recurso compartido no debe ser modificado por más de un hilo a la vez.
2.  **Exclusión Mutua:** La garantía de que solo un hilo ejecutará la sección crítica en un momento dado.
3.  **Semáforo Binario:** Un candado lógico que toma valores de 0 (bloqueado) o 1 (libre).
4.  **`sem_trywait()` vs `sem_wait()`:**
    *   `sem_wait()`: Bloquea al hilo si el semáforo está en 0, durmiéndolo hasta que se libere.
    *   `sem_trywait()`: Es una verificación "no bloqueante". Si el semáforo está en 0, no duerme al hilo, pero le devuelve un código de error indicando que el recurso está ocupado. Esto es vital para la zona libre: si el hilo detecta que el otro ya entró al 4000, se frena de inmediato.

---

## Explicación del Código

### 1. Configuración y Variables Globales
```c
#define LIMITE_INF 4000
#define LIMITE_SUP 6000
int fd;
sem_t sem_critico;
```
*   Se definen los nuevos límites de la sección crítica (`4000` a `6000`).
*   `fd` es el descriptor del archivo.
*   `sem_critico` es el semáforo que actuará como candado. Se inicializa en `1` en el `main` con `sem_init`.

### 2. Lógica de los Hilos (`imprimir_numeros`)
```c
if (i >= LIMITE_INF && i <= LIMITE_SUP) {
    if (i == LIMITE_INF) {
        sem_wait(&sem_critico); // Toma el control absoluto
    }
    dprintf(fd, "%s: %d | ", nombre, i);
    if (i == LIMITE_SUP) {
        sem_post(&sem_critico); // Libera el control
    }
} 
```
*   **Dentro del rango (4000-6000):** El primer hilo que llega al 4000 ejecuta `sem_wait`, poniendo el candado en 0. Imprime de corrido hasta el 6000. Al llegar, ejecuta `sem_post`, liberando el recurso.

```c
else {
    if (sem_trywait(&sem_critico) == 0) {
        dprintf(fd, "%s: %d | ", nombre, i);
        sem_post(&sem_critico);
    } else {
        sem_wait(&sem_critico); // Se congela aquí
        dprintf(fd, "%s: %d | ", nombre, i);
        sem_post(&sem_critico);
    }
}
```
*   **Fuera del rango (1-3999 y 6001-10000):** El hilo intenta avanzar con `sem_trywait`.
    *   Si el candado está libre (`== 0`), imprime su número y libera al instante.
    *   Si el candado está tomado (el otro hilo ya entró al 4000), el `trywait` falla. El código cae en el `else` y ejecuta un `sem_wait`, **congelando al hilo exactamente en el número donde iba**.

### 3. Formato de salida
```c
if (contador % 5 == 0) {
    dprintf(fd, "\n");
}
```
*   Lleva un contador local para añadir un salto de línea cada 5 números, mejorando la legibilidad del archivo y evitando 20,000 líneas individuales.

---


## Evidencia y Análisis de la Sincronización

Dado que se han retirado los textos de aviso visuales, la sincronización se demuestra matemáticamente en el archivo `salida_semaforos.txt`. 

Para comprobar el correcto funcionamiento del semáforo, abra el archivo `salida_semaforos.txt` y diríjase al siguiente bloque:

*   **Inicio de la congelación:** En la línea `1367`, se puede observar cómo los hilos venían intercalándose normalmente (por ejemplo, en el número `3987`). De repente, un hilo desaparece de la salida.
*   **Desarrollo de la sección crítica:** Desde la línea `1371` hasta la línea `1771`, se observa un bloque masivo y continuo de **un solo hilo** imprimiendo desde el `4000` hasta el `6000`. No hay ni un solo número del otro hilo en este tramo.
*   **Fin de la sección y reanudación:** En la línea `1771`, el primer hilo termina de imprimir el `6000`. Inmediatamente después, en la línea `1772`, el segundo hilo "despierta" y retoma la impresión exactamente desde el número donde se congeló (ej. el `3872`), demostrando que estuvo esperando en la puerta durante toda la sección crítica.

---
