# Dr. Wave 246: integración híbrida

El motor **07 / DR. WAVE** hace tocable el núcleo reconstruido sin esperar una
captura de L1 descifrada. BROWSE → **Dr. Wave 246 Hybrid → Init / Dr. Wave** carga
un punto de partida propio, sin efectos. No es una réplica de un preset original.

Set 1 ofrece **Shape, Spectrum, Warp y Spread**, respectivamente azul, verde,
blanco y naranja. Azul recorre tres secciones y tiene rango 0–24575; verde y
blanco usan 0–16383 y naranja 0–32767 (rangos corregidos en 0.16). Son los cuatro
controles del render contrastado, no macros que
seleccionan otros sintetizadores. Set 2 controla Attack, Decay, Sustain y Release
con la ADSR reconstruida. Release tiene la orientación raw del firmware: valores
altos acortan la cola. Los cuatro mandos MIDI siguen la página con el editor
abierto o cerrado. Set 3 conserva los efectos disponibles; desde 0.16, set 4
permite [Value por voz sobre Instrument/ADSR](value-hybrid246-native.md), además
del LFO propio disponible al apagarlo.

## Qué se conserva y qué se reconstruye por aproximación

`DrWave246Kernel.h`, sus generadores de las 320 ondas y las transformaciones no
cambian. Conservan los oráculos anteriores de lectura, espectro, preparación de
controles y estado entero. La integración llama a ese mismo núcleo en bloques de
128 muestras a 88,2 kHz. La ADSR mantiene su núcleo contrastado.

`DrWave246Voice.h` y el ruteo de `Engine.h` son adaptadores propios:

- Afinación temperada A4=440 Hz y conversión directa a la palabra de fase. Bend
  de ±2 semitonos por canal, sustain y posiciones MIDI del plugin.
- Seis voces polifónicas, primera libre y robo de la más antigua. Cada voz tiene
  un workspace independiente usando el slot cero y su reloj módulo seis; genera
  una onda en su primer bloque y la refresca cada seis bloques internos. Esto
  evita silencio inicial, pero **no reproduce el planificador compartido L1**.
- Controles suavizados en 12 ms; cambio de pitch cuantizado por el bloque interno.
  El callback externo puede tener cualquier longitud sin reiniciar esos bloques.
- ADSR aplicada a la salida de cada voz; dither por voz de la implementación
  existente. Su conexión completa en el hardware sigue sin recuperarse.
- Bloqueo DC de 15 Hz, filtro Butterworth causal de salida y conversión lineal a
  la frecuencia del host. Mezcla, ganancia/velocidad, efectos y modulación propios.
- Relevo de 3 ms desde la última contribución al robar una voz. No es la rampa de
  reinicio original. Cambiar entre este motor y otro retira las voces incompatibles.

La clave OTP no se ha recuperado. Esta estrategia no descifra el firmware ni
convierte en verificadas las dependencias pendientes. Tampoco demuestra todavía
que una nota completa suene igual al aparato. El usuario autoriza avanzar sin
priorizar compatibilidad de sesiones anteriores; el selector ahora tiene siete
opciones. Sí se prueba la persistencia de las sesiones nuevas.

## Validación y siguiente bloque

`drwave246_hybrid` comprueba afinación a 44,1/48/88,2/96 kHz, bend, sustain,
release, seis voces y robo, extremos MIDI y las 16 esquinas de los controles.
`plugin_smoke` comprueba proxies/GUI, slots, clipboard, sesiones y eventos MIDI
con buffers de 64 y 257 muestras. `realtime` añade seis cargas Dr. Wave a las 96
existentes: primer disparo y callback continuo sin llamadas al heap rastreadas.
Los oráculos previos siguen siendo la evidencia del núcleo, no de este adaptador.

Para acercarlo más al original, el siguiente bloque será calibrar notas aisladas,
velocidades, barridos de cada color, envolventes y acordes frente a capturas de un
OP-1 con ajustes conocidos. Medir afinación, espectro, RMS, transitorios y tiempos
permite corregir adaptadores sin alterar un núcleo ya contrastado. Si no hay
capturas, continuar con los bloques legibles de Cluster y sus límites explícitos;
no certificar fidelidad acústica por inspección del código. La réplica completa
de presets sigue reservada para la etapa final.
