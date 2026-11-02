# Phone 246 — buffers segmentados y voz telefónica

Desde 0.24.1, el volumen maestro sigue al bus de FX y controla también sus colas;
[orden de salida y límites](output-routing.md). Las menciones a «master» como
etapa anterior al bus se refieren a drive/eco, no al volumen de salida.

En **set 3 → selector FX → Phone 246**, los mandos son **Tone, Phonic, Baud y
Telematic**. Tone determina el avance del remuestreador; Phonic agrupa cruces por
cero para formar segmentos, Baud controla sus repeticiones y Telematic mezcla la
señal procesada con la seca. Value 246 FX puede modular cualquiera de los cuatro
colores con los rangos de tipo 3 ya contrastados.

Los cinco parámetros nuevos admiten automatización y controles MIDI contextuales
del host, y se guardan en proyectos, slots y LIFT/DROP. El selector incluye Phone
sin cambiar las identidades anteriores. Ante interruptores simultáneos del host,
la prioridad es **Phone → Grid → Delay → Nitro → Filter → Punch**. No se añaden
presets originales.

## Núcleo contrastado

El callback mono `0x01a1bb74` realiza dos pasadas por cada bloque de 64 muestras:
primero captura la entrada y su envolvente/cruces, y después reproduce y filtra
segmentos. Se conservan los intercambios de tres buffers de 5.880 muestras,
longitudes, contadores, flags y cursor Q16, más un anillo de 32 muestras.

Se reconstruyen el detector de nivel, filtros pasaaltos/pasabajos, interpolación
de siete taps y 32 fases, saturaciones y redondeos Q15/Q31. La envolvente conserva
la peculiaridad observada con entrada `-32768`: no se reemplaza el valor absoluto
vectorial por una fórmula supuestamente mejor.

Las tablas se generan desde datos legibles usando los helpers originales:

- Frecuencia Q31: 129 valores desde `0x018be744` por `0x01aa6fd0`; Phone usa el primero.
- Pasabajos Q15: 129 valores desde `0x018be948` por `0x01aa7014`; Phone usa el índice 127 (`29808`).
- Interpolación Q15: los 225 valores compartidos con Delay, desde `0x018bebcc`.

Esto reproduce fragmentos de conversión concretos, no el arranque entero.
Coinciden **4.096 bloques / 262.144 muestras**, todos los estados de **140 bytes**,
displays de 64 bytes y ocho capturas completas del workspace. Las pruebas incluyen
extremos de mezcla, DC negativa sostenida, cambios de controles y el borde máximo
de cursor/longitud. Una repetición privada independiente produce archivos idénticos.
[Metadatos y hashes](firmware/phone246-validation.json).

## Reset y límites de memoria

El reset original `0x01a1c03e`, con slot 0 explícito, restablece los roles de los
buffers, longitudes a 128, anillo/cursor/fase y detector, y limpia **solo el buffer
de reproducción 2**. Conserva la historia de los filtros y los otros dos buffers.
Ese comportamiento se contrasta sobre memoria retenida; no se confunde con pánico.

Con longitud máxima, el callback puede escribir un centinela en la primera celda
posterior al stride lógico de 17.640 muestras. El ensayo y el adaptador reservan
**dos celdas de guarda adicionales** (17.642 int16), sin asumir el allocator ni
la disposición entre slots del instrumento. Se comprueban esas celdas con valores
centinela explícitos. El puerto acotado acepta roles, cursores y longitudes válidos;
no se declara recuperada una captura L1 ni el protocolo de asignación original.

Al activar/cambiar el efecto o recibir STOP/pánico/CC120, el adaptador limpia toda
su memoria y estados para garantizar silencio. Ese reset completo del host es
propio y más amplio que el soft reset original contrastado.

## Integración y comprobaciones

El bus mono compartido sigue a sintetizador, cinta, entrada, EQ y master. Sus
tasas, DC de 15 Hz, guardia de silencio inicial y crossfade de activación son
adaptadores propios. La latencia fija del bus no aumenta: **65/71/141 muestras**
a 44,1/48/96 kHz (~1,5 ms), también en bypass. La espera/repetición de segmentos es
parte del efecto musical, no otra compensación PDC.

El efecto puede conservar/repetir segmentos con nivel residual; se declara cola
potencialmente infinita. El bypass apagado conserva el float seco exacto retrasado.
Activar el bus usa la ruta int16 y su conversión; las pistas crudas de cinta
exportadas no imprimen el efecto.

Se prueban siete tasas, bypass y falta de datos, silencio/pánico, 8.192 muestras
nativas frente al núcleo y salida solo procesada. Los extremos de Tone/Phonic/
Baud se automatizan durante el audio. El plugin comprueba selector/Configure,
proxies con editor cerrado, guardado/slots/clipboard, prioridad, bloques 64/257
y limpieza de buffers con CC120. Los tiempos del entorno compartido no certifican
cero dropouts físicos. Ableton real, acústica del hardware, ruteo estéreo y clock
completo siguen pendientes.

## Reproducción privada

```bash
python3 scripts/verify-phone246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/phone-nuevo
```

El directorio debe ser nuevo y externo al repositorio. Firmware, ELF,
desensamblado y pseudocódigo quedan privados. Los fixtures son entradas sintéticas
y resultados numéricos, no instrucciones ni memoria descifrada del instrumento.
