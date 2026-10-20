# Programación con Hilos POSIX (pthreads)

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
> **Fecha de entrega:** 20 de Octubre de 2026
---

## EJERCICIO 1

## Descripción del Ejercicio

El programa demuestra la concurrencia entre un proceso principal y un hilo creado por este. Usando la biblioteca `pthread`, el flujo principal imprime una cuenta ascendente del **1 al 10,000**, mientras que el hilo secundario ejecuta simultáneamente una cuenta regresiva del **10,000 al 1**. 
Toda la salida se redirige a un archivo de texto (`salida_proceso_hilo.txt`) para evidenciar el intercalado temporal.

## Funcionamiento del Código

1. **Archivo compartido:** Se abre un descriptor de archivo con `open()` en el proceso principal antes de invocar a `pthread_create`. Debido a que los hilos comparten el mismo espacio de memoria, el nuevo hilo hereda este descriptor.
2. **Creación del hilo:** Se usa `pthread_create` para lanzar el hilo que ejecutará la función `imprimir_regresivo()`.
3. **Concurrencia real:** Mientras el hilo hace su cuenta regresiva, el proceso principal (hilo `main`) no se detiene; continúa con su bucle ascendente. Ambos compiten por el tiempo de CPU.
4. **Sincronización:** Se utiliza `pthread_join` para que el proceso principal espere obligatoriamente a que el hilo regresivo termine antes de cerrar el archivo y finalizar el programa.

## Comportamiento Observado

Dado que ambos flujos de ejecución (el proceso principal y el hilo) comparten el mismo espacio de direcciones pero compiten por los ciclos de CPU, el planificador del sistema operativo les asigna rodajas de tiempo (*time slices*).

Al revisar el archivo, se observará un **intercalado desordenado** de las dos secuencias. No veremos un bloque perfecto del 1 al 10,000 seguido del 10,000 al 1, ni una alternancia perfecta (ej. 1, 10000, 2, 9999). El orden dependerá de en qué momento exacto el sistema operativo haga el cambio de contexto, demostrando que los hilos se ejecutan de forma concurrente y asíncrona.

## Archivos

| Archivo | Descripción |
|---|---|
| `procesoehilo.c` | Código fuente. |
| `salida_proceso_hilo.txt` | Evidencia generada tras la ejecución. |
| `README.md` | Este documento. |

## EJERCICIO 2

## Descripción

Esta práctica es la evolución de la Tarea 1 (basada en procesos con fork()). El objetivo sigue siendo el mismo, dos flujos de ejecución independientes imprimen de forma concurrente la misma secuencia numérica (del 1 al 10,000) en un archivo de texto (salida_hilos.txt), con el fin de observar el intercalado producido por el planificador.

La diferencia fundamental es que ahora la concurrencia se implementa mediante Hilos POSIX (pthread), migrando de una arquitectura de procesos pesados a un modelo multihilo ligero.

## Funcionamiento del Código

El código (hilos_1.c) está estructurado bajo los estándares de la biblioteca pthread, que he decidido resumir en los siguientes puntos:

- **Apertura del archivo compartido:** Antes de crear los hilos, el hilo principal abre el archivo con open() utilizando la bandera O_APPEND. Al crear los hilos, estos heredan y comparten este descriptor.

- **Creación de hilos (pthread_create):** Se instancian dos hilos (hilo1 y hilo2). A cada uno se le pasa como argumento un identificador en texto ("HILO 1" y "HILO 2") para distinguir sus escrituras.

- **Ejecución concurrente:** Ambos hilos ejecutan la función imprimir_numeros(). Para evitar que el buffering de la librería estándar oculte el intercalado real, se utiliza la función dprintf() (que llama directamente a la llamada al sistema write()). Así, cada número se escribe de manera atómica e inmediata en el archivo.

- **Sincronización (pthread_join):** El hilo principal (el main) se bloquea a la espera de que ambos hilos terminen su ciclo utilizando pthread_join. Finalmente, cierra el archivo y termina.

## Comportamiento observado

Al inspeccionar el archivo salida_hilos.txt, se evidencia un intercalado no determinista (ráfagas de números de un hilo seguidas de ráfagas del otro).

Esto sucede porque, al ser hilos de un mismo proceso, comparten el espacio de memoria, pero compiten por el tiempo de CPU. 
El planificador del sistema operativo asigna time slices (rodajas de tiempo) a cada hilo, cuando la rodaja termina o el hilo se bloquea (por ejemplo, esperando escribir en el archivo), el planificador hace un cambio de contexto y le cede el turno al otro hilo. Dado que el orden exacto depende de la carga del sistema y del hardware, el resultado final nunca es perfectamente alternado, demostrando la naturaleza asíncrona de los hilos.

<details>
<summary> Ver capturas de pantalla</summary>
Capturas de pantalla

| | | |
|---|---|---|
| <img src="https://github.com/user-attachments/assets/627d72de-a659-4698-ae7c-21572330bb04" alt="Ejemplo 1" width="250"/> | <img src="https://github.com/user-attachments/assets/d44fc2ea-bcec-4a57-8ce3-a24ee937b6b3" alt="Ejemplo 2" width="250"/> |  <img src="https://github.com/user-attachments/assets/32074804-0dff-4391-8467-0b1f350056f2" alt="Ejemplo 3" width="250"/> | 

</details>

## Archivos

| Archivo | Descripción |
|---|---|
| `hilos_1.c` | Código fuente. |
| `salida_hilos.txt` | Evidencia generada tras la ejecución. |
| `README.md` | Este documento. |
