# Procesos padre e hijo con `fork()` en C

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
> **Fecha de entrega:** 06 de Octubre de 2026
---

## Descripción

Programa en C que crea un proceso hijo mediante `fork()` y hace que ambos procesos impriman la misma secuencia numérica (Para nuestro caso el esquema del 1 al 10000, aunque editable en número max) sobre un archivo de texto. El propósito es evidenciar cómo el planificador del sistema operativo alterna la CPU entre ambos procesos, produciendo una **intercalación no determinista** en la salida.

## Base del código

El codeado de referencia lo he tomado del libro:

> Márquez García, Francisco Manuel. *UNIX Programación Avanzada*, 3ª ed. Ra-Ma, 2014. Cap. 6, § 6.2, Programa 6.2 (`fork.c`, pág. 188).

Ese ejemplo original bifurca el flujo con `fork()` y hace que padre e hijo impriman mensajes concurrentes, lo cual es la base conceptual retomada aquí.

## Adaptaciones aplicadas

- Ambos procesos imprimen la **misma secuencia** (`1..N`), lo que hace visible la intercalación como ruptura de una misma serie.
- El archivo `salida.txt` se abre **antes del `fork()`**, de modo que padre e hijo comparten descriptor y offset.
- Se activa `O_APPEND` para que cada escritura se realice atómicamente al final, sin sobrescrituras.
  
El código completo está en esta misma parte del repositorio, en el archivo: `padre_hijo10000.c`.

## Comportamiento observado

La salida de `salida.txt` presenta **ráfagas alternas** de ambos procesos, aqui miestro unos ejemplos de la salida donde se ve el fenómeno:

<details>
<summary> Ver capturas de pantalla</summary>
## Capturas de pantalla

| |
|---|
| <img src="https://github.com/user-attachments/assets/7e6bb13b-e8d5-41f5-a168-83a3a7a34f75" alt="Ejemplo 1" width="800"/> |
| <img src="https://github.com/user-attachments/assets/8e73fd37-2855-4138-b379-f3c14f2b7b4b" alt="Ejemplo 2" width="800"/> | 
| <img src="https://github.com/user-attachments/assets/f9a4293f-213e-4389-a67f-d7cd1b2f16f7" alt="Ejemplo 3" width="800"/> | 

</details>

No se observa un patrón fijo (`1,1,2,2,...`). Esto se debe a que, tras el `fork()`, ambos procesos compiten por la CPU y el planificador les asigna "rodajas de tiempo". 
Al agotarse la rodaja o al bloquearse por E/S, el núcleo realiza un **cambio de contexto** y cede la CPU al otro proceso. El orden exacto depende de la carga del sistema, las prioridades dinámicas y la frecuencia del reloj, por lo que el resultado varia.

Para cerrar, la tarea confirma que `fork()` duplica el flujo de ejecución y que, a partir de ahí, ambos procesos son entidades independientes gestionadas por el planificador. La intercalación observada es la evidencia directa de la conmutación de contexto, y demuestra que en programación concurrente no se puede asumir un orden de ejecución especial sin mecanismos explícitos de sincronización.

## Archivos

| Archivo | Descripción |
|---|---|
| `padre_hijo10000.c` | Código fuente. |
| `salida.txt` | Evidencia generada tras la ejecución. |
| `README.md` | Este documento. |
