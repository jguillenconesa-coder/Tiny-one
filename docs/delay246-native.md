# Delay 246 — memoria circular y siete taps

Desde 0.23, elige este efecto en el **selector FX de set 3**; sustituye los botones anteriores.

En **set 3**, activa **DELAY 246**. Los mandos son **Size, Speed, Feedback y Mix**.
Size selecciona seis tamaños; Speed cambia la velocidad de recorrido y el tono de
las repeticiones. Mix controla el envío al eco: el callback conserva la señal seca
y suma el retorno, en lugar de hacer un crossfade seco/mojado. Bajar Mix no borra
la memoria ya grabada. Feedback puede sostener el sonido; STOP o CC120 limpian
los estados y la memoria del adaptador.

**Value 246 FX** puede modular cualquiera de los cuatro colores utilizando la
ruta/rangos de tipo 0 ya contrastados. Los parámetros nuevos se guardan con
proyectos, slots y LIFT/DROP, y admiten automatización y los controles MIDI
contextuales del host con el editor cerrado. No se añaden presets originales.

Los cuatro botones de set 3 eligen un efecto. Si una automatización activa varios,
la prioridad es **Delay → Nitro → Filter → Punch**. El bus mono sigue al
sintetizador, cinta, entrada, EQ y master; es ruteo propio. No imprime el efecto en
las pistas crudas exportadas. No añade otro retardo al bus: sigue declarando unos
1,5 ms, incluso en bypass (65/71/141 muestras a 44,1/48/96 kHz).

## Reconstrucción contrastada

El callback mono `0x0197d8c4` usa una memoria circular de hasta **44.100 int32**,
tres cursores Q16 y convolución/interpolación de siete taps con 32 fases. Se
conservan el orden de acumulación/saturación, los redondeos Q15/Q31, la limpieza
por cursor, el envío y realimentación, y la suma seca. Cambiar Size reduce los
cursores módulo el tamaño seleccionado y reutiliza las celdas existentes, como
la ruta observada. No se sustituye por un delay lineal genérico.

Los **225 coeficientes Q15** proceden de los floats legibles en `0x018bebcc`,
convertidos por `0x01aa7014` siguiendo el fragmento `0x01a53c2a..0x01a53c48`.
El mapa de Size legible en `0x018d514c` determina los seis tamaños. No se ejecuta
el arranque completo del instrumento. El reset de cursores `0x0197d7d0` fija
lectura/limpieza/escritura en **9/8/1 muestras**, conservando tag y slot.

El puerto coincide en **4.096 bloques / 262.144 muestras**, todos los estados de
20 bytes y displays de 64 bytes, los coeficientes, y ocho instantáneas completas
de **44.100 celdas** al terminar cada segmento de 512 bloques. Cuatro segmentos
usan el reset original; los demás parten de cursores sintéticos. Una repetición
privada independiente produce los mismos archivos.
[Hashes y contrato](firmware/delay246-validation.json).

## Adaptación al plugin y límites

El harness enlaza explícitamente el slot 0 con un workspace externo. La reserva,
propiedad y limpieza de esa memoria son nuestras; no se afirma haber reproducido
el allocator original ni recuperado una captura L1. Los ceros iniciales son una
condición del ensayo. No se utilizan coeficientes hipotéticos para esta ruta.

El adaptador compartido procesa bloques de 64 muestras a 44,1 kHz. Su conversión
de tasas, anti-alias de entrada, interpolación de salida, DC de 15 Hz, guardia
inicial y crossfade de activación son propios. El núcleo opera en int16, por lo
que activar el bus puede colorear la señal seca incluso con Mix a cero; el bypass
del efecto apagado sí conserva exactamente el float seco retrasado.

Controles y selección se toman en los límites de bloque interno; el host suaviza
los controles antes. Cambiar de efecto reinicia su estado y limpia esta memoria.
La compensación de latencia permanece fija y la cola se declara potencialmente
infinita. Cinta/mezcla/ruteo estéreo, clock original y acústica del OP-1 completo
siguen sin contrastar. Las pruebas nativas de CI tampoco certifican aceptación en
Ableton ni ausencia de dropouts físicos.

Las pruebas incluyen oráculo completo, bypass a siete tasas, ausencia de falta de
datos, silencio/pánico, 8.192 muestras nativas frente al núcleo y retorno causal
de un impulso tras recorrer la memoria. También se comprueban proxies/GUI,
proyectos/slots/clipboard, prioridad y renders iguales con buffers 64/257.

## Reproducción privada

```bash
python3 scripts/verify-delay246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/delay-nuevo
```

El directorio debe ser nuevo y externo al repositorio. Firmware, ELF, disassembly
y pseudocódigo quedan privados. Los fixtures son entradas sintéticas y resultados
numéricos; no contienen instrucciones del firmware.
