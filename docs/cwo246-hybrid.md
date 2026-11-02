# CWO Hybrid 246 — composición mono contrastada e integrada

**Set 3 → FX → CWO Hybrid**: Frequency, Delay, Feedback y Sideband, con Value FX,
automatización, MIDI contextual del host y guardado en proyectos, slots y LIFT/DROP.
Los cuatro controles conservan el dominio raw **0–32767**. Los valores iniciales
son propios del adaptador; no se añaden presets originales.

## Qué coincide y qué depende del modelo

Se ejecuta el callback mono original **`0x0194480c`** con su suavizado, tablas,
filtro, allpass, recortador, retardo, red de fase, modulación, mezclas, RNG y display.
La reconstrucción C++ coincide en **4096 bloques / 262.144 muestras**, todos los
estados de **252 bytes**, display de 64 bytes y ocho memorias circulares completas
de **65.536 celdas int16**. Se prueban ocho estados/workspaces sintéticos,
controles extremos y continuos, impulsos, silencio, DC negativo y alternancia.
La repetición privada independiente reproduce las fixtures byte a byte.

La coincidencia está **condicionada a cuatro coeficientes de fase propios**.
Se conservan exactamente los otros cuatro, legibles en el firmware; el script
rechaza modificarlos. No se presenta el modelo como memoria descifrada ni como
recuperación de esos cuatro valores originales. [Procedencia y modelo](cwo246-foundation.md),
[registro de palabras protegidas](firmware/cwo246-phase-blocker.json).

El modelo tiene un presupuesto analítico de cuadratura inferior a **4,3°** entre
30 y 18.000 Hz a 44,1 kHz, comprobado sobre 10.000 puntos. Describe una red lineal
sin saturación; no demuestra respuesta acústica o rechazo de banda lateral del
OP-1 físico. Los errores respecto a esas cuatro constantes originales siguen
pendientes de una referencia independiente.

## Dependencias cerradas

Además de los [ocho auxiliares anteriores](cwo246-foundation.md), se contrastan:

- **Retardo variable `0x01943744`:** 8192 llamadas, dos rutas de interpolación,
  límite de |delta| 60/61, vueltas del cursor, supresión de lectura inicial y
  preservación de flags. Dieciséis memorias completas de 128 KiB coinciden.
- **Lookup `0x01944480` y oscilador `0x019446e0`:** 8192 bloques de 64 muestras
  en cada salida seno/coseno, fases e incrementos en `[-2^30,2^30)`, extremos y
  amplitud signed16. Se conserva la recurrencia Q15 y sus saturaciones.
- **ABI:** el oscilador recibe fase/incremento por R1/R2 y amplitud/salidas en la
  pila; R0 no se consume. Se comprueba en las instrucciones y el ensayo completo,
  sin modificar bytes del firmware. Las anotaciones del decompilador son metadatos
  del analista y el pseudocódigo permanece privado.

La composición mantiene las rampas por muestra, realimentación de un sample,
mezcla de bandas laterales, filtro previo, reloj y RNG del medidor. No se sustituye
el callback por una fórmula propia. El dominio del oráculo usa **slot 0**,
cursores válidos y puntero explícito al workspace. No ejecuta el allocator ni
el arranque completo. [Validación del tiempo](firmware/cwo246-time-validation.json)
y [del callback](firmware/cwo246-kernel-validation.json).

## Adaptador del host

CWO ocupa el bus mono compartido después de síntesis, cinta/entrada, EQ y
drive/eco maestro, y antes del volumen maestro. Prioridad si se activan varios
flags: **CWO > Spring > Phone > Grid > Delay > Nitro > Filter > Punch**.
El selector conserva las ocho opciones anteriores. [Cadena de salida](output-routing.md).

Se conservan los bloques internos de 64 muestras a 44,1 kHz y el PDC fijo
`ceil(64 × tasa_host / 44100) + 1`, incluso en bypass. El retardo musical no se
suma al PDC. Resampling, DC de 15 Hz, guarda de silencio y transición de bypass
son propios del adaptador compartido.

Activación, cambio de modo y pánico usan un reset completo propio de estados y
memoria, con fase inicial válida y semilla RNG determinista propia. No se afirma
que sea el constructor/reset original. La ruta estéreo, cadencia externa,
comparación acústica y Ableton real siguen fuera de esta evidencia.

La integración comprueba siete tasas (8–192 kHz), bypass exacto retrasado,
8192 muestras frente al núcleo directo, respuesta procesada y automatización
extrema. Las pruebas del plugin cubren las nueve opciones, Configure/proxies,
proyectos/slots/clipboard, renders 64/257 y borrado con CC120. La regresión de
volumen se amplía a CWO y se añaden 24 cargas de callback (372 en total).
[Validación de entrega](releases/v0.26.0-validation.json).

Los ocho FX del perfil rev246 tienen ya una opción tocable mono, con límites
individuales: Nitro y CWO conservan sustituciones explícitas, y la cadena de
salida del host sigue siendo propia. El siguiente bloque prioriza voz/ruteo y
motores restantes, empezando por las dependencias del FM de cuatro operadores.
Los presets originales completos permanecen para la etapa final.
