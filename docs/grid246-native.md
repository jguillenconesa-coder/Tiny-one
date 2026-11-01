# Grid 246 — dos planos y selector de efectos

En **set 3**, abre el selector **FX** y elige **Grid 246**. Los cuatro mandos
controlan **X Size, Y Size, Feedback y Mix**. Value 246 FX puede modular cualquiera
de esos colores con la ruta/rangos de tipo 1 ya contrastados. Los cinco parámetros
nuevos admiten automatización y controles MIDI contextuales del host, y se guardan
en proyectos, slots y LIFT/DROP. No se añaden presets originales.

El selector sustituye los cuatro botones de efectos anteriores. Incluye Off / Echo
+ Drive, Punch, Filter, Nitro Hybrid, Delay y Grid. Al elegir uno se actualizan los
interruptores de efecto existentes; siguen siendo automatizables por separado.
Configure sobre el selector apunta al interruptor seleccionado. Si el host activa
varios simultáneamente, la prioridad es **Grid → Delay → Nitro → Filter → Punch**.

## Qué se ha contrastado

El callback mono `0x019af338` usa **dos planos de 260 × 260 int16** (135.200
celdas en total), con escaneo de lectura, inyección y limpieza. Se conservan la
excitación ponderada por seno, reflexión de coordenadas, cuatro vecinos, orden de
saturación, negación saturada antes de sumar, feedback cuadrático y mezcla seca.
El mapa activo de cada eje depende de los controles; las celdas fuera de ese mapa
se limpian progresivamente. La pantalla es una ilustración propia de esa placa,
no una lectura de la memoria del DSP.

La tabla de seno procede de su fuente legible ya contrastada, `0x018b6688`; no se
inventan coeficientes para esta ruta. El reset original `0x019af27e` limpia los dos
planos del workspace fijo correspondiente al **slot 2**, reinicia los escaneos y
conserva tag/slot. El harness verifica ese reset sobre memoria retenida con datos,
además de estados sintéticos válidos. La reserva/binding del workspace sigue
siendo explícita; no se reconstruye el allocator del instrumento completo.

Coinciden **4.096 bloques / 262.144 muestras**, estado completo de 44 bytes,
display de 64 bytes por bloque y ocho instantáneas completas de ambos planos.
Los ocho segmentos de 512 bloques ejercitan dimensiones y extremos de feedback/
mix; cuatro ejecutan el reset original. Una repetición privada independiente
produce archivos idénticos. [Evidencia](firmware/grid246-validation.json).

## Adaptador y cambios de tamaño

El núcleo acotado exige cursores válidos para las dimensiones seleccionadas. Al
reducir bruscamente un eje, un cursor previo puede quedar fuera de ese dominio y
la ruta original puede leer fuera de la LUT aislada. El adaptador reinicia los
cursores si dejan de encajar y conserva la memoria; **esa política de transición
es propia**. No se afirma fidelidad del cambio abrupto de tamaño en el hardware.
Los controles se suavizan y se toman al completar cada bloque interno.

Se reutiliza el bus mono de Filter/Nitro/Delay tras sintetizador, cinta, entrada,
EQ y master. Resampling a 44,1 kHz, DC, guardia inicial, crossfade de activación y
ruteo son propios. No se añade otro retardo: siguen siendo 65/71/141 muestras a
44,1/48/96 kHz (~1,5 ms), incluso en bypass. El bypass apagado conserva el float
seco exacto retrasado; activar el bus usa la ruta int16 y su adaptación.

Mix regula tanto la excitación de la placa como el aporte seco; no es una mezcla
lineal convencional. La cola se declara potencialmente infinita. STOP/pánico y
CC120 limpian los planos y buffers; cambiar de efecto también reinicia su estado.
Las exportaciones crudas de cinta no imprimen este bus.

Se comprueban siete tasas, bypass, silencio/pánico, ausencia de falta de datos,
8.192 muestras nativas frente al núcleo, respuesta húmeda a impulso y reducción
brusca de ejes. También selector/Configure/proxies, proyectos/slots/clipboard y
renders iguales con buffers 64/257. El tiempo de CI compartido no certifica una
fecha límite física ni sustituye pruebas en Ableton real.

## Reproducción

```bash
python3 scripts/verify-grid246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/grid-nuevo
```

El directorio debe ser nuevo y externo al repositorio. Se conservan privados
firmware, ELF, desensamblado y pseudocódigo. Los fixtures publicados son entradas
sintéticas y resultados numéricos, no instrucciones ni una captura L1 descifrada.
Ruteo estéreo, clock original y comparación acústica con OP-1 siguen pendientes.
