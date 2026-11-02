# Spring 246: reset y borrado diferido contrastados

Este bloque prepara la reconstrucción de Spring. **Todavía no añade Spring al
selector ni implementa su render de audio.** Se ejecutan el reset original
`0x01a32e9a` y únicamente la rama pendiente del callback mono `0x01a326e8`.

El estado ocupa 104 bytes: tag, trece palabras de historial, cuatro valores de
feedback, nueve cursores y slot. El workspace explícito comienza en `0x01001ea8`,
con stride de 58.272 bytes (29.136 muestras int16). Se prueban slots 0 y 1;
no se ejecuta el allocator original.

| Paso | Resultado observado |
| --- | --- |
| Reset con R0=self, R1=slot | Conserva tag y los trece historiales, pone a cero feedback y cursores, escribe slot. Borra las primeras 6.984 muestras y pone a 2 el contador de `0x018b6488 + slot×4`. |
| Primer callback pendiente | Borra las últimas 10.984 muestras y reduce el contador a 1. Conserva audio, estado y display. |
| Segundo callback pendiente | Vuelve a borrar exactamente las mismas últimas 10.984 muestras y reduce el contador a 0. Conserva audio, estado y display. |
| Memoria intermedia | Las 11.168 muestras intermedias se conservan durante toda esta secuencia. No se interpreta ese resultado como una política completa de arranque. |

Ocho secuencias usan estados y memorias sintéticos distintos: **8 resets,
16 callbacks pendientes y 24 capturas completas de ambos slots**, estados,
contadores y display. La segunda ejecución privada coincide byte a byte.
El ensayo compara todos los bytes de ambos workspaces, incluido el slot vecino;
no comprueba otras regiones de memoria. El contenido sintético no se presenta
como estado de arranque recuperado.

[Implementación acotada](../Source/Spring246Lifecycle.h),
[prueba](../Tests/Spring246LifecycleTests.cpp),
[evidencia](firmware/spring246-lifecycle-validation.json).
El script `scripts/verify-spring246-lifecycle.py --help` documenta los argumentos
para repetirlo con firmware aportado por el operador y una carpeta privada nueva.
No se distribuyen firmware, ELF, desensamblado ni pseudocódigo del fabricante.

## Contratos localizados para continuar

Spring tiene render mono en `0x01a326e8`, estéreo candidato en `0x01a32f94` y
un inicializador estático candidato en `0x01a33964`. El mono consulta las tablas
compartidas de frecuencia `0xff801f50` y paso bajo `0x0106a5ec`, además de valores
en `0x01de6248/0x01de624c`. Se han localizado escrituras de estos últimos dentro
del inicializador candidato; sus resultados aún no se ejecutan ni contrastan.

CWO tiene mono candidato en `0x0194480c` y estéreo en `0x019450fc`. El mono llama
al suavizado `0x0194435c`, a conversiones `0x01944528/0x019445a8/0x01944660` y a
otros auxiliares de filtro, interpolación y fase. Estas conversiones consultan
`0x01ba567c`, `0x01ba5a80` y `0x01ba6288`; aún hay que localizar y ejecutar sus
productores. Un decompilador ayuda a describir los contratos, pero no convierte
estos candidatos en audio validado ni prueba por sí solo un bloqueo por cifrado.

El siguiente bloque es ejecutar el inicializador de Spring, contrastar su render
activo y después integrarlo con controles, Value FX y persistencia. CWO necesita
su propio cierre de dependencias. Los presets originales siguen para el final.
