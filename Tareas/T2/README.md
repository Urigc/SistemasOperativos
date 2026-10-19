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

## Descripción

Esta práctica es la evolución de la Tarea 1 (basada en procesos con fork()). El objetivo sigue siendo el mismo, dos flujos de ejecución independientes imprimen de forma concurrente la misma secuencia numérica (del 1 al 10,000) en un archivo de texto (salida_hilos.txt), con el fin de observar el intercalado producido por el planificador.

La diferencia fundamental es que ahora la concurrencia se implementa mediante Hilos POSIX (pthread), migrando de una arquitectura de procesos pesados a un modelo multihilo ligero.

## Funcionamiento del Código

El código (hilos_1.c) está estructurado bajo los estándares de la biblioteca pthread, que he decidido resumir en los siguientes puntos:

- Apertura del archivo compartido: Antes de crear los hilos, el hilo principal abre el archivo con open() utilizando la bandera O_APPEND. Al crear los hilos, estos heredan y comparten este descriptor.

- Creación de hilos (pthread_create): Se instancian dos hilos (hilo1 y hilo2). A cada uno se le pasa como argumento un identificador en texto ("HILO 1" y "HILO 2") para distinguir sus escrituras.

- Ejecución concurrente: Ambos hilos ejecutan la función imprimir_numeros(). Para evitar que el buffering de la librería estándar oculte el intercalado real, se utiliza la función dprintf() (que llama directamente a la llamada al sistema write()). Así, cada número se escribe de manera atómica e inmediata en el archivo.

- Sincronización (pthread_join): El hilo principal (el main) se bloquea a la espera de que ambos hilos terminen su ciclo utilizando pthread_join. Finalmente, cierra el archivo y termina.

## Comportamiento observado

Al inspeccionar el archivo salida_hilos.txt, se evidencia un intercalado no determinista (ráfagas de números de un hilo seguidas de ráfagas del otro).

Esto sucede porque, al ser hilos de un mismo proceso, comparten el espacio de memoria, pero compiten por el tiempo de CPU. 
El planificador del sistema operativo asigna time slices (rodajas de tiempo) a cada hilo, cuando la rodaja termina o el hilo se bloquea (por ejemplo, esperando escribir en el archivo), el planificador hace un cambio de contexto y le cede el turno al otro hilo. Dado que el orden exacto depende de la carga del sistema y del hardware, el resultado final nunca es perfectamente alternado, demostrando la naturaleza asíncrona de los hilos.

<details>
<summary> Ver capturas de pantalla</summary>
Capturas de pantalla

| | | |
|---|---|---|
| <img src="https://github.com/user-attachments/assets/7e6bb13b-e8d5-41f5-a168-83a3a7a34f75" alt="Ejemplo 1" width="250"/> | <img src="https://github.com/user-attachments/assets/8e73fd37-2855-4138-b379-f3c14f2b7b4b" alt="Ejemplo 2" width="250"/> |  <img src="https://github.com/user-attachments/assets/f9a4293f-213e-4389-a67f-d7cd1b2f16f7" alt="Ejemplo 3" width="250"/> | 

</details>

## Archivos

| Archivo | Descripción |
|---|---|
| `hilos.c` | Código fuente. |
| `salida_hilos.txt` | Evidencia generada tras la ejecución. |
| `README.md` | Este documento. |
