# FM 246: tablas, ciclo de voz y seis envolventes internas

Este bloque reconstruye dependencias del FM original. **El instrumento sigue en
0.26.0 y la opción FM disponible sigue siendo el motor propio de dos osciladores.**
No se añade un motor tocable ni se presenta esta base como render FM completo.
La coincidencia de dependencias tampoco demuestra equivalencia acústica.

## Rutinas y estado contrastados

| Componente | Entrada original | Evidencia |
| --- | --- | --- |
| Conversión inicial de tablas | `0x019a3de4` | 30 conversiones binary32 → Q15, con procedencia individual y sustituciones explícitas. |
| Copia al contexto compartido | `0x019a3d84` | Las 30 palabras se conservan en 8192 registros; guardas antes y después intactas. |
| Constructor de voz | `0x019a21d8` | Vtable, cuatro fases/muestras y dos grupos de seis campos; padding conservado. |
| Reset | `0x019a2248` | Fases/muestras y gains reiniciados; tag/padding conservados y seis ángulos a −32768. |
| Factoría | `0x019a3d38` | Seis llamadas válidas: pool completo de 360 bytes y puntero de retorno tras cada construcción, vecinos conservados. |
| Actualización interna | `0x019a22a4` | 8192 llamadas; 49152 gains y 49152 ángulos, estados completos de 60 bytes. |

El estado tiene cuatro subestados de ocho bytes (fase uint32, muestra int16 y
padding uint16), seis gains int16 y seis ángulos int16, después del tag de 32 bits.
El constructor deja los ángulos en cero; el reset los deja en −32768. Ambos puntos
producen gain inicial cero con el seno, pero difieren en la evolución posterior.
No se reduce el reset a una llamada al constructor.

La actualización usa, para cada uno de los seis canales:

- `gain = Q15(amount, sine[4096 + floor(angle/8)])`.
- `angle = wrap16(angle + Q15(rate, amount))`.

El callback de render llama a esta rutina con el cuadrado Q15 del control naranja.
Estos seis canales **no son la ADSR común por voz**. El ensayo aislado prueba además
amount y tasas con signo, extremos, ángulos arbitrarios y wrap. Las primeras 4096
entradas cargan estados independientes; las siguientes conservan secuencias con
recargas cada 256 llamadas y un reset intermedio. Las tablas por registro son
entradas sintéticas explícitas, no constantes originales recuperadas.

Coinciden todos los bytes de cada estado, las 30 palabras de tabla y ambas guardas.
La repetición en otro directorio privado produce archivos idénticos. AddressSanitizer
y UndefinedBehaviorSanitizer pasan. La compilación cruzada clang-cl con CRT/SDK de
Microsoft también pasa bajo Wine; el CI dedicado verifica las plataformas nativas.
[Evidencia](firmware/fm246-foundation-validation.json).

## Procedencia de las 30 constantes

Las fuentes están en `0xff906f70..0xff906fe4`, dentro del bloque CALLBACK de
`0xff906f10`. Bajo el perfil documentado de ocho bytes protegidos por grupo de 24,
**20 binary32 son legibles y 10 siguen protegidos**. El puntero de trabajo en
`0xff906f6c` está en una franja legible y vale `0xffb00168`, inmediatamente después
del pool de seis voces de 60 bytes. No se inventa ese puntero ni se identifica su
valor con una captura de todo el arranque.

Las seis primeras constantes alimentan la evolución interna y las siguientes
24 forman tres bancos de ocho entradas. El prefijo de render ya contrastado toma
sus índices de los bits 12–14, 9–11 y 6–8 del control verde. La interpretación
musical y el escalado completo de sus consumidores aún requieren el render.

[Modelo explícito](../research/blackfin/fm246-table-model.json): índices protegidos
`0,1,6,7,12,13,18,19,24,25`. Dos tasas moderadas y valores monótonos en los huecos
de los bancos son decisiones propias sin calibración acústica. **No se recuperan
esos valores originales**. El verificador rechaza cualquier cambio de las 20
palabras legibles; se comprueba su rechazo individual antes de simular.
El prefijo de 30 conversiones del oráculo está condicionado a ese modelo.
La aritmética de las envolventes, probada con entradas sintéticas independientes,
no necesita que el modelo coincida con los coeficientes desconocidos del aparato.
[Mapa de procedencia y rutas](firmware/fm246-table-provenance.json).

## Reproducción y formato

```sh
python3 scripts/verify-fm246-foundation.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-elf-as --linker /herramientas/bfin-elf-ld \
  --simulator /herramientas/run-parallel --testutils /herramientas/testsuite/bfin \
  --output /privado/fm-foundation-repetition
cmake -S research/fm-foundation -B build-fm -DCMAKE_BUILD_TYPE=Release
cmake --build build-fm --config Release --parallel 2
ctest --test-dir build-fm -C Release --output-on-failure
```

Se comprueban hash del LDR, mapa y microtest de ambos parches GNU antes del oráculo.
La salida debe ser nueva y externa al checkout; firmware, instrucciones, ELF,
desensamblado y logs privados no se distribuyen. `--record` actualiza solo vectores
numéricos sintéticos. El ejecutable independiente no necesita firmware, JUCE ni
instaladores. [Guía y formato de vectores](../research/fm-foundation/README.md).

## Siguiente cierre del FM

El callback `0x019a23d4` tiene nueve destinos legibles del selector. Este bloque
no reconstruye todavía sus bucles de audio ni confunde sus destinos con nueve
algoritmos musicalmente calibrados. Quedan la composición del render de cuatro
operadores, realimentación, memoria intermedia, preservación por bloque, pitch y
su comparación numérica completa. Después se integrarán reloj, voz/ADSR y controles
del host, identificando los adaptadores propios y manteniendo PDC/volumen de salida.

No se repite el bloqueo OTP como sustituto de esas tareas legibles. Los originales
completos, interpretación física de los huecos, asignación/cadencia y comparación
con hardware/Live siguen pendientes. Los presets originales completos quedan al
final. El avance global se mantiene en **48 % ±10 puntos** de planificación.
